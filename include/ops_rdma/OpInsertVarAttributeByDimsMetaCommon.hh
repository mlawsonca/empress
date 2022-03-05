


#ifndef OPINSERTVARATTRIBUTEBYDIMSMETA_HH
#define OPINSERTVARATTRIBUTEBYDIMSMETA_HH

#include <OpCoreCommon.hh>
class OpInsertVarAttributeByDimsMeta : public OpCore {



    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpInsertVarAttributeByDimsMeta");
        inline const static std::string  op_name = "OpInsertVarAttributeByDimsMeta"; 
        
        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpInsertVarAttributeByDimsMeta(opbox::net::peer_ptr_t peer, const md_insert_var_attribute_by_dims_args &args);
  // OpInsertVarAttributeByDimsMeta(op_create_as_target_t t);
  // ~OpInsertVarAttributeByDimsMeta();

  //Means for passing back the result
  //std::future<std::string> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


  //std::promise<std::string> return_msg_promise;//std::string serializeMsgToServer(const md_insert_var_attribute_by_dims_args &args);
//void deArchiveMsgFromClient(message_t *incoming_msg, md_insert_var_attribute_by_dims_args &args);

//std::string serializeMsgToClient(int return_value, uint64_t attribute_id);


};

#endif // OPINSERTVARATTRIBUTEBYDIMSMETA_HH
