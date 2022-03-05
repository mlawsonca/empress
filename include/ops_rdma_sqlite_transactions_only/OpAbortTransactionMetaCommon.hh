


#ifndef OPABORTTRANSACTIONMETA_HH
#define OPABORTTRANSACTIONMETA_HH

#include <OpCoreCommon.hh>

class OpAbortTransactionMeta : public OpCore {

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpAbortTransactionMeta");
        inline const static std::string  op_name = "OpAbortTransactionMeta";  

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpAbortTransactionMeta(opbox::net::peer_ptr_t peer, uint64_t database_to_query);
  // OpAbortTransactionMeta(op_create_as_target_t t);
  // ~OpAbortTransactionMeta();

  //Means for passing back the result
  // //std::future<int> GetFuture();
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_ABORT_TRANSACTION_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;

  //std::promise<int> return_msg_promise;


  //std::string serializeMsgToServer(const md_activate_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_activate_args &args);



};

#endif // OPABORTTRANSACTIONMETA_HH
