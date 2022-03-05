#include <OpCatalogAllVarAttributesWithTypeVarDimsByIdMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 * get_database(uint64_t job_id, md_query_type query_type, bool &trans_active);
extern void close_database(uint64_t job_id);




using namespace std;

int md_catalog_all_var_attributes_with_type_var_dims_by_id_stub (sqlite3 *db, const md_catalog_all_var_attributes_with_type_var_dims_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count);

static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_with_type_var_dims_by_id_args &args, uint32_t &count);


WaitingType OpCatalogAllVarAttributesWithTypeVarDimsByIdMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  
    int rc;
    md_catalog_all_var_attributes_with_type_var_dims_by_id_args args;
    std::vector<md_catalog_var_attribute_entry> entries;
    uint32_t count = 0;
    bool trans_active;

    deArchiveMsgFromClient(rdma, incoming_msg, args);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_DEARCHIVE_MSG_FROM_CLIENT);

    sqlite3 *db = get_database(args.job_id, args.query_type, trans_active);
    if(!trans_active) {
        rc = md_catalog_all_var_attributes_with_type_var_dims_by_id_stub(db, args, entries, count);
    }
    else {
        rc = RC_DB_BUSY;
    }

    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_STUB);


    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_CREATE_MSG_FOR_CLIENT);

    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_BY_ID_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}





int md_catalog_all_var_attributes_with_type_var_dims_by_id_stub (sqlite3 *db, const md_catalog_all_var_attributes_with_type_var_dims_by_id_args &args,
                           std::vector<md_catalog_var_attribute_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    string query;

    if (args.num_dims == 1) {
        query = "select vac.* from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
            "and ? <= vac.d0_max and ? >= vac.d0_min "; 
    }
    else if(args.num_dims == 2) {
        query = "select vac.* from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
            "and ? <= vac.d0_max and ? >= vac.d0_min "
            "and ? <= vac.d1_max and ? >= vac.d1_min "; 
    }
    else if(args.num_dims == 3) {
        query = "select vac.* from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
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
            fprintf (stderr, "Error catalog_all_var_attributes_with_type_var_dims_by_id stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            close_database(args.job_id);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, 4 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, 5 + (j * 2), args.dims.at(j).max);
        }
    
        rc = sql_retrieve_var_attrs(stmt, entries);

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_attribute_count (sqlite3 *db, const md_catalog_all_var_attributes_with_type_var_dims_by_id_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;


    string query;
    
    if (args.num_dims == 1) {
        query = "select count (*) from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
            "and ? <= vac.d0_max and ? >= vac.d0_min ";
    }
    else if(args.num_dims == 2) {
        query = "select count (*) from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
            "and ? <= vac.d0_max and ? >= vac.d0_min "
            "and ? <= vac.d1_max and ? >= vac.d1_min ";
    }
    else if(args.num_dims == 3) {
        query = "select count (*) from var_attribute_catalog vac where "
            "(vac.type_id = ?) "
            "and (vac.var_id = ?) "
            "and (vac.timestep_id = ?) "
            "and ? <= vac.d0_max and ? >= vac.d0_min "
            "and ? <= vac.d1_max and ? >= vac.d1_min "
            "and ? <= vac.d2_max and ? >= vac.d2_min ";
    }
 
    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.type_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.var_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.timestep_id); //assert (rc == SQLITE_OK);

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
