#include <md_local.hh>

extern sqlite3 *db;

using namespace std;

int md_activate_stub (const md_activate_args &args)
{
    int rc;
    sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;
    string query;

    switch(args.catalog_type) {
    	case RUN_CATALOG : {
    		query = "update run_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_CATALOG : {
    		query = "update timestep_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case VAR_CATALOG : {
    		query = "update var_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TYPE_CATALOG : {
    		query = "update type_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case RUN_ATTR_CATALOG : {
    		query = "update run_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case TIMESTEP_ATTR_CATALOG : {
    		query = "update timestep_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    	case VAR_ATTR_CATALOG : {
    		query = "update var_attribute_catalog set active = 1 where txn_id = ?";
    		break;
    	}
    }

    rc = sqlite3_prepare_v2 (db, query.c_str(), -1, &stmt, &tail); assert (rc == SQLITE_OK);
    rc = sqlite3_bind_int64 (stmt, 1, args.txn_id); assert (rc == SQLITE_OK);
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);

cleanup:

    return rc;
}