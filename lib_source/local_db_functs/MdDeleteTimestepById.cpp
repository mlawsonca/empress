#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);


int md_delete_timestep_by_id_stub (const md_delete_timestep_by_id_args &args)
{
    int rc = RC_OK;
    char * ErrMsg = NULL;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    //fix - would we ever want to delete a timestep but not its vars/attrs?
    const char * query1 = "delete from timestep_attribute_catalog where timestep_attribute_catalog.id in (select tac.id from timestep_attribute_catalog tac "
        "inner join type_catalog tc on tac.type_id = tc.id "
        "where tc.run_id = ? "
        "and tac.timestep_id = ? ) ";

    const char * query2 = "delete from var_attribute_catalog where var_attribute_catalog.id in (select vac.id from var_attribute_catalog vac "
        "inner join type_catalog tc on vac.type_id = tc.id "
        "where tc.run_id = ? "
        "and vac.timestep_id = ? ) ";

    const char * query3 = "delete from var_catalog where run_id = ? and timestep_id = ? ";

    const char * query4 = "delete from timestep_catalog where run_id = ? and id = ? ";


    rc = sqlite3_exec (db, "begin;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error begin delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_prepare_v2 (db, query1, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error first query delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query2, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error second query delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query3, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error third query delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);

    rc = sqlite3_prepare_v2 (db, query4, -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error fourth query delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }
    rc = sqlite3_bind_int64 (stmt, 1, args.run_id); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 2, args.timestep_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt);


    rc = sqlite3_exec (db, "end;", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error end query delete_timestep_by_id_stub: Line: %d SQL error: %s\n", __LINE__, ErrMsg);
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

