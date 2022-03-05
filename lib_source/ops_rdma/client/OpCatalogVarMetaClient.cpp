

#include <OpCatalogVarMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;


OpCatalogVarMeta::OpCatalogVarMeta(opbox::net::peer_ptr_t dst, const md_catalog_var_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_VAR_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_VAR_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CATALOG_VAR_CREATE_MSG_FOR_SERVER);
}

WaitingType OpCatalogVarMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
