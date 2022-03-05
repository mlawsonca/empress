#include <OpCatalogAllRunAttributesWithTypeMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_run_attributes_with_type_stub (const md_catalog_all_run_attributes_with_type_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_args &args, uint32_t &count);




WaitingType OpCatalogAllRunAttributesWithTypeMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_run_attributes_with_type_args sql_args;
    std::vector<md_catalog_run_attribute_entry> attribute_list;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_run_attributes_with_type_stub(sql_args, attribute_list, count);
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    // cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_run_attributes_with_type_stub (const md_catalog_all_run_attributes_with_type_args &args,
                           std::vector<md_catalog_run_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    // const char * query = "select rac.* from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and rac.type_id = ? "
    // "and tc.run_id = ? ";
    // }

    const char * query_all = "select rac.* from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? ";

    const char * query = "select rac.* from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and rac.run_id = ? ";

    rc = get_matching_attribute_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        if(args.run_id == ALL_RUNS) {
            rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);        
        }
        else {
            rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);        
            rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
        }

          rc = sql_retrieve_run_attrs(stmt, attribute_list);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_run_attributes_with_type_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    // const char * query = "select count (*) from run_attribute_catalog rac "
    // "inner join type_catalog tc on rac.type_id = tc.id "
    // "where (rac.txn_id = ? or rac.active = 1) "
    // "and rac.type_id = ? "
    // "and tc.run_id = ? ";
    
    const char * query_all = "select count (*) from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? ";

    const char * query = "select count (*) from run_attribute_catalog rac "
    "where (rac.txn_id = ? or rac.active = 1) "
    "and rac.type_id = ? "
    "and rac.run_id = ? ";

    if(args.run_id == ALL_RUNS) {
        rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);        
    }
    else {
        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.type_id); //assert (rc == SQLITE_OK);        
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
    }
        
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
