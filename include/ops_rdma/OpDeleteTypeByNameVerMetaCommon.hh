


#ifndef OPDELETETYPEBYNAMEVERMETA_HH
#define OPDELETETYPEBYNAMEVERMETA_HH

#include <OpCoreCommon.hh>
class OpDeleteTypeByNameVerMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpDeleteTypeByNameVerMeta");
        inline const static std::string  op_name = "OpDeleteTypeByNameVerMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpDeleteTypeByNameVerMeta(opbox::net::peer_ptr_t peer, const md_delete_type_by_name_ver_args &args);
  // OpDeleteTypeByNameVerMeta(op_create_as_target_t t);
  // ~OpDeleteTypeByNameVerMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();



private:
    unsigned short getOpTimingConstant() const { return MD_DELETE_TYPE_BY_NAME_VER_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;
  //std::string serializeMsgToServer(const md_delete_type_by_name_ver_args &args);
  
  //void deArchiveMsgFromClient(const std::string &serial_str, md_delete_type_by_name_ver_args &args); 
};

#endif // OPDELETETYPEBYNAMEVERMETA_HH
