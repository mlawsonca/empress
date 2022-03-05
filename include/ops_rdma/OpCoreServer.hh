#ifndef OPCORESERVER_HH
#define OPCORESERVER_HH

#include <OpCoreCommon.hh>


template <class T>
void OpCore::deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, T &args) {

    std::stringstream ss;
    if(rdma) {
        //std::cout << "in rdma deArchiveMsgFromClient" << std::endl; 
        //std::cout << "msg.GetDataPtr: " << shout_ldo.GetDataPtr<char *>() << std::endl;
        //std::cout << "msg.GetDataSize(): " << shout_ldo.GetDataSize() << std::endl;

        ss.write(shout_ldo.GetDataPtr<char *>(), shout_ldo.GetDataSize());

        //std::cout << "message body: " << ss.str() << std::endl;
    }
    else {
        //std::cout << "in regular deArchiveMsgFromClient" << std::endl; 
        // //std::cout << "serial_str to deArchive: " << &incoming_msg->body[1] << std::endl;
        // ss << incoming_msg->body[1];
        ss.write(&incoming_msg->body[1], incoming_msg->body_len-1);
    }

    // cout << "message size: " << msg.GetDataSize() << endl;

    boost::archive::text_iarchive ia(ss);
    ia >> args;

    // cout << "args.size(): " << args.size() << endl;
    // //cout << "deserialized text string: " << ss.str() << endl;
    // //cout << "deserialized text string length: " << ss.str().size() << endl;

}


// template <class T>
// void OpCore::deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, T &args) {

//     std::stringstream ss;
//     if(rdma) {
//         //std::cout << "in rdma deArchiveMsgFromClient" << std::endl; 
//         //std::cout << "msg.GetDataPtr: " << shout_ldo.GetDataPtr<char *>() << std::endl;
//         //std::cout << "msg.GetDataSize(): " << shout_ldo.GetDataSize() << std::endl;

//         ss.write(shout_ldo.GetDataPtr<char *>(), shout_ldo.GetDataSize());

//         //std::cout << "message body: " << ss.str() << std::endl;
//     }
//     else {
//         //std::cout << "in regular deArchiveMsgFromClient" << std::endl; 
//         // //std::cout << "serial_str to deArchive: " << &incoming_msg->body[1] << std::endl;
//         // ss << incoming_msg->body[1];
//         ss.write(&incoming_msg->body[1], incoming_msg->body_len-1);
//     }

//     // cout << "message size: " << msg.GetDataSize() << endl;

//     boost::archive::text_iarchive ia(ss);
//     ia >> args;

//     // cout << "args.size(): " << args.size() << endl;
//     // //cout << "deserialized text string: " << ss.str() << endl;
//     // //cout << "deserialized text string length: " << ss.str().size() << endl;

// }

template <class T>
std::string OpCore::serializeMsgToClient(const T &attribute_list,
                                                    uint32_t count, int return_value) {

    std::stringstream ss;
    boost::archive::text_oarchive oa(ss);
    oa << count;
    if (count > 0) {
        oa << attribute_list;
    }
    oa << return_value;
    //log("the archived message is " + ss.str());


    return ss.str();
}



#endif //OPCORESERVER_HH
