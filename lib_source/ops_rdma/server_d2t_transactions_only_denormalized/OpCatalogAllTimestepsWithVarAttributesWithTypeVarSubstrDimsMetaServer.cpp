#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args, uint32_t &count);


WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrDimsMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args sql_args;
    std::vector<md_catalog_timestep_entry> entries;
    uint32_t count;
    

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);        
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub(sql_args, entries, count);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 2) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 3) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min "
        "group by tmc.id, tmc.run_id";
    }
    else {
        count = 0;
        return rc;
    }    

    rc = get_matching_timestep_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
        // cout << "var_name_substr: " << args.var_name_substr << endl;
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
        }

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min ) as internalQuery"; 
    }
    else if(args.num_dims == 2) {
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min ) as internalQuery"; 
    }
    else if(args.num_dims == 3) {
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "inner join var_catalog vc on vac.run_id = vc.run_id and vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and (tmc.txn_id = ? or tmc.active = 1) "
        "and vac.type_id = ? "
        "and vc.name like ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min ) as internalQuery"; 
    }
    

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 6 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 7 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}
