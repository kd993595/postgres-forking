# Features

### Utilities

CREATE DBFORK - creates new fork directly from the current fork number whether main or some fork.
DROP DBFORK <number> - supposed to delete the current fork and children but for now switches to the given fork id.
SET DBFORK <number> - TODO
SHOW DBFORK - TODO

*** All other utility statements like alter, create, drop work the same in the main database with most not working from forked branches.

### Insert
 - Regular Insert ... Values (...) works as normal and places values into fork files not affecting parent files.
 - Insert Into Select works only with whatever capabilites select currently has.

### Select
 - Full table scans: "Select * from <table>;" or "Select * from <table> where <non index conditionals>" or "Select <some type of projection or computation> from <table>" is working as intended
 - Index scans: not tested
 - Joins: not working
 - all other types not working

 - Reordering by either "unique" or "order by" should work but not tested

### Delete
not working

### Update
not working
