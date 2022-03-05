#include <md_local.hh>

extern sqlite3 *db;

using namespace std;



static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count);




int md_catalog_all_timesteps_with_var_attributes_with_type_var_stub (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args,
                           std::vector<md_catalog_timestep_entry> &entries,
                           uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? "
    "group by tmc.id, tmc.run_id";

    rc = get_matching_timestep_count (args, count); assert (rc == RC_OK);

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); 
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error catalog_all_timesteps_with_var_attributes_with_type_var stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        // rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {
            md_catalog_timestep_entry entry;

            entry.timestep_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.active = sqlite3_column_int (stmt, 3);
            entry.txn_id = sqlite3_column_int64 (stmt, 4);
            // cout << "attr.dims.at(2).min: " << attribute.dims.at(2).min << " attr.dims.at(2).min: " << attribute.dims.at(2).max << endl;

            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
        

cleanup:

    return rc;
}



static int get_matching_timestep_count (const md_catalog_all_timesteps_with_var_attributes_with_type_var_args &args, uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
    "inner join type_catalog tc on tc.run_id = tmc.run_id "
    "inner join var_attribute_catalog vac on vac.timestep_id = tmc.id and vac.type_id = tc.id "
    "where (vac.txn_id = ? or vac.active = 1) "
    "and (tmc.txn_id = ? or tmc.active = 1) "
    "and vac.type_id = ? "
    "and vac.var_id = ? "
    "and tmc.run_id = ? ) as internalQuery";

    rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.type_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 5, args.run_id); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // cout << "count: " << count << endl;

    return rc;
}
