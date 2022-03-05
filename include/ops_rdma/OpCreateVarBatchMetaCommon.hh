


#ifndef OPCREATEVARBATCHMETA_HH
#define OPCREATEVARBATCHMETA_HH

#include <OpCoreCommon.hh>
class OpCreateVarBatchMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpCreateVarBatchMeta");
        inline const static std::string  op_name = "OpCreateVarBatchMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCreateVarBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_create_var_args> &args);
  // OpCreateVarBatchMeta(op_create_as_target_t t);
  // ~OpCreateVarBatchMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_CREATE_VAR_BATCH_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;
  //std::string serializeMsgToServer(const std::vector<md_create_var_args> &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, std::vector<md_create_var_args> &args);

  // //std::string serializeMsgToClient(uint64_t var_id,
  //                                                   int return_value);


 
};

#endif // OPCREATEVARBATCHMETA_HH
