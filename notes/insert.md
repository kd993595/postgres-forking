# Tracing Insertion logic

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
 * (pquery.c) PortalRun() //most of the execution here 
 * * (backend/tcop/pquery.c) PortalRunMulti() //executes mutliple queries or non-select-like queries 
// * * * (pquery.c) PortalRunUtility() //only for utility commands
 * * + (pquery.c) ProcessQuery() //actual code for execution logic
 * * * * (backend/executor/execMain.c) ExecutorStart() //Setup all necessary stuff to run a command aka standard_ExecutorStart()
 * * * * * (execUtils.c) CreateExecutorState() //sets up the memory for the node in queries
 * * * * * (execMain.c) InitPlan() //Initialize plan state tree for rest of execution
 * * * * * * (execUtils.c) ExecInitRangeTable() //sets up range table entry data and memory
 * * * * * * (execUtils.c) ExecGetRangeTableRelation() //opens up relations for rowmarks idk what that is
 * * * * * * (backend/executor/execProcnode.c) ExecInitNode() //initialize all nodes in plan tree
 * * * * * * * (backend/executor/nodeModifyTable.c) ExecInitModifyTable() //initialize for modify table likely the only one for insert stmt and possible t_result, sets ps.ExecProcNode very important this is the actual executor function
 * * * * * * * * (execUtils.c) ExecInitResultRelation() // Initialize and setup relation for further processing
 * * * * * * * * * (execUtils.c) ExecGetRangeTableRelation() //opens relation in parallel/non-parallel modes
 * * * * * * * * * (execMain.c) InitResultRelInfo() //zeroes out most of the stuff
 * * * * * * * * (execProcnode.c) ExecInitNode() //recrusively calls onto subplan
 * * * * * * * (backend/executor/nodeSubPlan.c) ExecInitSubPlan() //Idk if this is called for insert might want to check out but looks to be expr for select
 * * * * (backend/executor/execMain.c) ExecutorRun() //actually run through state plan tree aka standard_ExecutorRun
 * * * * * (execMain.c) ExecutePlan() //process query plan until completion
 * * * * * * (nodeModifyTable.c) ExecModifyTable() //The ExecProcNode is a function pointer set by the ExecutorStart, for this one actually call modify table code which has switch stmt for insert, update, delete, merge
 * * * * * * * (nodeModifyTable.c) ExecInitInsertProjection() //makes sure insert is proper format for tuples, as well as build projection info for tuple desc
 * * * * * * * (nodeModifyTable.c) ExecGetInsertNewTuple() //prepares actual tuple to be inserted and coerces if need to
 * * * * * * * (nodeModifyTable.c) ExecInsert() //actually insert tuple into relation
 * * * * * * * * (backend/executor/execIndexing.c) ExecOpenIndices() //open up indexes for a relation to also insert into
 * * * * * * * * (if stmt) (execIndexing.c) ExecCheckIndexConstraints() //check index for constraints
 * * * * * * * * (if stmt) (include/access/tableam.h) table_tuple_insert() //insert tuple
 * * * * * * * * * (backend/access/heap/heapam_handler.c) heapam_tuple_insert() //heap insertion logic
 * * * * * * * * * * (backend/access/heap/heapam.c) heap_insert() //actual logic for insertion, todo: modify here for wal
 * * * * * * * * * * * (backend/access/heap/hio.c) RelationGetBufferForTuple() //get buffer for this relation needs, should be fine sine buftag has it check if later write is fine
 * * * * * * * * * * * (hio.c) RelationPutHeapTuple() //critical function so no errors, places tuple in buffer
 * * * * * * * * (execIndexing.c) ExecInsertIndexTuples() //insert tuple into indexes of relation, todo: modify this to check parent index
 * * * * (backend/executor/execMain.c) ExecutorFinish() //Fires after triggers and any final commands
 * * * * * (execMain.c) ExecPostprocessPlan() //resets context expression and lets tuple finish if they didnt
 * * * * (backend/executor/execMain.c) ExecutorEnd() //Cleans up memory and restores context
 * (postgres.c) PortalDrop()
 * (postgres.c) EndCommand() //loopback to next query if any in parse tree
 * (postgres.c) finish_xact_command() //outside of loop marks end of transaction
