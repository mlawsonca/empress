

#include <OpCatalogAllTimestepsWithVarSubstrMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;

OpCatalogAllTimestepsWithVarSubstrMeta::OpCatalogAllTimestepsWithVarSubstrMeta(opbox::net::peer_ptr_t dst, const md_catalog_all_timesteps_with_var_substr_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_SUBSTR_CREATE_MSG_FOR_SERVER);
}

WaitingType OpCatalogAllTimestepsWithVarSubstrMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}