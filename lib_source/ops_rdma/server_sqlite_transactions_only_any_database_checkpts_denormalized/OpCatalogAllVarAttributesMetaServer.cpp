#include <OpCatalogAllVarAttributesMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id, md_query_type query_type, bool &trans_active);
extern void close_database(uint64_t job_id);


extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


using namespace std;

int md_catalog_all_var_attributes_stub (sqlite3 *db, const md_catalog_all_var_attributes_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count);

static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_args &args, uint32_t &count);


WaitingType OpCatalogAllVarAttributesMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_var_attributes_args args;
    std::vector<md_catalog_var_attribute_entry> entries;
    uint32_t count = 0;
    bool trans_active;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_DEARCHIVE_MSG_FROM_CLIENT);

    // cout << "OpCatalogAllVarAttributesMeta: about to query COMMITTED" << endl;
    sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    if(!trans_active) {
        rc = md_catalog_all_var_attributes_stub(db, args, entries, count);
        // cout << "rc: " << rc << endl;
    }
    else {
        // cout << "OpCatalogAllVarAttributesMeta: db_busy" << endl;
        rc = RC_DB_BUSY;
    }

    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_MD_CATALOG_ALL_VAR_ATTRIBUTES_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    // cout << "in catalog all, attr list is of size: " << entries.size() << endl;
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


int md_catalog_all_var_attributes_stub (sqlite3 *db, const md_catalog_all_var_attributes_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query = "select * from var_attribute_catalog vac where vac.run_id = ? "
    "and vac.timestep_id = ? ";

    // const char * query_all = "select * from var_attribute_catalog vac "
    // "inner join type_catalog tc on vac.type_id = tc.id "
    // "where vac.run_id = ? "
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

    rc = get_matching_attribute_count (db, args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        // if(args.timestep_id == ALL_TIMESTEPS) {
        //  rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        //  rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        // }
        // else {
            rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
            rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);  
        // }

        rc = sql_retrieve_var_attrs(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count (*) from var_attribute_catalog vac where vac.run_id = ? "
    "and vac.timestep_id = ? ";

    // const char * query_all = "select count (*) from var_attribute_catalog vac "
    // "inner join type_catalog tc on vac.type_id = tc.id "
    // "where vac.run_id = ? ";

    // if(args.timestep_id == ALL_TIMESTEPS) {
    //  rc = sqlite3_prepare_v2 (db, query_all, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    //  rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    // }
    // else {
        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);  
    // }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    // cout << "in catalog all attrs, count: " << count << endl;
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
