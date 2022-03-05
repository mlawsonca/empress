#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count);

static int get_matching_timestep_count (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args, uint32_t &count);


WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarDimsMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args sql_args;
    std::vector<md_catalog_timestep_entry> entries;
    uint32_t count;
    
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub(main_db, sql_args, entries, count);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub(db, sql_args, entries, count);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {
        uint32_t temp_count;
        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub(main_db, sql_args, entries, count);
        if(rc == RC_OK) {
            rc = md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub(db, sql_args, entries, temp_count);
            count += temp_count;
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;
    if (args.num_dims == 1) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 2) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "group by tmc.id, tmc.run_id";
    }
    else if(args.num_dims == 3) {
        query = "select tmc.* from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min "; 
        "group by tmc.id, tmc.run_id";      
    }
    else {
        count = 0;
        return rc;
    }

    rc = get_matching_timestep_count (db, args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
        }

        rc = sql_retrieve_timesteps(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (sqlite3 *db, const md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;

    if(args.num_dims == 1) {
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min ) as internalQuery"; 
    }
    else if(args.num_dims == 2) {   
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min ) as internalQuery"; 
    }
    else if(args.num_dims == 3) {   
        query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
        "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.run_id = tmc.run_id "
        "where vac.type_id = ? "
        "and vac.var_id = ? "
        "and tmc.run_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min ) as internalQuery"; 
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
    // if (rc != SQLITE_OK)
    // {
    //     fprintf (stderr, "Error md_catalog_all_timesteps_with_var_attributes_with_type_var_dims_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
    //     sqlite3_close (db);
    //     return rc;
    // }    
    //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);
//printf ("matching attr count: %d\n", *count);

    return rc;
}
