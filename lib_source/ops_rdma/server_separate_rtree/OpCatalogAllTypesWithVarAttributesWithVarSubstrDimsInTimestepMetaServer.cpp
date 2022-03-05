#include <OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMetaCommon.hh>
#include <server_timing_constants_new.hh>
#include <sql_helper_functs.hh>
#include <OpCoreServer.hh>

extern sqlite3 *db;

using namespace std;

int md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count);


WaitingType OpCatalogAllTypesWithVarAttributesWithVarSubstrDimsInTimestepMeta::handleMessage(bool rdma, message_t *incoming_msg) {
  

    md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args sql_args;
    std::vector<md_catalog_type_entry> entries;
    uint32_t count;

    //convert the serialized string back into the args and count pointer
    deArchiveMsgFromClient(rdma, incoming_msg, sql_args); 
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DEARCHIVE_MSG_FROM_CLIENT);

    int rc = md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub(sql_args, entries, count);
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_STUB);

    // std::// cout << "num of entries is " << entries.size() << std::endl;

    std::string serial_str = serializeMsgToClient(entries, count, rc);
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_SERIALIZE_MSG_FOR_CLIENT);

    createOutgoingMessage(dst, 
                          0, //Not expecting a reply
                          dst_mailbox,
                          serial_str);
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_CREATE_MSG_FOR_CLIENT);
    
    opbox::net::SendMsg(peer, ldo_msg);
    state=State::done;
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_SEND_MSG_TO_CLIENT_OP_DONE);
    return WaitingType::done_and_destroy;

}



// extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;
    string count_query;

    if(args.timestep_id == ALL_TIMESTEPS) {
        query = "select tc.* from type_catalog tc "
              "inner join var_attribute_catalog vac on tc.id = vac.type_id "
              "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
             "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
              "where (vac.txn_id = ? or vac.active = 1) "
              "and (tc.txn_id = ? or tc.active = 1) "
              "and tc.run_id = ? "
              "and vc.name like ? ";     

        count_query = "select count(*) from (select distinct tc.id from type_catalog tc  "
              "inner join var_attribute_catalog vac on tc.id = vac.type_id "
              "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
             "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
              "where (vac.txn_id = ? or vac.active = 1) "
              "and (tc.txn_id = ? or tc.active = 1) "
              "and tc.run_id = ? "
              "and vc.name like ? ";
    }
    else {
        query = "select tc.* from type_catalog tc "
              "inner join var_attribute_catalog vac on tc.id = vac.type_id "
              "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
             "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
              "where (vac.txn_id = ? or vac.active = 1) "
              "and (tc.txn_id = ? or tc.active = 1) "
              "and tc.run_id = ? "
              "and vac.timestep_id = ? "
              "and vc.name like ? ";

        count_query = "select count(*) from (select distinct tc.id from type_catalog tc  "
              "inner join var_attribute_catalog vac on tc.id = vac.type_id "
              "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
             "inner join var_attribute_dims vad on vac.id = vad.var_attr_id "
              "where (vac.txn_id = ? or vac.active = 1) "
              "and (tc.txn_id = ? or tc.active = 1) "
              "and tc.run_id = ? "
              "and vac.timestep_id = ? "
              "and vc.name like ? ";
    }
    if(args.num_dims == 1) {
        query += "and ? <= vad.d0_max and ? >= vad.d0_min "
          "group by tc.id"; 

          count_query += "and ? <= vad.d0_max and ? >= vad.d0_min ) as internalQuery";
    }
    else if(args.num_dims == 2) {
        query += "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min "
          "group by tc.id";

          count_query += "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min ) as internalQuery";
    }
    else if(args.num_dims == 3) {
        query += "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min "
        "and ? <= vad.d2_max and ? >= vad.d2_min "
          "group by tc.id";

          count_query += "and ? <= vad.d0_max and ? >= vad.d0_min "
        "and ? <= vad.d1_max and ? >= vad.d1_min "
        "and ? <= vad.d2_max and ? >= vad.d2_min ) as internalQuery";
    }
    else {
        count = 0;
        return rc;
    }    
    rc = sqlite3_prepare_v2 (db, count_query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);

    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
    int index = 4;
    if(args.timestep_id != ALL_TIMESTEPS) {
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);
        index += 1;
    }
    rc = sqlite3_bind_text (stmt, index, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);

    for (int j = 0; j < args.num_dims; j++)
    {
        rc = sqlite3_bind_double (stmt, index+1 + (j * 2), args.dims.at(j).min);
        rc = sqlite3_bind_double (stmt, index+2 + (j * 2), args.dims.at(j).max);
    }

    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); //assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM type_catalog";
    // cout << "starting type catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with type catalog \n";

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); //assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_types_with_var_attributes_with_var_substr_dims_in_timestep_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); //assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); //assert (rc == SQLITE_OK);
        index = 4;
        if(args.timestep_id != ALL_TIMESTEPS) {
            rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); //assert (rc == SQLITE_OK);
            index += 1;
        }
        rc = sqlite3_bind_text (stmt, index, strdup(args.var_name_substr.c_str()), -1, free); //assert (rc == SQLITE_OK);

        for (int j = 0; j < args.num_dims; j++)
        {
            rc = sqlite3_bind_double (stmt, index+1 + (j * 2), args.dims.at(j).min);
            rc = sqlite3_bind_double (stmt, index+2 + (j * 2), args.dims.at(j).max);
        }

        rc = sql_retrieve_types(stmt, entries);

        rc = sqlite3_finalize (stmt);   
    }
//printf ("rows in var_catalog: %d\n", count);
       

cleanup:

    return rc;
}
