


#ifndef OPFULLSHUTDOWNMETACOMMON_HH
#define OPFULLSHUTDOWNMETACOMMON_HH

#include "OpCoreCommon.hh"

extern bool debug;

class OpFullShutdownMeta : public OpCore {

    public:
        //Unique name and id for this op
        using OpCore::OpCore;
        const static unsigned int op_id = const_hash("OpFullShutdownMeta");
        inline const static std::string  op_name = "OpFullShutdownMeta"; 

        unsigned int getOpID() const { return op_id; }
        std::string  getOpName() const { return op_name; }

        //WaitingType UpdateOrigin(OpArgs *args);
        WaitingType UpdateTarget(OpArgs *args); 
  OpFullShutdownMeta(opbox::net::peer_ptr_t peer);
  // OpFullShutdownMeta(op_create_as_target_t t);

    WaitingType UpdateOrigin(OpArgs *args);


private:
    unsigned short getOpTimingConstant() const { return MD_FULL_SHUTDOWN_START; };
    WaitingType handleMessage(bool rdma, message_t *incoming_msg=NULL) override;


};

#endif // OPFULLSHUTDOWNMETACOMMON_HH
