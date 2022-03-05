


#ifndef OPCREATERUNMETA_HH
#define OPCREATERUNMETA_HH

#include <OpCoreCommon.hh>
class OpCreateRunMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateRunMeta");
        inline const static std::string  op_name = "OpCreateRunMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateRunMeta(opbox::net::peer_ptr_t peer, const md_create_run_args &args);
  // OpCreateRunMeta(op_create_as_target_t t);
  // ~OpCreateRunMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();

    virtual WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_RUN_START; };


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_create_run_args &args);
  

  //void deArchiveMsgFromClient(message_t *incoming_msg, md_create_run_args &args);

  //std::string serializeMsgToClient(uint64_t run_id, int return_value);



 
};

#endif // OPCREATERUNMETA_HH
