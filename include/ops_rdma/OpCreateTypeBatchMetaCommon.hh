


#ifndef OPCREATETYPEBATCHMETA_HH
#define OPCREATETYPEBATCHMETA_HH

#include <OpCoreCommon.hh>


class OpCreateTypeBatchMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateTypeBatchMeta");
        inline const static std::string  op_name = "OpCreateTypeBatchMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateTypeBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_create_type_args> &args);
  // OpCreateTypeBatchMeta(op_create_as_target_t t);
  // ~OpCreateTypeBatchMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_TYPE_BATCH_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const std::vector<md_create_type_args> &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, std::vector<md_create_type_args> &args);

  //std::string serializeMsgToClient(uint64_t first_type_id, int return_value);
  // //std::string serializeMsgToClient(const std::vector<uint64_t> &type_ids, int return_value);

 
};

#endif // OPCREATETYPEBATCHMETA_HH
