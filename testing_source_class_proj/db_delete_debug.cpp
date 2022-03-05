#include <stdlib.h>
#include <iostream>
#include <assert.h>
#include <sqlite3.h>
#include <string.h>

sqlite3 * db = NULL;

using namespace std;

static int metadata_database_init ();
static int insert_into_db ();
static int catalog_seq();
static int catalog_table();
static int get_seq_value();
static int update_seq(uint64_t new_seq_start);
static int delete_db();

// int main() {
// 	metadata_database_init ();
// 	insert_into_db ();
// 	catalog_table();
// 	// catalog_seq();
// 	uint64_t old_seq_value = get_seq_value();

// 	sqlite3_close (db);

// 	cout << "initing new db" << endl;
// 	metadata_database_init ();
// 	catalog_seq();
// 	update_seq(old_seq_value);
// 	insert_into_db ();
// 	catalog_table();
// 	catalog_seq();
// 	sqlite3_close (db);

// }

int main() {
	metadata_database_init ();
	insert_into_db ();
	catalog_table();
	
	cout << "about to delete" << endl;
	delete_db();
	catalog_table();
	cout << "about to insert" << endl;
	insert_into_db ();
	catalog_table();
	catalog_seq();
	sqlite3_close (db);

}




static int callback (void * NotUsed, int argc, char ** argv, char ** ColName)
{
    int i;
    for (i = 0; i < argc; i++)
    {
        printf ("%s = %s\n", ColName [i], argv [i] ? argv [i] : "NULL");
    }
    printf ("\n");
    return 0;
}


static int update_seq(uint64_t old_seq_value)
{
	int rc;
	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

	string catalog_name = "catalog";

	rc = sqlite3_prepare_v2 (db, "insert into sqlite_sequence (name,seq) values (?, ?)", -1, &stmt, &tail); assert(rc == SQLITE_OK);
	rc = sqlite3_bind_text (stmt, 1, strdup(catalog_name.c_str()), -1, free); assert (rc == SQLITE_OK);
	rc = sqlite3_bind_int64 (stmt, 2, old_seq_value); assert (rc == SQLITE_OK); 
	rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
}

static int catalog_seq()
{
	int rc;
	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

	rc = sqlite3_prepare_v2 (db, "select * from sqlite_sequence", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of select run attr seq. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    if(rc != SQLITE_ROW) {
    	cout << "catalog seq is empty" << endl;
    }
	while (rc == SQLITE_ROW) {
		string name = (char *)sqlite3_column_text (stmt, 0);
		uint64_t value = (uint64_t)sqlite3_column_int64 (stmt, 1);
		cout << "table: " << name << " seq value: " << value << endl;
		rc = sqlite3_step (stmt);
	}
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
}


static int get_seq_value()
{
	int rc;
	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

    uint64_t value;

	rc = sqlite3_prepare_v2 (db, "select * from sqlite_sequence", -1, &stmt, &tail);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error start of select run attr seq. Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        return rc;
    }
    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	if (rc == SQLITE_ROW) {
		string name = (char *)sqlite3_column_text (stmt, 0);
		value = (uint64_t)sqlite3_column_int64 (stmt, 1);
		cout << "table: " << name << " seq value: " << value << endl;
		rc = sqlite3_step (stmt);
	}
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
    return value;
}

static int catalog_table()
{
	int rc;
	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

	rc = sqlite3_prepare_v2 (db, "select * from catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
	while (rc == SQLITE_ROW) {
		uint64_t id = (uint64_t)sqlite3_column_int64 (stmt, 0);
		uint64_t value = (uint64_t)sqlite3_column_int64 (stmt, 1);
		cout << "id: " << id << " value: " << value << endl;
		rc = sqlite3_step (stmt);
	}
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
}
	

static int insert_into_db ()
{

	int values[] = {5,6,7};

    int rc;
    int i = 0;
    sqlite3_stmt * stmt_index = NULL;
    const char * tail_index = NULL;

    rc = sqlite3_prepare_v2 (db, "insert into catalog (id, value) values (?, ?)", -1, &stmt_index, &tail_index);
    if (rc != SQLITE_OK)
    {
        fprintf (stderr, "Error at start of insert_var_object_stub: Line: %d SQL error: %s\n", __LINE__, sqlite3_errmsg (db));
        sqlite3_close (db);
        goto cleanup;
    }

    for(int i = 0; i < 3; i++) {

	    rc = sqlite3_bind_null (stmt_index, 1); assert (rc == SQLITE_OK);
	    rc = sqlite3_bind_int64 (stmt_index, 2, values[i]); assert (rc == SQLITE_OK);   
	    rc = sqlite3_step (stmt_index); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    	rc = sqlite3_reset (stmt_index); assert (rc == SQLITE_OK); 
	}

    rc = sqlite3_finalize (stmt_index); assert (rc == SQLITE_OK); 


cleanup:

    return rc;
}


static int delete_db()
{
	int rc;
	sqlite3_stmt * stmt = NULL;
    const char * tail = NULL;

	rc = sqlite3_prepare_v2 (db, "delete from catalog", -1, &stmt, &tail); assert(rc == SQLITE_OK);

    rc = sqlite3_step (stmt); assert (rc == SQLITE_ROW || rc == SQLITE_DONE);
    rc = sqlite3_finalize (stmt); assert (rc == SQLITE_OK);
}



static int metadata_database_init ()
{
    //setup the database
    int rc;
    char  *ErrMsg = NULL;
    
    rc = sqlite3_open (":memory:", &db);
    
    if (rc != SQLITE_OK)
    {   
        fprintf (stderr, "Can't open database: %s\n", sqlite3_errmsg (db));        
        sqlite3_close (db);
        goto cleanup;
    }

    rc = sqlite3_exec (db, "create table catalog (id integer primary key autoincrement not null, value integer)", callback, 0, &ErrMsg);
    if (rc != SQLITE_OK)    {
        fprintf (stdout, "Line: %d SQL error: %s\n", __LINE__, ErrMsg);
        sqlite3_free (ErrMsg);
        sqlite3_close (db);
        goto cleanup;
    }


cleanup:
    return rc;
}