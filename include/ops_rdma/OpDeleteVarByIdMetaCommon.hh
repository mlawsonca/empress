


#ifndef OPDELETE_VAR_BY_IDMETA_HH
#define OPDELETE_VAR_BY_IDMETA_HH

#include <OpCoreCommon.hh>
class OpDeleteVarByIdMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpDeleteVarByIdMeta");
        inline const static std::string  op_name = "OpDeleteVarByIdMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpDeleteVarByIdMeta(opbox::net::peer_ptr_t peer, const md_delete_var_by_id_args &args);
  // OpDeleteVarByIdMeta(op_create_as_target_t t);
  // ~OpDeleteVarByIdMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_DELETE_VAR_BY_ID_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;
  //std::string serializeMsgToServer(const md_delete_var_by_id_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_delete_var_by_id_args &args); 
};

#endif // OPDELETE_VAR_BY_IDMETA_HH
