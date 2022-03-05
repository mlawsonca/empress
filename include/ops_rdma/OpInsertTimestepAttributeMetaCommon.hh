


#ifndef OPINSERTTIMESTEPATTRIBUTEMETA_HH
#define OPINSERTTIMESTEPATTRIBUTEMETA_HH

#include <OpCoreCommon.hh>
class OpInsertTimestepAttributeMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpInsertTimestepAttributeMeta");
        inline const static std::string  op_name = "OpInsertTimestepAttributeMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpInsertTimestepAttributeMeta(opbox::net::peer_ptr_t peer, const md_insert_timestep_attribute_args &args);
  // OpInsertTimestepAttributeMeta(op_create_as_target_t t);
  // ~OpInsertTimestepAttributeMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_INSERT_TIMESTEP_ATTRIBUTE_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;//std::string serializeMsgToServer(const md_insert_timestep_attribute_args &args);
//void deArchiveMsgFromClient(message_t *incoming_msg, md_insert_timestep_attribute_args &args);

//std::string serializeMsgToClient(int return_value, uint64_t attribute_id);


};

#endif // OPINSERTTIMESTEPATTRIBUTEMETA_HH
