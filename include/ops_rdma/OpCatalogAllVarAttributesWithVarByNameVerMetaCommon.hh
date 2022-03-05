


#ifndef OPCATALOGALLVARATTRIBUTESWITHVARBYNAMEVERMETA_HH
#define OPCATALOGALLVARATTRIBUTESWITHVARBYNAMEVERMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogAllVarAttributesWithVarByNameVerMeta : public OpCore {



    public:
        using OpCore::OpCore;
        //Unique name and id for this op
        const static unsigned int op_id = const_hash("OpCatalogAllVarAttributesWithVarByNameVerMeta");
        inline const static std::string  op_name = "OpCatalogAllVarAttributesWithVarByNameVerMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
        //Unique name and id for this op

OpCatalogAllVarAttributesWithVarByNameVerMeta(opbox::net::peer_ptr_t peer, const md_catalog_all_var_attributes_with_var_by_name_ver_args &args);
// OpCatalogAllVarAttributesWithVarByNameVerMeta(op_create_as_target_t t);
// ~OpCatalogAllVarAttributesWithVarByNameVerMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_BY_NAME_VER_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;



  //std::string serializeMsgToServer(const md_catalog_all_var_attributes_with_var_by_name_ver_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_var_attributes_with_var_by_name_ver_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_var_attribute_entry> &attribute_list, uint32_t count, int return_value);



 
};

#endif // OPCATALOGALLVARATTRIBUTESWITHVARBYNAMEVERMETA_HH
