

#ifndef CLIENTTIMINGCONSTANTSWRITE_HH
#define CLIENTTIMINGCONSTANTSWRITE_HH


//unsigned short -> max value: 65535
enum CLIENT_TIMING_CONSTANTS_WRITE : unsigned short {
    ERR_ARGC = 10000,
    ERR_DIRMAN = 10001,
    ERR_CREATE_TIMESTEP = 10002,
    ERR_CREATE_RUN = 10003,
    ERR_ACTIVATE_RUN = 10004,
    ERR_CREATE_TYPE = 10005,

    ERR_CREATE_TYPE_BATCH = 10006,
    ERR_ACTIVATE_TYPE = 10007,
    ERR_CREATE_VAR = 10008,
    ERR_CREATE_OBJ_NAMES_AND_DATA = 10009,
    ERR_TYPE_COMPARE = 10010,
    ERR_INSERT_VAR_ATTR = 10011,

    ERR_CREATE_VAR_BATCH = 10012,
    ERR_INSERT_VAR_ATTR_BATCH = 10013,
    ERR_INSERT_TIMESTEP_ATTR_BATCH = 10014,
    ERR_INSERT_TIMESTEP_ATTR = 10015,
    ERR_ACTIVATE_TIMESTEP = 10016,
    ERR_ACTIVATE_VAR = 10017,

    ERR_ACTIVATE_VAR_ATTR = 10018,
    ERR_ACTIVATE_TIMESTEP_ATTR = 10019,
    ERR_INSERT_RUN_ATTR_BATCH = 10020,
    ERR_INSERT_RUN_ATTR = 10021,
    ERR_ACTIVATE_RUN_ATTR = 10022,

    PROGRAM_START = 0,
    MPI_INIT_DONE = 1,
    METADATA_CLIENT_INIT_DONE = 2,
    INIT_VARS_DONE = 3,
    DIRMAN_SETUP_DONE = 4,
    SERVER_SETUP_DONE_INIT_DONE = 5,

    WRITING_START = 6,
    OBJECTOR_LOAD_START = 7,
    OBJECTOR_LOAD_DONE = 8,
    CREATE_TIMESTEPS_START = 9,
    CREATE_NEW_TIMESTEP_START = 10,
    CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_START = 11,
    BOUNDING_BOX_TO_OBJ_NAMES_START = 12,
    BOUNDING_BOX_TO_OBJ_NAMES_DONE = 13,

    CREATE_VARS_AND_ATTRS_FOR_NEW_TIMESTEP_DONE = 14,
    CREATE_TIMESTEP_DONE = 15,
    CREATE_AND_ACTIVATE_TIMESTEP_DONE = 16,
    CREATE_ALL_TIMESTEPS_DONE = 17,
    WRITING_DONE = 18,
    WRITING_DONE_FOR_ALL_PROCS_START_CLEANUP = 19,

    SERVER_SHUTDOWN_DONE = 20,
    DIRMAN_SHUTDOWN_DONE = 21,
    CREATE_NEW_RUN_START = 22,
    CREATE_NEW_RUN_DONE = 23,
    CREATE_TYPES_START = 24,
    CREATE_TYPES_DONE = 25,

    CREATE_VAR_AND_ATTRS_FOR_NEW_VAR_START = 26,
    CREATE_OBJS_AND_STORE_IN_MAP_START = 27,
    CREATE_OBJS_AND_STORE_IN_MAP_DONE = 28,
    CHUNK_MAX_MIN_FIND_START = 29,
    CHUNK_MAX_MIN_FIND_DONE = 30,
    CREATE_VAR_ATTRS_FOR_NEW_VAR_START = 31,
    CREATE_NEW_VAR_ATTR_START = 32,
    VAR_ATTR_INIT_DONE = 33,

    CREATE_NEW_VAR_ATTR_DONE = 34,
    CREATE_VAR_ATTRS_FOR_NEW_VAR_DONE = 35,
    CREATE_TIMESTEP_ATTRS_START = 36,
    GATHER_DONE_PROC_MAX_MIN_FIND_START = 37,
    PROC_MAX_MIN_FIND_DONE = 38,
    CREATE_TIMESTEP_ATTRS_DONE = 39,

    CREATE_RUN_ATTRS_START = 40,
    TIMESTEP_MAX_MIN_FIND_START = 41,
    TIMESTEP_MAX_MIN_FIND_DONE = 42,
    CREATE_RUN_ATTRS_DONE = 43,
    WRITE_CHUNK_DATA_START = 44,
    CREATE_DATA_START = 45,
    CREATE_DATA_DONE = 46,

    WRITE_CHUNK_DATA_DONE = 47,
    HDF5_CREATE_TIMESTEP_START = 48,
    HDF5_CREATE_TIMESTEP_DONE = 49,
    HDF5_CREATE_VARS_DONE = 50,
    HDF5_OPEN_TIMESTEP_COLLECTIVELY_START = 51,
    HDF5_OPEN_TIMESTEP_COLLECTIVELY_DONE = 52,

    HDF5_CREATE_CHUNKED_VAR_START = 53,
    HDF5_CREATE_CHUNKED_VAR_DONE = 54,


    BOUNDING_BOX_TO_OBJ_NAMES = 9999,

};

#endif 

