source code for the EMPRESS core library. Contains the code for message passing between client and servers and server-side managment of the metadata

#################################################################
####### source code #############################################
#################################################################

database - includes the source for SQLite / the metadata storage implementation. Is external - sqlite3.{hc} are generated from the public domain SQLite project.

local_db_functs - for EMPRESS's local service mode

lua - code for the objector

ops_rdma - code for EMPRESS's default (dedicated) service mode