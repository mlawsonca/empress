


#ifndef OPCATALOGRUNMETA_HH
#define OPCATALOGRUNMETA_HH

#include <OpCoreCommon.hh>
class OpCatalogRunMeta : public OpCore {

 

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCatalogRunMeta");
        inline const static std::string  op_name = "OpCatalogRunMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCatalogRunMeta(opbox::net::peer_ptr_t peer, const md_catalog_run_args &args);
  // OpCatalogRunMeta(op_create_as_target_t t);
  // ~OpCatalogRunMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CATALOG_RUN_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_catalog_run_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_catalog_run_args &args);

  //std::string serializeMsgToClient(const std::vector<md_catalog_run_entry> &entries, uint32_t count, int return_value);

  
};

#endif // OPCATALOGRUNMETA_HH
