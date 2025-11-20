/*-------------------------------------------------------------------------
 *
 * pg_dbfork.h
 *	  definition of the "shared dependency" system catalog (pg_shdepend)
 *
 * pg_shdepend has no preloaded contents, so there is no pg_shdepend.dat
 * file; dependencies for system-defined objects are loaded into it
 * on-the-fly during initdb.  Most built-in objects are pinned anyway,
 * and hence need no explicit entries in pg_shdepend.
 *
 * NOTE: we do not represent all possible dependency pairs in pg_shdepend;
 * for example, there's not much value in creating an explicit dependency
 * from a relation to its database.  Currently, only dependencies on roles
 * are explicitly stored in pg_shdepend.
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/catalog/pg_shdepend.h
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
 * ----------------
 */
CATALOG(pg_dbfork,6347,SharedDBForkIDRelation) BKI_SHARED_RELATION
{
	/*
	 * Idk what to put here yet since we custom read it
	 *
	 */
	int32 forkid;
	int32 parentid;
	int64 curTime;
} FormData_pg_dbfork;

/* ----------------
 *		Form_pg_shdepend corresponds to a pointer to a row with
 *		the format of pg_shdepend relation.
 * ----------------
 */
typedef FormData_pg_dbfork *Form_pg_dbfork;

DECLARE_UNIQUE_INDEX_PKEY(pg_dbfork_index, 6348, SharedDBForkOidIndexId, pg_dbfork, btree(forkid int4_ops));

#endif 		/* PG_DBFORK_H */
