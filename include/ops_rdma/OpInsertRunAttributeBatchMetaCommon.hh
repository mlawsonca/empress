


#ifndef OPINSERTRUNATTRIBUTEBATCHMETA_HH
#define OPINSERTRUNATTRIBUTEBATCHMETA_HH

#include <OpCoreCommon.hh>
class OpInsertRunAttributeBatchMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpInsertRunAttributeBatchMeta");
        inline const static std::string  op_name = "OpInsertRunAttributeBatchMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpInsertRunAttributeBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_insert_run_attribute_args> &args);
  // OpInsertRunAttributeBatchMeta(op_create_as_target_t t);
  // ~OpInsertRunAttributeBatchMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_INSERT_RUN_ATTRIBUTE_BATCH_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<int> return_msg_promise;

//std::string serializeMsgToServer(const std::vector<md_insert_run_attribute_args> &args);
//void deArchiveMsgFromClient(message_t *incoming_msg, std::vector<md_insert_run_attribute_args> &args);



};

#endif // OPINSERTRUNATTRIBUTEBATCHMETA_HH
