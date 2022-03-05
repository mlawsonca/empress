


#ifndef OPCREATEVARMETA_HH
#define OPCREATEVARMETA_HH

#include <OpCoreCommon.hh>
class OpCreateVarMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateVarMeta");
        inline const static std::string  op_name = "OpCreateVarMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateVarMeta(opbox::net::peer_ptr_t peer, const md_create_var_args &args);
  // OpCreateVarMeta(op_create_as_target_t t);
  // ~OpCreateVarMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_VAR_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_create_var_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_create_var_args &args);

  //std::string serializeMsgToClient(uint64_t var_id, int return_value);



 
};

#endif // OPCREATEVARMETA_HH
