# tracing Simple Select FullScan

./src/backend/tcop/postgres.c
PostgresMain()
  |
./src/backend/tcop/postgres.c
exec_simply_query()
  |
 * (same file as above) pg_parse_query() //calls yacc parser returns basic tree
 * (./backend/tcop/utility.c) CreateCommandTag() //gives stmt CMDTAG_INSERT
 * (postgres.c) pg_analyze_and_rewrite_fixedparams() //analyzes and gives CMD_UTILITY tag and makes querytree
 * (postgres.c) pg_plan_queries() //makes the plan for queries and calls optimizer
 * * (backend/optimizer/plan/planner.c) standard_planner() //makes better plan
 * (portalmem.c) CreatePortal()
 * (pquery.c) PortalStart()
 * * (execMain.c) ExecutorStart() //setup initialization in select mode
 * * * (execUtils.c) ExecGetRangeTableRelation() //actually open relation
 * * * (execProcnode.c) ExecInitNode() //recursively intialize nodes
 * * * * (backend/executor/nodeSeqscan.c) ExecInitSeqScan //initialize for seqscan, sets ExecSeqScan as the execution funciton
 * * * * * (execUtils.c) ExecOpenScanRelation() //high level function for opening relation to scan
 * * * * * * (execUtils.c) ExecGetRangeTableRelation() //actually calls table_open here
 * (pquery.c) PortalRun() //most of the execution here 
 * * (pquery.c) PortalRunSelect() //the portal run code for select stmts singular
 * * * (execMain.c) ExecutorRun() //for select the start shouldve been before, aka standard_ExecutorRun
 * * * * (execMain.c) ExecutePlan() //calls execprocnode repeatdely to get result tuple, remember execprocnode is a function pointer so changes for each
 * * * * * (nodeSeqscan.c) ExecSeqScan() // ExecProcnode function in ExecutePlan loop, calls the functions to actually scan relation
 * * * * * * (backend/executor/execScan.c) ExecScan() //this function takes in 2 function pointer ExecScanAccessMtd=SeqNext && ExecScanRecheckMtd=SeqRecheck. Common function to route for scanning relation in different ways. note seqrecheck just returns true always
 * * * * * * * (execScan.c) ExecScanFetch() //fetch next tuple using the functions provided
 * * * * * * * * (nodeSeqscan.c) SeqNext() //access method function pointer in ExecScanFetch called for select scan to fetch tuple
 * * * * * * * * * (tableam.h) table_beginscan() //setup scan for table scan in not parallel 
 * * * * * * * * * * (heapam.c) heap_beginscan() //setup heap file for scanning
 * * * * * * * * * * * (heapam.c) initscan() //common function for begin scan in heaps
 * * * * * * * * * (include/access/tableam.h) table_scan_getnextslot() //table function to fetch tuple
 * * * * * * * * * * (backend/access/heap/heapam.c) heap_getnextslot() //heap function to fetch tuple
 * * * * * * * * * * * (heapam.c) heapgettup() //loop to fetch from heap
 * * * * * * * * * * * * (heapam.c) heap_fetch_next_buffer() //start function to fetching the buffers from heap files
 * * * * * * * * * * * * (heapam.c) heapgettup_start_page() //set pointer to tuples in page for first scan
 * * * * * * * * * * * * (heapam.c) heapgettup_continue_page() //sets pointer to continue off last page scanned
 * (postgres.c) PortalDrop()
 * (postgres.c) EndCommand() //loopback to next query if any in parse tree
 * (postgres.c) finish_xact_command() //outside of loop marks end of transaction
