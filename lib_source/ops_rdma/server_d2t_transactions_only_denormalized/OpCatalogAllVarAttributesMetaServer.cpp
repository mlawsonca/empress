#include <OpCatalogAllVarAttributesMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_var_attributes_stub (const md_catalog_all_var_attributes_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_var_attributes_args &args, uint32_t &count);


WaitingType OpCatalogAllVarAttributesMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_var_attributes_args sql_args;
    std::vector<md_catalog_var_attribute_entry> attribute_list;
    uint32_t count;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_var_attributes_stub(sql_args, attribute_list, count);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_MD_CATALOG_ALL_VAR_ATTRIBUTES_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    // cout << "in catalog all, attr list is of size: " << attribute_list.size() << endl;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_catalog_all_var_attributes_stub (const md_catalog_all_var_attributes_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select vac.* from var_attribute_catalog vac "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.run_id = ? "
    "and vac.timestep_id = ? ";

    // const char * query_all = "select vac.* from var_attribute_catalog vac "
    // "inner join type_catalog tc on vac.type_id = tc.id "
    // "where (vac.txn_id = ? or vac.active = 1) "
    // "and vac.run_id = ? "
    // "and vac.timestep_id = ? ";

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_attribute_catalog";
    // fprintf(stdout, "Starting attr operation\n");
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // if (rc == SQLITE_OK) {
    //     fprintf(stdout, "Attr Operation done successfully\n");
    // }

    // err_msg = 0;
    // sql = "SELECT * FROM type_catalog";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // if (rc == SQLITE_OK) {
    //     fprintf(stdout, "Type Operation done successfully\n");
    // }

    rc = get_matching_attribute_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        // if(args.timestep_id == ALL_TIMESTEPS) {
        //     rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        //     rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        //     rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
        // }
        // else {
            rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);    
        // }

        rc = sql_retrieve_var_attrs(stmt, attribute_list);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from var_attribute_catalog vac "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and vac.run_id = ? "
    "and vac.timestep_id = ? ";

    // const char * query_all = "select count (*) from var_attribute_catalog vac "
    // "inner join type_catalog tc on vac.type_id = tc.id "
    // "where (vac.txn_id = ? or vac.active = 1) "
    // "and vac.run_id = ? ";

    // if(args.timestep_id == ALL_TIMESTEPS) {
    //     rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    //     rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    //     rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    // }
    // else {
        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);    
    // }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
