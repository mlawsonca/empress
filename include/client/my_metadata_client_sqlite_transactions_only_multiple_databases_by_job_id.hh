


#ifndef MY_METADATA_CLIENT_SQLITE_TRANSACTIONS_ONLY_MULTIPLE_DATABASES_READ_AND_WRITE_NEW
#define MY_METADATA_CLIENT_SQLITE_TRANSACTIONS_ONLY_MULTIPLE_DATABASES_READ_AND_WRITE_NEW

//
#include <my_metadata_client_sqlite_transactions_only_multiple_databases_read_and_write_new.hh>

int metadata_commit_transaction (const md_server &server
                                 , uint64_t job_id);

int metadata_abort_transaction (const md_server &server
                                 , uint64_t job_id);

#endif
