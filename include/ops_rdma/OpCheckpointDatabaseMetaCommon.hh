


#ifndef OPCHECKPOINT_DATABASEMETA_HH
#define OPCHECKPOINT_DATABASEMETA_HH

#include <OpCoreCommon.hh>

class OpCheckpointDatabaseMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;

        const static unsigned int op_id = const_hash("OpCheckpointDatabaseMeta");
        inline const static std::string  op_name = "OpCheckpointDatabaseMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpCheckpointDatabaseMeta(opbox::net::peer_ptr_t dst , const md_checkpoint_database_args &args);
  // OpCheckpointDatabaseMeta(opbox::net::peer_ptr_t dst , uint64_t job_id, int checkpt_type);
  // OpCheckpointDatabaseMeta(op_create_as_target_t t);
  // ~OpCheckpointDatabaseMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();

private:
    unsigned short getOpTimingConstant() const { return MD_CHECKPOINT_DATABASE_START; };
    // WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;
    // WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;


};

#endif // OPCHECKPOINT_DATABASEMETA_HH
