


#ifndef OPBEGINTRANSACTIONMETA_HH
#define OPBEGINTRANSACTIONMETA_HH

#include <OpCoreCommon.hh>

class OpBeginTransactionMeta : public OpCore {

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpBeginTransactionMeta");
        inline const static std::string  op_name = "OpBeginTransactionMeta";  

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpBeginTransactionMeta(opbox::net::peer_ptr_t peer, uint64_t database_to_query);
  // OpBeginTransactionMeta(op_create_as_target_t t);
  // ~OpBeginTransactionMeta();

  //Means for passing back the result
  // //std::future<int> GetFuture();
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_BEGIN_TRANSACTION_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;

  //std::promise<int> return_msg_promise;


  //std::string serializeMsgToServer(const md_activate_args &args);
  //void deArchiveMsgFromClient(const std::string &serial_str, md_activate_args &args);



};


#endif // OPBEGINTRANSACTIONMETA_HH
