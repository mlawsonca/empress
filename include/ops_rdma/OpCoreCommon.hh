

#ifndef OPCORECOMMON_HH
#define OPCORECOMMON_HH

#include <future>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <sstream>
#include <iostream>
#include "opbox/OpBox.hh"
#include "faodel-common/Common.hh"
#include "lunasa/DataObject.hh"
#include "opbox/ops/OpHelpers.hh" //needed for AllEventsCallback

#include <my_metadata_args.h>

#include <md_client_timing_constants.hh>


extern bool debug;
// extern std::vector<int> catg_of_time_pts;
// extern std::vector<std::chrono::high_resolution_clock::time_point> time_pts;
// extern bool output_timing;
extern void add_timing_point(int catg);


class OpCore : public opbox::Op {


    public:
        // Unique name and id for this op
        // const static unsigned int op_id;
        // const static std::string  op_name;  

        virtual unsigned int getOpID() const=0;
        virtual std::string  getOpName() const=0;

        WaitingType UpdateOrigin(OpArgs *args);
        WaitingType UpdateTarget(OpArgs *args);
        OpCore(bool t);
        OpCore(op_create_as_target_t t);

        // template <class T> 
        // OpCore(opbox::net::peer_ptr_t dst , const T &args);

        // OpCore(opbox::net::peer_ptr_t dst , const op_args &args);

        ~OpCore();

        std::string GetStateName() const;
        std::future<std::string> GetFuture();

    protected:
        // enum class State : int  {
        //     start=0,
        //     snd_wait_for_reply,
        //     done 
        // };
        enum class State : int  {
            start=0,
            snd_wait_for_reply,
            get_wait_complete,
            done 
        };

        enum timing_constants_shared : short int {
            RDMA_GET_START = 8,
            RDMA_GET_FINISHED = 9,
        };

        enum timing_constants_client : int {
            //START = 1,
            SERIALIZE_MSG_FOR_SERVER = 2,
            //CREATE_MSG = 3,
            SEND_MSG_TO_SERVER = 4,
            RETURN_MSG_RECEIVED_FROM_SERVER = 5,
            //ROMISE_VAL_SET_OP_DONE = 6,
        };

        enum timing_constants_server : int {
            START = 0,
            // DEARCHIVE_MSG = 1,
            // DB_QUERY = 2,
            // SERIALIZE_MSG = 3,
            // CREATE_MSG = 4,
            // DONE = 5,
            SERVER = 50,
        };



        State state;
        opbox::net::peer_t *peer;

        std::promise<std::string> return_msg_promise;

        virtual unsigned short getOpTimingConstant() const=0;
        //handle message on server side from client
        //problem - requires clients to define it
        virtual WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL);
        // virtual WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL);
        // WaitingType handleServerMessage(message_t *incoming_msg=NULL);
        WaitingType handleRDMA(message_t *incoming_msg);

        WaitingType handleMessageFromServer(message_t *incoming_msg);

        void createOutgoingMessage(faodel::nodeid_t dst, 
                                       const mailbox_t &src_mailbox, 
                                       const mailbox_t &dst_mailbox,
                                       const std::string &archived_msg);

        void createEmptyOutgoingMessage(faodel::nodeid_t dst, 
                                       const mailbox_t &src_mailbox, 
                                       const mailbox_t &dst_mailbox);
        
        // template <class T>
        // void OpCore::createOutgoingMessage(faodel::nodeid_t dst , 
        //                                    const mailbox_t &src_mailbox, 
        //                                    const mailbox_t &dst_mailbox,
        //                                    const T &args
        //                                    );

        // template <class T>
        // void createOutgoingMessage(faodel::nodeid_t dst , 
        //                                    const mailbox_t &src_mailbox, 
        //                                    const mailbox_t &dst_mailbox,
        //                                    const T &args,
        //                                    bool vector);
        // template <class T>
        // void deArchiveMsgFromClient(const std::string &serial_str, T &args);

        // template <class T>
        // void deArchiveMsgFromClient(lunasa::DataObject msg, T &args);

        template <class T>
        void deArchiveMsgFromClient(bool rdma, message_t *incoming_msg, T &args);

        template <class T>
        std::string serializeMsgToClient(const T &attribute_list,
                                                            uint32_t count,
                                                            int return_value);

        std::string serializeMsgToClient(uint64_t id,
                                                  int return_value);

        template <class T>
        std::string serializeMsgToServer( const T &args);

        lunasa::DataObject ldo_msg;


        // vars needed for rdma
        faodel::nodeid_t dst;
        mailbox_t dst_mailbox;

        // we need to hold a copy of the NBR between calls to Update()
        opbox::net::NetBufferRemote nbr;

        lunasa::DataObject  ping_ldo;
        lunasa::DataObject  shout_ldo;

        //void log(const std::string &s);

};

#endif // OPCORECOMMON_HH
