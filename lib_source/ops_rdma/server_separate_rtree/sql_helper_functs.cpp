#include <my_metadata_args.h>
#include <sqlite3.h>
#include <iostream>

extern md_db_index_type index_type;
extern int callback (void * NotUsed, int argc, char ** argv, char ** ColName);
extern sqlite3 *db;

using namespace std;


string get_query_beginning() 
{
    if(index_type == INDEX_RTREE) {
        return "select vac.*, vad.* from var_attribute_catalog vac inner join var_attribute_dims vad on vac.id = vad.var_attr_id ";
    }
    else {
        return "select vac.* from var_attribute_catalog vac ";
    }
}

string get_count_query_beginning() 
{
    if(index_type == INDEX_RTREE) {
        return "select count (*) from var_attribute_catalog vac inner join var_attribute_dims vad on vac.id = vad.var_attr_id ";
    }
    else {
        return "select count (*) from var_attribute_catalog vac ";
    }
}

int sql_retrieve_var_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_var_attribute_entry> &attribute_list)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    uint32_t aux_col_start = 1, dim_cols_start = 7, data_cols_start = 13;
    if(index_type == INDEX_RTREE) {
        // dim_cols_start = 9; //first col is the var_attr id which we don't need
        dim_cols_start = 10; 
        data_cols_start = 7;

        //not using the virtual table approach since we can't make indices on auxiliary columns
        // aux_col_start = 7;
        // dim_cols_start = 1;
    }

    // cout << "aux_col_start: " << aux_col_start << ", dim_cols_start: " << dim_cols_start << ", data_cols_start: " << data_cols_start << endl;

// timestep_id: 0 var_id: 10 type_id: 0 active: 19 txn_id: 10 num_dims: 19 x: (0/2) y: (1/0) z: (5/3)


    // char *err_msg = 0;
    // char *sql = "SELECT * FROM var_attribute_dims";
    // cout << "starting var_attribute_dims \n";
    // sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var_attribute_dims \n";

    // char *sql = "SELECT * FROM var_attribute_catalog vac inner join var_attribute_dims vad on vac.id = vad.var_attr_id";
    // cout << "starting var_attribute_dims \n";
    // sqlite3_exec(db, sql, callback, 0, &err_msg);
    // cout << "done with var_attribute_dims \n";



    while (rc == SQLITE_ROW)
    {
        md_catalog_var_attribute_entry attribute;

        // (id, timestep_id, type_id, var_id, active, txn_id, num_dims, data_type, data


        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.timestep_id = sqlite3_column_int64 (stmt, aux_col_start);
        attribute.type_id = sqlite3_column_int64 (stmt, aux_col_start+1);
        attribute.var_id = sqlite3_column_int64 (stmt, aux_col_start+2);
        attribute.active = sqlite3_column_int (stmt, aux_col_start+3);
        attribute.txn_id = sqlite3_column_int64 (stmt, aux_col_start+4);
        attribute.num_dims = sqlite3_column_int (stmt, aux_col_start+5);
        attribute.dims.reserve(attribute.num_dims);

        int j = 0;
        // cout << "dim_cols_start: " << dim_cols_start << ", num_dims: " << attribute.num_dims << endl;
        while (j < attribute.num_dims)
        {
            md_dim_bounds bounds;
            bounds.min =  sqlite3_column_double (stmt, dim_cols_start + (j * 2));
            bounds.max = sqlite3_column_double (stmt, dim_cols_start+1 + (j * 2));

            // cout << "dims[" << (j * 2) << "]: min: " << bounds.min << " max: " << bounds.max << endl;

            attribute.dims.push_back(bounds);
            j++;
        }

        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, data_cols_start);   
        switch(attribute.data_type) {  
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, data_cols_start+1));                                                                                
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, data_cols_start+1));                                                                                
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, data_cols_start+1);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, data_cols_start+1);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, data_cols_start+1),(char *)sqlite3_column_text (stmt, data_cols_start+1) + size_blob);
                break;
            }
        }                        

        rc = sqlite3_step (stmt);
        attribute_list.push_back(attribute);
    }

    return rc;
}

    
int sql_retrieve_run_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_run_attribute_entry> &attribute_list)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_run_attribute_entry attribute;

        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.run_id = sqlite3_column_int64 (stmt, 1);
        attribute.type_id = sqlite3_column_int64 (stmt, 2);
        attribute.active = sqlite3_column_int (stmt, 3);
        attribute.txn_id = sqlite3_column_int64 (stmt, 4);    
        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 5);

        switch(attribute.data_type) {
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, 6));
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, 6));
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, 6);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, 6);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, 6),(char *)sqlite3_column_text (stmt, 6) + size_blob);
                break;
            }
        }            

        rc = sqlite3_step (stmt);
        attribute_list.push_back(attribute);
    }

    return rc;
}

