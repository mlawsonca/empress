#include <OpCatalogAllVarAttributesWithDimsMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t txn_id);
extern sqlite3 *main_db;


using namespace std;

int md_catalog_all_var_attributes_with_dims_stub (sqlite3 *db, const md_catalog_all_var_attributes_with_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count);

static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_with_dims_args &args, uint32_t &count);



WaitingType OpCatalogAllVarAttributesWithDimsMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_var_attributes_with_dims_args sql_args;
    std::vector<md_catalog_var_attribute_entry> entries;
    uint32_t count;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

    if(sql_args.query_type == COMMITTED) {
        rc = md_catalog_all_var_attributes_with_dims_stub(main_db, sql_args, entries, count);
    }
    else if(sql_args.query_type == UNCOMMITTED) {
        sqlite3 *db = get_database(sql_args.txn_id);
        rc = md_catalog_all_var_attributes_with_dims_stub(db, sql_args, entries, count);
    }
    else if(sql_args.query_type == COMMITTED_AND_UNCOMMITTED) {
        uint32_t temp_count;
        
        sqlite3 *db = get_database(sql_args.txn_id);

        rc = md_catalog_all_var_attributes_with_dims_stub(main_db, sql_args, entries, count);
        if(rc == RC_OK) {
            rc = md_catalog_all_var_attributes_with_dims_stub(db, sql_args, entries, temp_count);
            count += temp_count;
        }
    }
    else {
        cout << "ERROR. Query type " << sql_args.query_type << " is not defined" << endl;
    }
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_var_attributes_with_dims_stub (sqlite3 *db, const md_catalog_all_var_attributes_with_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select * from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min ";
    }
    else if(args.num_dims == 2) {
        query = "select * from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min ";
    }
    else if(args.num_dims == 3) {
        query = "select * from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min ";
    }
    else {
        count = 0;
        return rc;
    }   

    rc = get_matching_attribute_count (db, args, count); //assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_dims: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);
        // cout << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << endl;

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 3 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).max);
            // cout << "dims["<< j << "] min: " << args.dims.at(j).min << endl;
            // cout << "dims["<< j << "] max: " << args.dims.at(j).max << endl;
        }

        rc = sql_retrieve_var_attrs(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_with_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select count (*) from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min ";
    }
    else if(args.num_dims == 2) {
        query = "select count (*) from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min ";
    }
    else if(args.num_dims == 3) {
        query = "select count (*) from var_attribute_catalog vac "
        "where vac.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vac.d0_max and ? >= vac.d0_min "
        "and ? <= vac.d1_max and ? >= vac.d1_min "
        "and ? <= vac.d2_max and ? >= vac.d2_min ";
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error get all attrs by dim: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
// cout << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << 
//     " num_dims: " << args.num_dims << " args.dims.size(): " << args.dims.size() << endl;


    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 3 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).max);
        // cout << "min[" << j << "]: " << args.dims.at(j).min << " and max[ " << j << "]: " << args.dims.at(j).max << endl;
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
