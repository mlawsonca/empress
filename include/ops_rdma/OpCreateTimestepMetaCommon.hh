


#ifndef OPCREATETIMESTEPMETA_HH
#define OPCREATETIMESTEPMETA_HH

#include <OpCoreCommon.hh>
class OpCreateTimestepMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateTimestepMeta");
        inline const static std::string  op_name = "OpCreateTimestepMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateTimestepMeta(opbox::net::peer_ptr_t peer, const md_create_timestep_args &args);
  // OpCreateTimestepMeta(op_create_as_target_t t);
  // ~OpCreateTimestepMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_TIMESTEP_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;
  //std::string serializeMsgToServer(const md_create_timestep_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_create_timestep_args &args);

  //std::string serializeMsgToClient(uint64_t timestep_id, int return_value);



 
};

#endif // OPCREATETIMESTEPMETA_HH
