


#ifndef OPCATALOGTYPEMETA_HH
#define OPCATALOGTYPEMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogTypeMeta : public OpCore {

 

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogTypeMeta");
        inline const static std::string  op_name = "OpCatalogTypeMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogTypeMeta(opbox::net::peer_ptr_t peer, const md_catalog_type_args &args);
  // OpCatalogTypeMeta(op_create_as_target_t t);
  // ~OpCatalogTypeMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_TYPE_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_catalog_type_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_type_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_type_entry> &entries, uint32_t count, int return_value);

  
};

#endif // OPCATALOGTYPEMETA_HH
