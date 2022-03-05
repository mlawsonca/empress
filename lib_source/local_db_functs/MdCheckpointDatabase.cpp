#include <md_local.hh>

extern sqlite3 *db;

int db_checkpoint_copy(int rank, uint64_t job_id);
int db_checkpoint_incremental_output(int rank, uint64_t job_id);
int db_checkpoint_copy_and_delete(int rank, uint64_t job_id);
int db_checkpoint_copy_and_reset(int rank, uint64_t job_id, md_write_type write_type);
int db_checkpoint_incremental_output_and_delete(int rank, uint64_t job_id);
int db_checkpoint_incremental_output_and_reset(int rank, uint64_t job_id, md_write_type write_type);

using namespace std;



int md_checkpoint_database_stub (int rank, uint64_t job_id, md_db_checkpoint_type checkpt_type, md_write_type write_type)
{
	int rc;

	switch(checkpt_type) {
		case DB_COPY: {
			rc = db_checkpoint_copy(rank, job_id);
			break;
		}
		case DB_INCR_OUTPUT: {
			rc = db_checkpoint_incremental_output(rank, job_id);
			break;
		}	   		
		case DB_COPY_AND_DELETE: {
			rc = db_checkpoint_copy_and_delete(rank, job_id);
			break;
		}	    		
		case DB_COPY_AND_RESET: {
			rc = db_checkpoint_copy_and_reset(rank, job_id, write_type);
			break;
		}	    		
		case DB_INCR_OUTPUT_AND_DELETE: {
			rc = db_checkpoint_incremental_output_and_delete(rank, job_id);
			break;
		}	    		
		case DB_INCR_OUTPUT_AND_RESET: {
			rc = db_checkpoint_incremental_output_and_reset(rank, job_id, write_type);
			break;
		}
	}

	return rc;
}

	    	