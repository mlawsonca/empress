#ifndef OPCORECLIENT_HH
#define OPCORECLIENT_HH

#include <OpCoreCommon.hh>

extern uint32_t MAX_EAGER_MSG_SIZE;

// template <class T>
// OpCore::OpCore(opbox::net::peer_ptr_t dst , const T &args) 
//     : OpCore(true) {

//     peer = dst;
//     // 
//     // add_timing_point(getOpTimingConstant() + START);

//     //serialize the message
//     std::stringstream ss;
//     boost::archive::text_oarchive oa(ss);
//     oa << args;
//     std::string serial_str = ss.str();

//     add_timing_point(getOpTimingConstant() + SERIALIZE_MSG_FOR_SERVER);

//     createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
//                       GetAssignedMailbox(), 
//                       0, 
//                       serial_str);


//     // add_timing_point(getOpTimingConstant() + CREATE_MSG_FOR_SERVER); 
//   //Work picks up again in Server's state machine

// }

//reminder: This won't work because we are using strings (and other embedded structs) of non-predetermined size
// template <class T>
// void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const T &args,
//                                    bool vector)
// {  

//     // //std::cout << "src_mailbox: " << src_mailbox << std::endl;
//     // //std::cout << "dst_mailbox: " << dst_mailbox << std::endl;

//     std::cout << "new createOutgoingMessage: about to start" << std::endl;

//     uint32_t args_size;
//     if(vector) {
//         args_size = args.size() * sizeof(T);
//     }
//     else {
//         args_size = sizeof(T);
//     }

//     std::cout << "new createOutgoingMessage: args_size: " << args_size << std::endl;
//     //to fix - need to get a 0 or 1 as the first char to indicate whether its rdma
    
//     if(args_size > MAX_EAGER_MSG_SIZE) {
//         std::cout << "new createOutgoingMessage: about to memcpy" << std::endl;

//     // if(archived_msg.size() > 0) { //make sure we only use rdma
//     // if(archived_msg.size() > 1000000000) { //dummy to make sure we never use rdma
//         //std::cout << "creating RDMA message" << std::endl;
//         lunasa::DataObject data_obj_msg = lunasa::DataObject(0, args_size+1, lunasa::DataObject::AllocatorType::eager);
//         // copy the message into the RDMA target
//         memcpy(data_obj_msg.GetDataPtr(), &args, args_size+1);
//         ping_ldo = data_obj_msg;
//         add_timing_point(getOpTimingConstant() + SERIALIZE_MSG_FOR_SERVER);

//         std::cout << "new createOutgoingMessage: about to NewMessage" << std::endl;

//         ldo_msg = opbox::net::NewMessage(sizeof(message_t)+sizeof(struct opbox::net::NetBufferRemote)+1);
//         message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//         msg->src           = opbox::net::GetMyID();
//         msg->dst           = dst;
//         msg->src_mailbox   = src_mailbox;
//         msg->dst_mailbox   = dst_mailbox;
//         msg->op_id         = getOpID();
//         msg->body_len      = sizeof(struct opbox::net::NetBufferRemote)+1;

//         opbox::net::NetBufferLocal  *nbl = nullptr;
//         opbox::net::NetBufferRemote  nbr;
//         opbox::net::GetRdmaPtr(&ping_ldo, &nbl, &nbr);

//         //set the first char to 1 to indicate it's an rdma message
//         msg->body[0] = '1';
//         memcpy(&msg->body[1], &nbr, sizeof(opbox::net::NetBufferRemote));

//         //std::cout << "nbr: " << nbr.str() << std::endl;
//         //std::cout << "message: " << msg->body << std::endl;
//         //std::cout << "message length: " << msg->body_len << std::endl;
//     }
//     else {
//         std::cout << "new createOutgoingMessage: about to serializeMsgToServer" << std::endl;

//         std::string archived_msg = serializeMsgToServer(args);
//         add_timing_point(getOpTimingConstant() + SERIALIZE_MSG_FOR_SERVER);

//         std::cout << "new createOutgoingMessage: about to NewMessage" << std::endl;

//         //std::cout << "creating regular message" << std::endl;
//         ldo_msg = opbox::net::NewMessage( sizeof(message_t)+archived_msg.size()+2);
//         message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//         msg->src           = opbox::net::GetMyID();
//         msg->dst           = dst;
//         msg->src_mailbox   = src_mailbox;
//         msg->dst_mailbox   = dst_mailbox;
//         msg->op_id         = getOpID();
//         msg->body_len      = archived_msg.size()+2;

//         //set the first char to 0 to indicate it's an eager message (not rdma)
//         msg->body[0] = '0';
//         archived_msg.copy(&msg->body[1], archived_msg.size());
//         msg->body[archived_msg.size()+1] = '\0';
//         //std::cout << "message: " << msg->body << std::endl;
//         //std::cout << "message length: " << msg->body_len << std::endl;
//     }

//     //std::cout << "done with createOutgoingMessage" << std::endl;
// }





// template <class T>
// void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const T &args
//                                    )
// {  

//     std::cout << "am using the new createOutgoingMessage" << std::endl;
//     // if(archived_msg.size() > 0) { //make sure we only use rdma
//     // if(archived_msg.size() > 1000000000) { //dummy to make sure we never use rdma
//         //std::cout << "creating RDMA message" << std::endl;
//         lunasa::DataObject data_obj_msg = lunasa::DataObject(0, args_size+1, lunasa::DataObject::AllocatorType::eager);
//         // copy the message into the RDMA target
//         memcpy(data_obj_msg.GetDataPtr(), &args, args_size+1);
//         ping_ldo = data_obj_msg;

//         ldo_msg = opbox::net::NewMessage(sizeof(message_t)+sizeof(struct opbox::net::NetBufferRemote)+1);
//         message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//         msg->src           = opbox::net::GetMyID();
//         msg->dst           = dst;
//         msg->src_mailbox   = src_mailbox;
//         msg->dst_mailbox   = dst_mailbox;
//         msg->op_id         = getOpID();
//         msg->body_len      = sizeof(struct opbox::net::NetBufferRemote)+1;

//         opbox::net::NetBufferLocal  *nbl = nullptr;
//         opbox::net::NetBufferRemote  nbr;
//         opbox::net::GetRdmaPtr(&ping_ldo, &nbl, &nbr);

//         //set the first char to 1 to indicate it's an rdma message
//         msg->body[0] = '1';
//         memcpy(&msg->body[1], &nbr, sizeof(opbox::net::NetBufferRemote));

//         //std::cout << "nbr: " << nbr.str() << std::endl;
//         //std::cout << "message: " << msg->body << std::endl;
//         //std::cout << "message length: " << msg->body_len << std::endl;

//     //std::cout << "done with createOutgoingMessage" << std::endl;
// }


template <class T>
std::string OpCore::serializeMsgToServer( const T &args) {
    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << args;
    //log("the archived message is " + ss.str());
    //std::cout << "serialized message: " << ss.str() << std::endl;

    return ss.str();
}


#endif //OPCORECLIENT_HH


