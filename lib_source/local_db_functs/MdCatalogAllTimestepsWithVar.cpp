#include <md_local.hh>

extern sqlite3 *db;

using namespace std;


int md_catalog_all_timesteps_with_var_stub (md_catalog_all_timesteps_with_var_args &args, 
                     std::vector<md_catalog_timestep_entry> &entries,
                     uint32_t &count
                     )
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tmc.* from timestep_catalog tmc "
    "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    "where (tmc.txn_id = ? or tmc.active = 1) "
    "and (vc.txn_id = ? or vc.active = 1) "
    "and tmc.run_id = ? "
    "and vc.id = ? "
    "group by tmc.id, tmc.run_id "; 
    // "group by id, run_id "; 

    size_t size = 0;

    // rc = sqlite3_prepare_v2 (db, "select count(distinct timestep_catalog.id timestep_catalog.run_id) from timestep_catalog tmc "
    //   "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
    //   "where (tmc.txn_id = ? or tmc.active = 1) "
    //   "and (vc.txn_id = ? or vc.active = 1) "
    //   "and tmc.run_id = ? "
    //   "and vc.id = ? ", -1, &stmt, &tail); 
      rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tmc.id, tmc.run_id from timestep_catalog tmc "
      "inner join var_catalog vc on tmc.run_id = vc.run_id and tmc.id = vc.timestep_id "
      "where (tmc.txn_id = ? or tmc.active = 1) "
      "and (vc.txn_id = ? or vc.active = 1) "
      "and tmc.run_id = ? "
      "and vc.id = ? ) as internalQuery", -1, &stmt, &tail); 
          if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error count md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
    // assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);



//printf ("rows in run_catalog: %d\n", count);
    if (count > 0) {
      entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_timesteps_with_var_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.var_id); assert (rc == SQLITE_OK);
        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        while (rc == SQLITE_ROW)
        {
            int j = 0;

            md_catalog_timestep_entry entry;


            entry.timestep_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.path = (char *)sqlite3_column_text (stmt, 2);
            entry.active = sqlite3_column_int (stmt, 3);
            entry.txn_id = sqlite3_column_int64 (stmt, 4);

    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
    //printf ("\n");
            rc = sqlite3_step (stmt);
            entries.push_back(entry);
        }

        rc = sqlite3_finalize (stmt);  
    }
    

cleanup:

    return rc;
}
