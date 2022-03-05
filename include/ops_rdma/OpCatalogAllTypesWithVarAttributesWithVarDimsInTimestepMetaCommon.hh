


#ifndef OPCATALOGALLTYPESWITHVARATTRIBUTESWITHVARDIMSINTIMESTEPMETA_HH
#define OPCATALOGALLTYPESWITHVARATTRIBUTESWITHVARDIMSINTIMESTEPMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta : public OpCore {

 

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta");
        inline const static std::string  op_name = "OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta";  

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta(opbox::net::peer_ptr_t peer, const md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args &args);
  // OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta(op_create_as_target_t t);
  // ~OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_type_entry> &entries, uint32_t count, int return_value);

  
};

#endif // OPCATALOGALLTYPESWITHVARATTRIBUTESWITHVARDIMSINTIMESTEPMETA_HH
