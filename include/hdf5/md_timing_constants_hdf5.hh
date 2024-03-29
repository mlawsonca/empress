



#ifndef MDTIMINGCONSTANTSHDF5_HH
#define MDTIMINGCONSTANTSHDF5_HH

enum MD_TIMING_CONSTANTS_HDF5 : unsigned short {



    MD_CATALOG_ALL_RUN_ATTRIBUTES_START = 1000,
    MD_CATALOG_ALL_RUN_ATTRIBUTES_DONE = 1001,

    MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START = 1100,
    MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_DONE = 1101,

    MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_START = 1200,
    MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_DONE = 1201,

    MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_START = 1300,
    MD_CATALOG_ALL_TIMESTEP_ATTRIBUTES_WITH_TYPE_DONE = 1301,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START = 1400,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE = 1401,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_START = 1500,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE = 1501,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START = 1600,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE = 1601,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START = 1700,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE = 1701,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_START = 1800,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_DONE = 1801,

    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START = 1900,
    MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_DONE = 1901,

    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_START = 2000,
    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_IN_TIMESTEP_DONE = 2001,

    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START = 2100,
    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_DONE = 2101,

    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_START = 2200,
    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_IN_TIMESTEP_DONE = 2201,

    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_START = 2300,
    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_IN_TIMESTEP_DONE = 2301,

    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_START = 2400,
    MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_IN_TIMESTEP_DONE = 2401,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_START = 2500,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_DONE = 2501,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_START = 2600,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_DIMS_DONE = 2601,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_START = 2700,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_DONE = 2701,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_START = 2800,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DONE = 2801,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_START = 2900,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DIMS_DONE = 2901,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_START = 3000,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_DONE = 3001,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_START = 3100,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DIMS_DONE = 3101,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_START = 3200,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_DONE = 3201,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_START = 3300,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DIMS_DONE = 3301,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_START = 3400,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_DONE = 3401,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_START = 3500,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DIMS_DONE = 3501,

    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START = 3600,
    MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_DONE = 3601,

    MD_CATALOG_TIMESTEP_START = 3700,
    MD_CATALOG_TIMESTEP_DONE = 3701,

    MD_CATALOG_VAR_START = 3800,
    MD_CATALOG_VAR_DONE = 3801,

    MD_CREATE_CHUNKED_VAR_START = 3900,
    MD_CREATE_CHUNKED_VAR_DONE = 3901,

    MD_CREATE_RUN_START = 4000,
    MD_CREATE_RUN_DONE = 4001,

    MD_CREATE_TIMESTEP_START = 4100,
    MD_CREATE_TIMESTEP_DONE = 4101,

    MD_CREATE_VAR_START = 4200,
    MD_CREATE_VAR_DONE = 4201,

    MD_INSERT_RUN_ATTRIBUTE_BATCH_START = 4300,
    MD_INSERT_RUN_ATTRIBUTE_BATCH_DONE = 4301,

    MD_INSERT_RUN_ATTRIBUTE_START = 4400,
    MD_INSERT_RUN_ATTRIBUTE_DONE = 4401,

    MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START = 4500,
    MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_DONE = 4501,

    MD_INSERT_TIMESTEP_ATTRIBUTE_START = 4600,
    MD_INSERT_TIMESTEP_ATTRIBUTE_DONE = 4601,

    MD_INSERT_VAR_ATTRIBUTE_BATCH_START = 4700,
    MD_INSERT_VAR_ATTRIBUTE_BATCH_DONE = 4701,

    MD_INSERT_VAR_ATTRIBUTE_START = 4800,
    MD_INSERT_VAR_ATTRIBUTE_DONE = 4801,


};

#endif 

