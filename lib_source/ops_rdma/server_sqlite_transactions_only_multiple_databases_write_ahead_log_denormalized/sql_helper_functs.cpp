#include <my_metadata_args.h>
#include <sqlite3.h>
#include <iostream>

using namespace std;

int sql_retrieve_var_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_var_attribute_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_var_attribute_entry attribute;

        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.run_id = sqlite3_column_int64 (stmt, 1);
        attribute.timestep_id = sqlite3_column_int64 (stmt, 2);
        attribute.type_id = sqlite3_column_int64 (stmt, 3);
        attribute.var_id = sqlite3_column_int64 (stmt, 4);
        attribute.num_dims = sqlite3_column_int (stmt, 5);
        // cout << "in sql_helper_functs num_dims: " << attribute.num_dims << endl;
        attribute.dims.reserve(attribute.num_dims);
        int j = 0;
        while (j < attribute.num_dims)
        {
            md_dim_bounds bounds;
            bounds.min =  sqlite3_column_double (stmt, 6 + (j * 2));
            bounds.max = sqlite3_column_double (stmt, 7 + (j * 2));
            attribute.dims.push_back(bounds);
            j++;
        }
        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 12);   
        switch(attribute.data_type) {  
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, 13));                                                                               
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, 13));                                                                              
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, 13);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, 13);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, 13),(char *)sqlite3_column_text (stmt, 13) + size_blob);
                break;
            }
        }                         

        rc = sqlite3_step (stmt);
        entries.push_back(attribute);
    }

    return rc;
}

    
int sql_retrieve_run_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_run_attribute_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_run_attribute_entry attribute;

        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.run_id = sqlite3_column_int64 (stmt, 1);
        attribute.type_id = sqlite3_column_int64 (stmt, 2);
        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 3);

        switch(attribute.data_type) {
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, 4));
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, 4));
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, 4);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, 4);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, 4),(char *)sqlite3_column_text (stmt, 4) + size_blob);
                break;
            }
        }            

        rc = sqlite3_step (stmt);
        entries.push_back(attribute);
    }

    return rc;
}

int sql_retrieve_timestep_attrs(sqlite3_stmt * stmt, std::vector<md_catalog_timestep_attribute_entry> &entries)
{
    int rc;
    rc = sqlite3_step (stmt); //assert (rc == SQLITE_ROW || rc == SQLITE_DONE);

    while (rc == SQLITE_ROW)
    {
        md_catalog_timestep_attribute_entry attribute;

        attribute.attribute_id = sqlite3_column_int64 (stmt, 0);
        attribute.run_id = sqlite3_column_int64 (stmt, 1);
        attribute.timestep_id = sqlite3_column_int64 (stmt, 2);
        attribute.type_id = sqlite3_column_int64 (stmt, 3);
        attribute.data_type = (attr_data_type) sqlite3_column_int (stmt, 4);
        switch(attribute.data_type) {
            // case ATTR_DATA_TYPE_NULL : {
            //     break;
            // }        
            case ATTR_DATA_TYPE_INT : {
                attribute.data = std::to_string(sqlite3_column_int64 (stmt, 5));
                break;
            }
            case ATTR_DATA_TYPE_REAL : {
                attribute.data = std::to_string(sqlite3_column_double (stmt, 5));                    
                break;
            }
            case ATTR_DATA_TYPE_TEXT : {
                attribute.data = (char *)sqlite3_column_text (stmt, 5);
                break;
            }
            case ATTR_DATA_TYPE_BLOB : {
                int size_blob = sqlite3_column_bytes (stmt, 5);
                
                attribute.data.assign((char *)sqlite3_column_text (stmt, 5),(char *)sqlite3_column_text (stmt, 5) + size_blob);
                break;
            }
        }                        

        rc = sqlite3_step (stmt);
        entries.push_back(attribute);
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
        entry.npx = sqlite3_column_int (stmt, 4);
        entry.npy = sqlite3_column_int (stmt, 5);
        entry.npz = sqlite3_column_int (stmt, 6);


        int size_rank_funct = sqlite3_column_bytes (stmt, 7);
        entry.rank_to_dims_funct.assign((char *)sqlite3_column_text (stmt, 7),(char *)sqlite3_column_text (stmt, 7) + size_rank_funct);
        int size_obj_funct = sqlite3_column_bytes (stmt, 8);
        entry.objector_funct.assign((char *)sqlite3_column_text (stmt, 8),(char *)sqlite3_column_text (stmt, 8) + size_obj_funct);

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
        entry.num_dims = sqlite3_column_int (stmt, 7);
        entry.dims.reserve(entry.num_dims);
        int j = 0;
        while (j < entry.num_dims)
        {
            md_dim_bounds bounds;
            bounds.min =  sqlite3_column_double (stmt, 8 + (j * 2));
            bounds.max = sqlite3_column_double (stmt, 9 + (j * 2));
            entry.dims.push_back(bounds);
            j++;
        }

        rc = sqlite3_step (stmt);
        entries.push_back(entry);
    }

    return rc;
}

