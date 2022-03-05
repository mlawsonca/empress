          

#include <OpCoreCommon.hh>
// #include "faodel-common/Debug.hh"

using namespace std;

extern uint32_t MAX_EAGER_MSG_SIZE;

OpCore::OpCore(bool t) 
  : state(State::start), ldo_msg(), Op(t) {
  //No work to do - done in Client's state machine
}

OpCore::OpCore(op_create_as_target_t t) 
  : state(State::start), ldo_msg(), Op(t) {
  //No work to do - done in Client's state machine
}

OpCore::~OpCore() {
  //   if((state == State::start) && (ldo_msg != nullptr)) {
  //   net::ReleaseMessage(ldo_msg);
  // }
}

void OpCore::createEmptyOutgoingMessage(faodel::nodeid_t dst , 
                                   const mailbox_t &src_mailbox, 
                                   const mailbox_t &dst_mailbox){  

  ldo_msg = opbox::net::NewMessage( sizeof(message_t) );
  message_t *msg = ldo_msg.GetDataPtr<message_t *>();
  msg->src           = opbox::net::GetMyID();
  msg->dst           = dst;
  msg->src_mailbox   = src_mailbox;
  msg->dst_mailbox   = dst_mailbox;
  msg->op_id         = getOpID();
  msg->body_len      = 0;
  }

void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
                                   const mailbox_t &src_mailbox, 
                                   const mailbox_t &dst_mailbox,
                                   const std::string &archived_msg){  

    //cout << "message size: " << archived_msg.size() << endl;    
    //cout << "message: " << archived_msg << endl;
    // //cout << "src_mailbox: " << src_mailbox << endl;
    // //cout << "dst_mailbox: " << dst_mailbox << endl;


    //to fix - need to get a 0 or 1 as the first char to indicate whether its rdma
    
    if(archived_msg.size() > MAX_EAGER_MSG_SIZE) {
    // if(archived_msg.size() > 0) { //make sure we only use rdma
    // if(archived_msg.size() > 1000000000) { //dummy to make sure we never use rdma
        //cout << "creating RDMA message" << endl;
        lunasa::DataObject data_obj_msg = lunasa::DataObject(0, archived_msg.size()+1, lunasa::DataObject::AllocatorType::eager);
        // copy the message into the RDMA target
        memcpy(data_obj_msg.GetDataPtr(), archived_msg.c_str(), archived_msg.size()+1);
        ping_ldo = data_obj_msg;

        ldo_msg = opbox::net::NewMessage(sizeof(message_t)+sizeof(struct opbox::net::NetBufferRemote)+1);
        message_t *msg = ldo_msg.GetDataPtr<message_t *>();
        msg->src           = opbox::net::GetMyID();
        msg->dst           = dst;
        msg->src_mailbox   = src_mailbox;
        msg->dst_mailbox   = dst_mailbox;
        msg->op_id         = getOpID();
        msg->body_len      = sizeof(struct opbox::net::NetBufferRemote)+1;

        opbox::net::NetBufferLocal  *nbl = nullptr;
        opbox::net::NetBufferRemote  nbr;
        opbox::net::GetRdmaPtr(&ping_ldo, &nbl, &nbr);

        //set the first char to 1 to indicate it's an rdma message
        msg->body[0] = '1';
        memcpy(&msg->body[1], &nbr, sizeof(opbox::net::NetBufferRemote));

        //cout << "nbr: " << nbr.str() << endl;
        //cout << "message: " << msg->body << endl;
        //cout << "message length: " << msg->body_len << endl;
    }
    else {
        //cout << "creating regular message" << endl;
        ldo_msg = opbox::net::NewMessage( sizeof(message_t)+archived_msg.size()+2);
        message_t *msg = ldo_msg.GetDataPtr<message_t *>();
        msg->src           = opbox::net::GetMyID();
        msg->dst           = dst;
        msg->src_mailbox   = src_mailbox;
        msg->dst_mailbox   = dst_mailbox;
        msg->op_id         = getOpID();
        msg->body_len      = archived_msg.size()+2;

        //set the first char to 0 to indicate it's an eager message (not rdma)
        msg->body[0] = '0';
        archived_msg.copy(&msg->body[1], archived_msg.size());
        msg->body[archived_msg.size()+1] = '\0';
        //cout << "message: " << msg->body << endl;
        //cout << "message length: " << msg->body_len << endl;
    }

    //cout << "done with createOutgoingMessage" << endl;
}


// void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const std::string &archived_msg){  

//         //cout << "creating regular message" << endl;
//         ldo_msg = opbox::net::NewMessage( sizeof(message_t)+archived_msg.size()+1);
//         message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//         msg->src           = opbox::net::GetMyID();
//         msg->dst           = dst;
//         msg->src_mailbox   = src_mailbox;
//         msg->dst_mailbox   = dst_mailbox;
//         msg->op_id         = getOpID();
//         msg->body_len      = archived_msg.size()+1;

//         //set the first char to 0 to indicate it's an eager message (not rdma)
//         archived_msg.copy(&msg->body[0], archived_msg.size());
//         msg->body[archived_msg.size()] = '\0';
//         //cout << "message: " << msg->body << endl;
//         //cout << "message length: " << msg->body_len << endl;

//     //cout << "done with createOutgoingMessage" << endl;
// }



std::string OpCore::GetStateName() const {
  switch(state){
  case State::start:              return "Start";
  case State::snd_wait_for_reply: return "Sender-WaitForReply";
  case State::get_wait_complete: return "GetWaitComplete";  
  case State::done:               return "Done";
  }
  KFAIL();
}

// void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
//                                    const mailbox_t &src_mailbox, 
//                                    const mailbox_t &dst_mailbox,
//                                    const std::string &archived_msg){  

//     ldo_msg = opbox::net::NewMessage( sizeof(message_t)+archived_msg.size()+1);
//     message_t *msg = ldo_msg.GetDataPtr<message_t *>();
//     msg->src           = opbox::net::GetMyID();
//     msg->dst           = dst;
//     msg->src_mailbox   = src_mailbox;
//     msg->dst_mailbox   = dst_mailbox;
//     msg->op_id         = getOpID();
//     msg->body_len      = archived_msg.size()+1;
//     archived_msg.copy(&msg->body[0], archived_msg.size());
//     msg->body[archived_msg.size()] = '\0';
// }
 
// std::string OpCore::GetStateName() const {
//     switch(state){
//     case State::start:              return "Start";
//     case State::snd_wait_for_reply: return "Sender-WaitForReply";
//     case State::done:               return "Done";
//     }
//     KFAIL();
// }

// void OpCoreMeta::log(const std::string &s)  {
//   if(debug) //cout << "("<< op_name << "): "<< s<<endl;
// }



