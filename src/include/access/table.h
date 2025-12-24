/*-------------------------------------------------------------------------
 *
 * table.h
 *	  Generic routines for table related code.
 *
 *
 * Portions Copyright (c) 1996-2024, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/access/table.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef TABLE_H
#define TABLE_H

#include "nodes/primnodes.h"
#include "storage/lockdefs.h"
#include "utils/relcache.h"

extern Relation table_open(Oid relationId, LOCKMODE lockmode, int32 dbforkId);
extern Relation table_openrv(const RangeVar *relation, LOCKMODE lockmode, int32 dbforkId);
extern Relation table_openrv_extended(const RangeVar *relation,
									  LOCKMODE lockmode, bool missing_ok, int32 dbforkId);
extern Relation try_table_open(Oid relationId, LOCKMODE lockmode, int32 dbforkId);
extern void table_close(Relation relation, LOCKMODE lockmode);

#endif							/* TABLE_H */
