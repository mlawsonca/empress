



#ifndef MYMETADATACLIENTLOCALSQLITEONLYTRANSACTIONS_H
#define MYMETADATACLIENTLOCALSQLITEONLYTRANSACTIONS_H

#include <my_metadata_client_local.hh>


// used by each of the activate functions
int metadata_abort_transaction();
int metadata_begin_transaction();
int metadata_commit_transaction();


#endif
