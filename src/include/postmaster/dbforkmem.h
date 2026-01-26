/*-------------------------------------------------------------------------
 *
 * dbforkmem.h
 *	  Exports from postmaster/sharedbfork.c 
 *
 * database forking must have shared memory to handle an atomic global forkid
 * that all backend processes respect to select the new fork id.
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 *
 * src/include/postmaster/dbforkmem.h
 *
 *-------------------------------------------------------------------------
 */

#ifndef _DBFORKCOUNTER_H
#define _DBFORKCOUNTER_H

/* Adding the global dbfork counter here for now since in both ipci.c and dbcommands.c - pgforking*/
extern Size DBForkShmemSize(void);
extern void DBForkShmemInit(void);
extern int32 DBForkNewId(void);
extern int32 DBForkSetNewIdExpensive(int32 newId);

#endif /* _DBFORKMEM_H */