int sql_retrieve_timestep_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_timestep_attribute_entry> &attribute_list)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_timestep_attribute_entry attribute;

        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.timestep_id = sqlite3_column_int64 (stmt, 1);
        attribute.type_id = sqlite3_column_int64 (stmt, 2);
        attribute.active = sqlite3_column_int (stmt, 3);
        attribute.txn_id = sqlite3_column_int64 (stmt, 4);            
        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 5);
        switch(attribute.data_type) {
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, 6));
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, 6));                    
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, 6);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, 6);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, 6),(char *)sqlite3_column_text (stmt, 6) + size_blob);
                break;
            }
        }                        

        rc = sqlite3_step (stmt);
        attribute_list.push_back(attribute);
    }

    return rc;
}

int sql_retrieve_timesteps(sqlite3_stmt * stmt, std::vector<md_catalog_timestep_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_timestep_entry entry;

        entry.timestep_id = sqlite3_column_int64 (stmt, 0);
        entry.run_id = sqlite3_column_int64 (stmt, 1);
        entry.path = (char *)sqlite3_column_text (stmt, 2);
        entry.active = sqlite3_column_int (stmt, 3);
        entry.txn_id = sqlite3_column_int64 (stmt, 4);

        rc = sqlite3_step (stmt);
        entries.push_back(entry);
    }

    return rc;
}

int sql_retrieve_types(sqlite3_stmt * stmt, std::vector<md_catalog_type_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_type_entry entry;

        entry.type_id = sqlite3_column_int64 (stmt, 0);
        entry.run_id = sqlite3_column_int64 (stmt, 1);
        entry.name = (char *)sqlite3_column_text (stmt, 2);
        entry.version = sqlite3_column_int (stmt, 3);
        entry.active = sqlite3_column_int (stmt, 4);
        entry.txn_id = sqlite3_column_int64 (stmt, 5);

        rc = sqlite3_step (stmt);
        entries.push_back(entry);

    }

    return rc;
}

int sql_retrieve_runs(sqlite3_stmt * stmt, std::vector<md_catalog_run_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_run_entry entry;
        entry.run_id = sqlite3_column_int64 (stmt, 0);
        entry.job_id = sqlite3_column_int64 (stmt, 1);
        entry.name = (char *)sqlite3_column_text (stmt, 2);
        entry.date = (char *)sqlite3_column_text (stmt, 3);
        entry.active = sqlite3_column_int (stmt, 4);
        entry.txn_id = sqlite3_column_int64 (stmt, 5);
        entry.npx = sqlite3_column_int (stmt, 6);
        entry.npy = sqlite3_column_int (stmt, 7);
        entry.npz = sqlite3_column_int (stmt, 8);


        int size_rank_funct = sqlite3_column_bytes (stmt, 9);
        entry.rank_to_dims_funct.assign((char *)sqlite3_column_text (stmt, 9),(char *)sqlite3_column_text (stmt, 9) + size_rank_funct);
        int size_obj_funct = sqlite3_column_bytes (stmt, 10);
        entry.objector_funct.assign((char *)sqlite3_column_text (stmt, 10),(char *)sqlite3_column_text (stmt, 10) + size_obj_funct);

        rc = sqlite3_step (stmt);
        entries.push_back(entry);
    }

    return rc;
}


int sql_retrieve_vars(sqlite3_stmt * stmt, std::vector<md_catalog_var_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_var_entry entry;
        
        entry.var_id = sqlite3_column_int64 (stmt, 0);
        entry.run_id = sqlite3_column_int64 (stmt, 1);
        entry.timestep_id = sqlite3_column_int64 (stmt, 2);
        entry.name = (char *)sqlite3_column_text (stmt, 3);
        entry.path = (char *)sqlite3_column_text (stmt, 4);
        entry.version = sqlite3_column_int (stmt, 5);
        entry.data_size = sqlite3_column_int (stmt, 6);
        entry.active = sqlite3_column_int (stmt, 7);
        entry.txn_id = sqlite3_column_int64 (stmt, 8);
        entry.num_dims = sqlite3_column_int (stmt, 9);
        entry.dims.reserve(entry.num_dims);
        int j = 0;
        while (j < entry.num_dims)
        {
            md_dim_bounds bounds;
            bounds.min =  sqlite3_column_double (stmt, 10 + (j * 2));
            bounds.max = sqlite3_column_double (stmt, 11 + (j * 2));
            entry.dims.push_back(bounds);
            j++;
        }

        rc = sqlite3_step (stmt);
        entries.push_back(entry);
    }

    return rc;
}

