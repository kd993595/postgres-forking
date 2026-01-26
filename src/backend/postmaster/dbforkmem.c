/*-------------------------------------------------------------------------
 *
 * dbforkmem.c
 *
 * The checkpointer is new as of Postgres 9.2.  It handles all checkpoints.
 *
 * Shared memory between backend processes for a global single forkid.
 * This forkid is used between all databases since the relation between
 * forkid is irrelevant and the only criteria is uniqueness between them.
 * Currently only need an atomic counter for the actual forkid may change in
 * the future so treating it as if it was large size. Putting it here since
 * it seems both ipci.c and dbcommands.c are aware of this file.

 *
 * Utility functions for manipulating the current forkid for a backend process
 * also included here.
 *
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 *
 *
 * IDENTIFICATION
 *	  src/backend/postmaster/dbforkmem.c
 *
 *-------------------------------------------------------------------------
 */
#include "postgres.h"

#include <sys/time.h>
#include <time.h>

#include "access/transam.h"
#include "access/xact.h"
#include "access/xlog.h"
#include "access/xlog_internal.h"
#include "access/xlogrecovery.h"
#include "libpq/pqsignal.h"
#include "miscadmin.h"
#include "postmaster/dbforkmem.h"
#include "storage/condition_variable.h"
#include "storage/fd.h"
#include "storage/ipc.h"
#include "storage/lwlock.h"
#include "storage/shmem.h"
#include "storage/smgr.h"
#include "storage/spin.h"
#include "utils/memutils.h"
#include "utils/resowner.h"
#include "catalog/pg_dbfork.h"


/*
 * DBFork Shmem Counter - pgforking
 */
typedef struct
{
	pg_atomic_uint32 fork_id_counter;
}			DBForkShmemStruct;

typedef struct DBForkNode
{
	FormData_pg_dbfork entry;
	struct DBForkNode *firstChild;
	struct DBForkNode *nextSibling;
	struct DBForkNode *parent;
	int			depth;
}			DBForkNode;

static DBForkShmemStruct * DBForkShmem;

bool		
InsertDBForkNode(DBForkNode * root, DBForkNode * insert, int curDepth);

/*
 * DBForkShmemSize - computer the space needed for the dbforking-related shared memory
 */
Size
DBForkShmemSize(void)
{
	Size		size = 0;

	size = add_size(size, sizeof(DBForkShmemStruct));
	return size;
}

/*
 * DBForkShmemInit - allocate and initialize dbfork-related shared memory
 * TODO: have to add more checks for things like out of mem or file access
 * This issue is here it has to do with trying to access file outside of the bootstrap mode.
 */
void
DBForkShmemInit(void)
{
	Size		size = DBForkShmemSize();
	bool		found;

	DBForkShmem = (DBForkShmemStruct *) ShmemInitStruct("DBForking Data", size, &found);

	if (!found)
	{
		char	   *tmpPath;
		FILE	   *dbfork_config;

		/* First time through, so initialize. */
		tmpPath = psprintf("global/%d", SharedDBForkIDRelation);
		dbfork_config = fopen(tmpPath, "rb");
		if (dbfork_config != NULL)
		{
			/* File already exists so we have to read it */
			FormData_pg_dbfork last_entry;
			size_t		num_read;
			int			seek_res;

			seek_res = fseek(dbfork_config, -sizeof(FormData_pg_dbfork), SEEK_END);
			if (seek_res != 0)
			{
				/*
				 * file probably doesn't have anything so we just initialize
				 * to 1
				 */
				pg_atomic_init_u32(&DBForkShmem->fork_id_counter, 1);
			}
			else
			{
				num_read = fread(&last_entry, sizeof(FormData_pg_dbfork), 1, dbfork_config);
				Assert(num_read == 1);
				pg_atomic_init_u32(&DBForkShmem->fork_id_counter, last_entry.forkid + 1);
			}
			fclose(dbfork_config);
		}
		else
		{
			/* fprintf(stderr, "file cannot be opened for some reason"); */
			pg_atomic_init_u32(&DBForkShmem->fork_id_counter, 1);
		}

		/* Assert(dbfork_config != NULL); */
		pfree(tmpPath);
	}
}

/*
 * DBForkNewId - pgforking
 * creates a new fork id and inserts into the dbfork config file so everyone else aware.
 */
int32
DBForkNewId(void)
{
	int32		newForkId;
	FormData_pg_dbfork new_entry;
	FILE	   *dbfork_config;
	char	   *tmpPath;

	LWLockAcquire(DBForkCounterLock, LW_EXCLUSIVE);

	newForkId = pg_atomic_fetch_add_u32(&DBForkShmem->fork_id_counter, 1);
	new_entry.forkid = newForkId;
	new_entry.parentid = MyDBForkId;
	new_entry.db_xid = GetCurrentTransactionId();
	tmpPath = psprintf("global/%d", SharedDBForkIDRelation);
	dbfork_config = AllocateFile(tmpPath, PG_BINARY_A);
	if (dbfork_config == NULL)
	{
		ereport(ERROR,
				(errcode_for_file_access(),
				 errmsg("could not open dbfork config file 6347\n")));
	}
	else
	{
		fwrite(&new_entry, sizeof(FormData_pg_dbfork), 1, dbfork_config);
	}

	pfree(tmpPath);
	FreeFile(dbfork_config);
	LWLockRelease(DBForkCounterLock);
	return newForkId;
}


