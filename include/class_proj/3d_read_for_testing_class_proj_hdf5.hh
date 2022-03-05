#ifndef THREEDREADFORTESTINGLCLASSPROJHDF5_HH
#define THREEDREADFORTESTINGLCLASSPROJHDF5_HH

#include <hdf5.h>
#include <my_metadata_client_hdf5.hh>

#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/serialization/vector.hpp>
#include <sstream>


void read_attrs(int rank, uint32_t num_client_procs, std::string timestep_file_name, 
			std::string var_name, std::string type_name,
			uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
			MPI_Comm read_comm
			);


void read_attrs(int rank, uint32_t num_client_procs, std::string timestep_file_name, 
			std::string var_name, std::string type_name, md_dim_bounds query_dims,
			uint16_t catalog_attr_start_code, uint16_t catalog_attr_done_code,
			MPI_Comm read_comm
			);

void read_pattern_1 (int rank, std::string timestep_file_name, const std::vector<std::string> &var_names,
					const std::vector<std::string> &type_names, uint32_t num_client_procs, MPI_Comm read_comm  );

void read_pattern_2 (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
					std::string var_name, uint32_t num_client_procs, MPI_Comm read_comm  );

// 3. all of a few vars (3 for 3-d, for example)
void read_pattern_3 (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
				const std::vector<std::string> &var_names, uint32_t num_client_procs, 
				MPI_Comm read_comm );

// 4. 1 plane in each dimension for 1 variable
// note: this can only be used for 3 dimensional variables
void read_pattern_4 (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
					std::string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm); 



// 5. an arbitrary rectangular subset representing a cubic area of interest
// note - this is currently just set up for 3d vars
void read_pattern_5 (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
					std::string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm);


// 6. an arbitrary area on an orthogonal plane (decomposition dimensions) aka partial plane
// note: this can only be used for 3 dimensional variables
void read_pattern_6 (int rank, std::string timestep_file_name, const std::vector<std::string> &type_names,
					std::string var_name, hsize_t *var_dims, uint32_t num_client_procs, MPI_Comm read_comm); 


#endif //THREEDREADFORTESTINGLCLASSPROJHDF5_HH