


#ifndef OPCATALOGALLVARATTRIBUTESWITHTYPEDIMSBYNAMEVERMETA_HH
#define OPCATALOGALLVARATTRIBUTESWITHTYPEDIMSBYNAMEVERMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta");
        inline const static std::string  op_name = "OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta(opbox::net::peer_ptr_t peer, const md_catalog_all_var_attributes_with_type_dims_by_name_ver_args &args);
  // OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta(op_create_as_target_t t);
  // ~OpCatalogAllVarAttributesWithTypeDimsByNameVerMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_TYPE_DIMS_BY_NAME_VER_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;



  //std::string serializeMsgToServer(const md_catalog_all_var_attributes_with_type_dims_by_name_ver_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_var_attributes_with_type_dims_by_name_ver_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_var_attribute_entry> &attribute_list, uint32_t count, int return_value);



 
};

#endif // OPCATALOGALLVARATTRIBUTESWITHTYPEDIMSBYNAMEVERMETA_HH