bool
InsertDBForkNode(DBForkNode * root, DBForkNode * insert, int curDepth)
{
	if (insert->entry.parentid == root->entry.forkid)
	{
		/* insert the current node into the child of the root node */
		if (root->firstChild == NULL)
		{
			root->firstChild = insert;
		}
		else
		{
			DBForkNode *tmpNode = root->firstChild;

			while (tmpNode->nextSibling != NULL)
			{
				tmpNode = tmpNode->nextSibling;
			}
			tmpNode->nextSibling = insert;
		}
		insert->depth = curDepth + 1;
		insert->parent = root;
		return true;
	}
	/* check next sibling to see if we can insert into any of them */
	if (root->nextSibling != NULL)
	{
		if (InsertDBForkNode(root->nextSibling, insert, curDepth))
		{
			return true;
		}
	}
	/* check first child to see if we can insert into it */
	if (root->firstChild != NULL)
	{
		if (InsertDBForkNode(root->firstChild, insert, curDepth + 1))
		{
			return true;
		}
	}
	return false;
}

/* DBForkSetNewId - pgforking
 * set the backend process to use the current id passed in
 * TODO: fix for 4 states of changing
 */
int32
DBForkSetNewIdExpensive(int32 newId)
{
	char	   *tmpPath;
	FILE	   *dbfork_config;


	/*
	 * traverse the config file to build the tree until we get to the current
	 * id and then fetch path to our path
	 */
	LWLockAcquire(DBForkCounterLock, LW_SHARED);
	tmpPath = psprintf("global/%d", SharedDBForkIDRelation);
	dbfork_config = AllocateFile(tmpPath, PG_BINARY_R);
	if (dbfork_config == NULL)
	{
		ereport(ERROR,
				(errcode_for_file_access(),
				 errmsg("could not open dbfork config file 0$1259 ")));
	}
	else
	{
		FormData_pg_dbfork new_entry;
		DBForkNode	headNode;
		MemoryContext oldcontext,
					tmpcontext;

		MyParentForkId = MyDBForkId;
		MyDBForkId = newId;

		headNode.entry = (FormData_pg_dbfork)
		{
			0, 0, FirstNormalTransactionId
		};
		headNode.firstChild = NULL;
		headNode.nextSibling = NULL;
		headNode.parent = NULL;
		headNode.depth = 0;

		oldcontext = MemoryContextSwitchTo(TopMemoryContext);
		if (DBForkPath != NULL)
		{
			pfree(DBForkPath);
			DBForkPath = NULL;
		}
		MemoryContextSwitchTo(oldcontext);

		tmpcontext = AllocSetContextCreate(CurrentMemoryContext, "DBFork fork tree temporary context", ALLOCSET_SMALL_SIZES);
		oldcontext = MemoryContextSwitchTo(tmpcontext);

		while (fread(&new_entry, sizeof(FormData_pg_dbfork), 1, dbfork_config) == 1)
		{
			bool		isInserted;
			DBForkNode *tmpNode = palloc(sizeof(DBForkNode));

			tmpNode->entry = new_entry;
			tmpNode->firstChild = NULL;
			tmpNode->nextSibling = NULL;
			tmpNode->parent = NULL;
			isInserted = InsertDBForkNode(&headNode, tmpNode, 1);
			if (!isInserted)
			{
				ereport(ERROR,
						(errcode(ERRCODE_INVALID_PARAMETER_VALUE),
						 errmsg("invalid forkid given: %d", newId)));
			}
			if (new_entry.forkid == newId)
			{
				/* get path from this node to headnode */
				MemoryContextSwitchTo(TopMemoryContext);	/* allocate in the
															 * topmemory so it
															 * doesnt get erase for
															 * our cached path */
				DBForkPath = palloc(sizeof(int32) * tmpNode->depth);
				DBForkPath[0] = (int32) tmpNode->depth; /* set first int32 to be
														 * the depth of the
														 * forks */
				for (int i = tmpNode->depth - 1; i >= 1; i--)
				{
					DBForkPath[i] = tmpNode->entry.forkid;
					tmpNode = tmpNode->parent;
				}
				MemoryContextSwitchTo(tmpcontext);
				break;
			}
		}

		if (DBForkPath == NULL)
		{
			ereport(ERROR, (errcode(ERRCODE_INVALID_PARAMETER_VALUE), errmsg("Could not create the new dbfork path")));
		}
		MemoryContextSwitchTo(oldcontext);
		MemoryContextDelete(tmpcontext);
	}

	pfree(tmpPath);
	FreeFile(dbfork_config);
	LWLockRelease(DBForkCounterLock);
	return newId;
}
