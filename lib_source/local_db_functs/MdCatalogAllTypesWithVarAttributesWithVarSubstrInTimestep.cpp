#include <md_local.hh>

extern sqlite3 *db;

using namespace std;



int md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub (md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_args &args, 
                          std::vector<md_catalog_type_entry> &entries,
                          uint32_t &count)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    const char * query = "select tc.* from type_catalog tc "
      "inner join var_attribute_catalog vac on tc.id = vac.type_id "
      "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
      "where (vac.txn_id = ? or vac.active = 1) "
      "and (tc.txn_id = ? or tc.active = 1) "
      "and tc.run_id = ? "
      "and vac.timestep_id = ? "
      "and vc.name like ? "
      "group by tc.id";

    size_t size = 0;

    rc = sqlite3_prepare_v2 (db, "select count(*) from (select distinct tc.id from type_catalog tc "
      "inner join var_attribute_catalog vac on tc.id = vac.type_id "
      "inner join var_catalog vc on vac.timestep_id = vc.timestep_id and vac.var_id = vc.id "
      "where (vac.txn_id = ? or vac.active = 1) "
      "and (tc.txn_id = ? or tc.active = 1) "
      "and tc.run_id = ? "
      "and vac.timestep_id = ? "
      "and vc.name like ? ) as internalQuery", -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 5, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    count = sqlite3_column_int64 (stmt, 0);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

    // char *err_msg = 0;
    // char *sql = "SELECT * FROM type_catalog";
    // cout << "starting type catalog \n";
    // rc = sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with type catalog \n";

    if (count > 0) {
        entries.reserve(count);

        rc = sqlite3_prepare_v2 (db, query, -1, &stmt, &tail); assert (rc == SQLITE_OK);
        if (rc != SQLITE_OK)
        {
            fprintf (stderr, "Error md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub: Line: %d SQL error (%d): %s\n", __LINE__, rc, sqlite3_errmsg (db));
            sqlite3_close (db);
            goto cleanup;
        }
        rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 2, args.txn_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 3, args.run_id); assert (rc == SQLITE_OK);
        rc = sqlite3_bind_int64 (stmt, 4, args.timestep_id); assert (rc == SQLITE_OK);
    	rc = sqlite3_bind_text (stmt, 5, strdup(args.var_name_substr.c_str()), -1, free); assert (rc == SQLITE_OK);


        rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

        // std::// cout << "Server thinks there are " << count << " types \n";

        while (rc == SQLITE_ROW)
        {
            md_catalog_type_entry entry;


            entry.type_id = sqlite3_column_int64 (stmt, 0);
            entry.run_id = sqlite3_column_int64 (stmt, 1);
            entry.name = (char *)sqlite3_column_text (stmt, 2);
            entry.version = sqlite3_column_int (stmt, 3);
            entry.active = sqlite3_column_int (stmt, 4);
            entry.txn_id = sqlite3_column_int64 (stmt, 5);

    //printf ("id: %d name: %s path: %s version: %d num_dims: %d ", id, entry [i].name, entry [i].path, entry [i].version, entry [i].num_dims);
            // std::// cout << "in md_catalog_all_types_with_var_attributes_with_var_substr_in_timestep_stub the given entry has name " << entry.name << std::endl;
    //printf ("\n");

            rc = sqlite3_step (stmt);
            entries.push_back(entry);

        }

        rc = sqlite3_finalize (stmt);   
    }
//printf ("rows in var_catalog: %d\n", count);
       

cleanup:

    return rc;
}
