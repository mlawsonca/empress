


#ifndef OPCATALOGALLRUNATTRIBUTESWITHTYPEMETA_HH
#define OPCATALOGALLRUNATTRIBUTESWITHTYPEMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogAllRunAttributesWithTypeMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogAllRunAttributesWithTypeMeta");
        inline const static std::string  op_name = "OpCatalogAllRunAttributesWithTypeMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogAllRunAttributesWithTypeMeta(opbox::net::peer_ptr_t peer, const md_catalog_all_run_attributes_with_type_args &args);
  // OpCatalogAllRunAttributesWithTypeMeta(op_create_as_target_t t);
  // ~OpCatalogAllRunAttributesWithTypeMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_ALL_RUN_ATTRIBUTES_WITH_TYPE_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;



  //std::string serializeMsgToServer(const md_catalog_all_run_attributes_with_type_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_all_run_attributes_with_type_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_run_attribute_entry> &attribute_list, uint32_t count, int return_value);



 
};

#endif // OPCATALOGALLRUNATTRIBUTESWITHTYPEMETA_HH
