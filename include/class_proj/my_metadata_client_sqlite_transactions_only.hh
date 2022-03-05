


#ifndef MYMETADATACLIENTSQLITEONLYTRANSACTIONS_H
#define MYMETADATACLIENTSQLITEONLYTRANSACTIONS_H#include <my_metadata_client.h>



int metadata_abort_transaction(const md_server &server);
int metadata_begin_transaction(const md_server &server);
int metadata_commit_transaction(const md_server &server);



#endif
