


#ifndef OPDELETEALLVARSWITHSUBSTRMETA_HH
#define OPDELETEALLVARSWITHSUBSTRMETA_HH

#include <OpCoreCommon.hh>
class OpDeleteAllVarsWithSubstrMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpDeleteAllVarsWithSubstrMeta");
        inline const static std::string  op_name = "OpDeleteAllVarsWithSubstrMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpDeleteAllVarsWithSubstrMeta(opbox::net::peer_ptr_t peer, const md_delete_all_vars_with_substr_args &args);
  // OpDeleteAllVarsWithSubstrMeta(op_create_as_target_t t);
  // ~OpDeleteAllVarsWithSubstrMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_DELETE_ALL_VARS_WITH_SUBSTR_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;
  //std::string serializeMsgToServer(const md_delete_all_vars_with_substr_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_delete_all_vars_with_substr_args &args); 
};

#endif // OPDELETEALLVARSWITHSUBSTRMETA_HH
