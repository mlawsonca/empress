

#ifndef OPACTIVATERUNMETA_HH
#define OPACTIVATERUNMETA_HH

#include <OpCoreCommon.hh>


class OpActivateRunMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpActivateRunMeta");
        inline const static std::string  op_name = "OpActivateRunMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpActivateRunMeta(opbox::net::peer_ptr_t peer, const md_activate_args &args);
  // OpActivateRunMeta(op_create_as_target_t t);
  // ~OpActivateRunMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_ACTIVATE_RUN_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;
    



  //std::promise<int> return_msg_promise;



  //std::string serializeMsgToServer(const md_activate_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_activate_args &args);



};

#endif // OPACTIVATERUNMETA_HH
