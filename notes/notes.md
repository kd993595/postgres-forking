# Notes Postgres forking development


## Parsing (new utility function)
- (/src/ assumed)/backend/parser/parser.c
- /backend/parser/scan.l (FORK,FORKDELETE)
- /backend/parser/gram.y (FORKDELETE <id>)
- /common/keywords.c
- /include/parser/kwlist.h


## Executor
- /backend/tcop/pquery.c
- /backed/execute

## WAL (xlog)
- 


### utility commands
In dbcommands the dropdb and createdb are made. Seems that utility query are not optimizable and are passed with an exclusive lock for the entire database.

https://github.com/kd993595/postgres-forking/blob/main/src/include/nodes/parsenodes.h
### links:
https://github.com/postgres/postgres/tree/master/src/backend/parser
https://www.postgresql.org/docs/current/install-requirements.html
https://www.postgresql.org/ftp/source/v17.5/
https://github.com/neondatabase/neon/blob/main/libs/wal_decoder/src/decoder.rs
https://wiki.postgresql.org/wiki/Developer_FAQ#How_do_I_get_involved_in_PostgreSQL_development.3F
https://neon.tech/blog/architecture-decisions-in-neon
wal: https://github.com/postgres/postgres/blob/master/src/backend/access/transam/xlog.c
https://www.postgresql.org/docs/current/app-pg-ctl.html
https://www.postgresql.org/docs/devel/


### page service and wal for neon
https://github.com/neondatabase/neon/blob/main/docs/pageserver-storage.md
https://github.com/neondatabase/neon/blob/main/docs/walservice.md


### extra info
the /usr/local/pgsql is general postgres tools and logs
the /usr/local/pgsql/data is created by me to initdb a new database and is the data storage for the actual database and should be deleted if wanna rerun
