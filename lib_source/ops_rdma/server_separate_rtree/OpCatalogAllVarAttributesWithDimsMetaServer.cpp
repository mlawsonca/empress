#include <OpCatalogAllVarAttributesWithDimsMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_var_attributes_with_dims_stub (const md_catalog_all_var_attributes_with_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count);

static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_dims_args &args, uint32_t &count);



WaitingType OpCatalogAllVarAttributesWithDimsMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_var_attributes_with_dims_args sql_args;
    std::vector<md_catalog_var_attribute_entry> attribute_list;
    uint32_t count;

    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_var_attributes_with_dims_stub(sql_args, attribute_list, count);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_STUB);


    std::string serial_str = serializeMsgToClient(attribute_list, count, rc);
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





int md_catalog_all_var_attributes_with_dims_stub (const md_catalog_all_var_attributes_with_dims_args &args,
                           std::vector<md_catalog_var_attribute_entry> &attribute_list,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and tc.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vad.d0_max and ? >= vad.d0_min ";
    }
    else if(args.num_dims == 2) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and tc.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min ";
    }
    else if(args.num_dims == 3) {
        query = "select vac.*, vad.* from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
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

    rc = get_matching_attribute_count (args, count); //assert (rc == RC_OK);

    if (count > 0) {
        attribute_list.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_var_attributes_with_dims: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);
        // cout << "txn_id: " << args.txn_id << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << endl;

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
            // cout << "dims["<< j << "] min: " << args.dims.at(j).min << endl;
            // cout << "dims["<< j << "] max: " << args.dims.at(j).max << endl;
        }

        rc = sql_retrieve_var_attrs(stmt, attribute_list);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (const md_catalog_all_var_attributes_with_dims_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    if(args.num_dims == 1) {
        query = "select count (*) from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and tc.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vad.d0_max and ? >= vad.d0_min ";
    }
    else if(args.num_dims == 2) {
        query = "select count (*) from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and tc.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min ";
    }
    else if(args.num_dims == 3) {
        query = "select count (*) from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
        "where (vac.txn_id = ? or vac.active = 1) "
        "and tc.run_id = ? "
        "and vac.timestep_id = ? "
        "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min "
        "and ? <= vad.d2_max and ? >= vad.d2_min ";
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error get all attrs by dim: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
// cout << "txn_id: " << args.txn_id << " run_id: " << args.run_id << " timestep_id: " << args.timestep_id << 
//     " num_dims: " << args.num_dims << " args.dims.size(): " << args.dims.size() << endl;


    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.run_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
        // cout << "min[" << j << "]: " << args.dims.at(j).min << " and max[ " << j << "]: " << args.dims.at(j).max << endl;
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    return rc;
}
