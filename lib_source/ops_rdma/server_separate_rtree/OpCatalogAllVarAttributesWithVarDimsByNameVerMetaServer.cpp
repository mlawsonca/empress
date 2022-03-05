#include <OpCatalogAllVarAttributesWithVarDimsByNameVerMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_var_attributes_with_var_dims_by_name_ver_stub (const md_catalog_all_var_attributes_with_var_dims_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_var_dims_by_name_ver_args &args, uint32_t &count);


WaitingType OpCatalogAllVarAttributesWithVarDimsByNameVerMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_var_attributes_with_var_dims_by_name_ver_args sql_args;
    std::vector<md_catalog_var_attribute_entry> attribute_list;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_var_attributes_with_var_dims_by_name_ver_stub(sql_args, attribute_list, count);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_BY_NAME_VER_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}




// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_catalog_all_var_attributes_with_var_dims_by_name_ver_stub (const md_catalog_all_var_attributes_with_var_dims_by_name_ver_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if (args.num_dims == 1) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min ";   
    }
    else if(args.num_dims == 2) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min "
            "and ? <= vad.d1_max and ? >= vad.d1_min ";   
    }
    else if(args.num_dims == 3) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min "
            "and ? <= vad.d1_max and ? >= vad.d1_min "
            "and ? <= vad.d2_max and ? >= vad.d2_min ";   
    }
    else {
        count = 0;
        return rc;
    }

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_catalog";
    // cout << "starting var catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var catalog \n";

    // err_msg = 0;
    // sql = "SELECT * FROM var_attribute_catalog";
    // cout << "starting attr catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with attr catalog \n";

    rc = get_matching_attribute_count (args, count); //assert (rc == RC_OK);

   //  if(count == 0) {
   //    cout << "Var name asked for: " << args.var_name << endl;
   //    cout << "Var ver asked for: " << args.var_version << endl;
   //     cout << "timestep_id asked for: " << args.timestep_id << endl;
   // }

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_var_dims_by_name_ver_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 2, strdup(args.var_name.c_str()), -1, free); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int (stmt, 3, args.var_version); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.timestep_id); //assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
        }
        rc = sql_retrieve_var_attrs(stmt, attribute_list);


        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_var_dims_by_name_ver_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;
    
    if (args.num_dims == 1) {
        query = "select count (*) from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min "; 
    }
    else if(args.num_dims == 2) {
        query = "select count (*) from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min "
            "and ? <= vad.d1_max and ? >= vad.d1_min "; 
    }
    else if(args.num_dims == 3) {
        query = "select count (*) from var_attribute_catalog vac "
            "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
            "inner join type_catalog tc on vac.type_id = tc.id and vc.run_id = tc.run_id "
            "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
            "where (vac.txn_id = ? or vac.active = 1) "
            "and vc.name = ? and vc.version = ? "
            "and tc.run_id = ? "
            "and vac.timestep_id = ? "
            "and ? <= vad.d0_max and ? >= vad.d0_min "
            "and ? <= vad.d1_max and ? >= vad.d1_min "
            "and ? <= vad.d2_max and ? >= vad.d2_min "; 
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 2, strdup(args.var_name.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 3, args.var_version); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.timestep_id); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
