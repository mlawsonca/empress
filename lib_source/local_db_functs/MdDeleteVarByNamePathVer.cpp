#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_delete_var_by_name_path_ver_stub (const md_delete_var_by_name_path_ver_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    const char * query1 = "delete from var_attribute_catalog where var_attribute_catalog.id in (select vac.id from var_attribute_catalog vac "
    "inner join type_catalog tc on vac.type_id = tc.id "
    "inner join var_catalog vc on vac.var_id = vc.id and vac.timestep_id = vc.timestep_id and tc.run_id = vc.run_id "
    "where vc.run_id = ? "
    "and vc.timestep_id = ? "
    "and vc.name = ? "
    "and vc.path = ? "
    "and vc.version = ?) ";

    const char * query2 = "delete from var_catalog where "
    "run_id = ? "
    "and timestep_id = ? "
    "and name = ? "
    "and path = ? "
    "and version = ? ";

    // cout << "deleting var where run_id: " << args.run_id << " timestep_id: " << args.timestep_id << " name: " << 
    //     args.name << " path: " << args.path << " ver: " << args.version << endl;

    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin delete_var_by_name_path_ver_stub Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_var_by_name_path_ver_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, args.version); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_var_by_name_path_ver_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 3, strdup(args.name.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_text (stmt, 4, strdup(args.path.c_str()), -1, free); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int (stmt, 5, args.version); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt);

    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end delete_var_by_name_path_ver_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

cleanup:
    if (rc != SQLITE_OK)
    {
        rc = sqlite3_exec (db, "rollback;", callback, 0, &ErrMsg);
    }

    return rc;
}
