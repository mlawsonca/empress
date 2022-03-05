


#ifndef OPINSERTTIMESTEPATTRIBUTEBATCHMETA_HH
#define OPINSERTTIMESTEPATTRIBUTEBATCHMETA_HH

#include <OpCoreCommon.hh>
class OpInsertTimestepAttributeBatchMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpInsertTimestepAttributeBatchMeta");
        inline const static std::string  op_name = "OpInsertTimestepAttributeBatchMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpInsertTimestepAttributeBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_insert_timestep_attribute_args> &args);
  // OpInsertTimestepAttributeBatchMeta(op_create_as_target_t t);
  // ~OpInsertTimestepAttributeBatchMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_INSERT_TIMESTEP_ATTRIBUTE_BATCH_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;//std::string serializeMsgToServer(const std::vector<md_insert_timestep_attribute_args> &args);
//void deArchiveMsgFromClient(message_t *incoming_msg, std::vector<md_insert_timestep_attribute_args> &args);

// //std::string serializeMsgToClient(int return_value, uint64_t attribute_id);
////std::string serializeMsgToClient(int return_value);


};

#endif // OPINSERTTIMESTEPATTRIBUTEBATCHMETA_HH
