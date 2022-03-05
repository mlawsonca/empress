


#ifndef OPCREATETYPEMETA_HH
#define OPCREATETYPEMETA_HH

#include <OpCoreCommon.hh>


class OpCreateTypeMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateTypeMeta");
        inline const static std::string  op_name = "OpCreateTypeMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateTypeMeta(opbox::net::peer_ptr_t peer, const md_create_type_args &args);
  // OpCreateTypeMeta(op_create_as_target_t t);
  // ~OpCreateTypeMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_TYPE_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_create_type_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_create_type_args &args);

  //std::string serializeMsgToClient(uint64_t row_id, int return_value);

 
};

#endif // OPCREATETYPEMETA_HH
