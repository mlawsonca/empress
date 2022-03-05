


#ifndef OPINSERTVARATTRIBUTEBYDIMSBATCHMETA_HH
#define OPINSERTVARATTRIBUTEBYDIMSBATCHMETA_HH

#include <future>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
//#include <boost/archive/binary_iarchive.hpp>
//#include <boost/archive/binary_oarchive.hpp>
#include <sstream>
#include <iostream>
#include "opbox/OpBox.hh"
#include "faodel-common/Common.hh"
#include "lunasa/DataObject.hh"
// #include <chrono>
#include <my_metadata_args.h>


extern bool debug;
// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
// extern bool output_timing;
extern void add_timing_point(int catg);



// class OpInsertVarAttributeByDimsBatchMeta : public opbox::Op {

//  enum class State : int  {
//       start=0,
//       snd_wait_for_reply,
//       done };

// public:
//   OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_insert_var_attribute_by_dims_args> &args);
//   OpInsertVarAttributeByDimsBatchMeta(op_create_as_target_t t);
//   ~OpInsertVarAttributeByDimsBatchMeta();

//   //Means for passing back the result
//   std::future<std::string> GetFuture();

//   //Unique name and id for this op
//   const static unsigned int op_id;
//   const static std::string  op_name;  

//   unsigned int getOpID() const { return op_id; }
//   std::string  getOpName() const { return op_name; }



//   WaitingType UpdateOrigin(OpArgs *args);
//   WaitingType UpdateTarget(OpArgs *args);
 
//   std::string GetStateName() const;


// // void deArchiveMsgFromServer(const std::string &serial_str, int &return_value, uint64_t &attribute_id);

// private:
//   State state;
//   opbox::net::peer_t *peer;

//   std::promise<std::string> return_msg_promise;

//   //void log(const std::string &s);

//   void createOutgoingMessage(faodel::nodeid_t dst, 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const std::string &archived_msg);

//   lunasa::DataObject ldo_msg;

// std::string serializeMsgToServer(const std::vector<md_insert_var_attribute_by_dims_args> &args);
// void deArchiveMsgFromClient(message_t *incoming_msg, std::vector<md_insert_var_attribute_by_dims_args> &args);

// std::string serializeMsgToClient(int return_value, uint64_t attribute_id);

// };


// #include <OpCoreCommon.hh>

// class OpInsertVarAttributeByDimsBatchMeta : public OpCore {

//     public:
//         // Unique name and id for this op
//         // using OpCore::OpCore;
//         const static unsigned int op_id = const_hash("OpInsertVarAttributeByDimsBatchMeta");
//         inline const static std::string  op_name = "OpInsertVarAttributeByDimsBatchMeta"; 

//         unsigned int getOpID() const { return op_id; }
//         std::string  getOpName() const { return op_name; }

//         WaitingType UpdateOrigin(OpArgs *args);
//         WaitingType UpdateTarget(OpArgs *args); 
//   OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_insert_var_attribute_by_dims_args> &args);
//   OpInsertVarAttributeByDimsBatchMeta(op_create_as_target_t t);
//   ~OpInsertVarAttributeByDimsBatchMeta();

//     std::string GetStateName() const;
//   //Means for passing back the result
//   std::future<std::string> GetFuture();



// private:
//         State state;
//         opbox::net::peer_t *peer;
//         lunasa::DataObject ldo_msg;

//         std::promise<std::string> return_msg_promise;

//     unsigned short getOpTimingConstant() const { return MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START; };
//     WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;
//   //std::promise<int> return_msg_promise;


// std::string serializeMsgToServer(const std::vector<md_insert_var_attribute_by_dims_args> &args);
// //void deArchiveMsgFromClient(message_t *incoming_msg, std::vector<md_insert_var_attribute_by_dims_args> &args);
// void deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, std::vector<md_insert_var_attribute_by_dims_args> &args);

// //std::string serializeMsgToClient(int return_value, uint64_t attribute_id);

//         void createOutgoingMessage(faodel::nodeid_t dst, 
//                                        const mailbox_t &src_mailbox, 
//                                        const mailbox_t &dst_mailbox,
//                                        const std::string &archived_msg);

// };

#include <OpCoreCommon.hh>

class OpInsertVarAttributeByDimsBatchMeta : public OpCore {

    public:
        // Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpInsertVarAttributeByDimsBatchMeta");
        inline const static std::string  op_name = "OpInsertVarAttributeByDimsBatchMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        //WaitingType UpdateTarget(OpArgs *args); 
  OpInsertVarAttributeByDimsBatchMeta(opbox::net::peer_ptr_t peer, const std::vector<md_insert_var_attribute_by_dims_args> &args);
  // OpInsertVarAttributeByDimsBatchMeta(op_create_as_target_t t);
  // ~OpInsertVarAttributeByDimsBatchMeta();

  //Means for passing back the result
  //std::future<int> GetFuture();


private:
    unsigned short getOpTimingConstant() const { return MD_INSERT_VAR_ATTRIBUTE_BY_DIMS_BATCH_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;
  //std::promise<int> return_msg_promise;


// std::string serializeMsgToServer(const std::vector<md_insert_var_attribute_by_dims_args> &args);
//void deArchiveMsgFromClient(message_t *incoming_msg, std::vector<md_insert_var_attribute_by_dims_args> &args);
// void deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, std::vector<md_insert_var_attribute_by_dims_args> &args);

//std::string serializeMsgToClient(int return_value, uint64_t attribute_id);


};

#endif // OPINSERTVARATTRIBUTEBYDIMSBATCHMETA_HH
