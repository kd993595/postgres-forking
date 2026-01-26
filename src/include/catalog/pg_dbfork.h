/*-------------------------------------------------------------------------
 *
 * pg_dbfork.h
 *	  definition of the "shared dbfork" system catalog (pg_dbfork)
 *
 * The system catalog file that is global to all databases responsible for
 * keeping track of the forkid in this postgres instance.
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_dbfork.h
 *
 * NOTES
 *	  The Catalog.pm module reads this file and derives schema
 *	  information.
 *
 *-------------------------------------------------------------------------
 */
#ifndef PG_DBFORK_H
#define PG_DBFORK_H

#include "catalog/genbki.h"
#include "catalog/pg_dbfork_d.h"

/* ----------------
 *		pg_dbfork definition.  cpp turns this into
 *		typedef struct FormData_pg_dbfork
 *		note: db_xid should represent an xid uin32
 * ----------------
 */
CATALOG(pg_dbfork,6347,SharedDBForkIDRelation) BKI_SHARED_RELATION
{
	int32		forkid;
	int32		parentid;
	TransactionId db_xid;
} FormData_pg_dbfork;

/* ----------------
 *		Form_pg_dbfork corresponds to a pointer to a row with
 *		the format of pg_dbfork relation.
 * ----------------
 */
typedef FormData_pg_dbfork * Form_pg_dbfork;

DECLARE_UNIQUE_INDEX_PKEY(pg_dbfork_index, 6348, SharedDBForkOidIndexId, pg_dbfork, btree(forkid int4_ops));

#endif							/* PG_DBFORK_H */
