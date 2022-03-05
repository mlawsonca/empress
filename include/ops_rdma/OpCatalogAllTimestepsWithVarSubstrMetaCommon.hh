


#ifndef OPCATALOGALLTIMESTEPSWITHVARSUBSTRMETA_HH
#define OPCATALOGALLTIMESTEPSWITHVARSUBSTRMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogAllTimestepsWithVarSubstrMeta : public OpCore {

 

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogAllTimestepsWithVarSubstrMeta");
        inline const static std::string  op_name = "OpCatalogAllTimestepsWithVarSubstrMeta";   

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogAllTimestepsWithVarSubstrMeta(opbox::net::peer_ptr_t peer, const md_catalog_all_timesteps_with_var_substr_args &args);
  // OpCatalogAllTimestepsWithVarSubstrMeta(op_create_as_target_t t);
  // ~OpCatalogAllTimestepsWithVarSubstrMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_catalog_all_timesteps_with_var_substr_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_timesteps_with_var_substr_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_timestep_entry> &entries, uint32_t count, int return_value);

  
};

#endif // OPCATALOGALLTIMESTEPSWITHVARSUBSTRMETA_HH
