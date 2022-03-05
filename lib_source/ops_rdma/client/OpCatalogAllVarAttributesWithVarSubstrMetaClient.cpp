

#include <OpCatalogAllVarAttributesWithVarSubstrMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;



OpCatalogAllVarAttributesWithVarSubstrMeta::OpCatalogAllVarAttributesWithVarSubstrMeta(opbox::net::peer_ptr_t dst, const md_catalog_all_var_attributes_with_var_substr_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_CATALOG_ALL_VAR_ATTRIBUTES_WITH_VAR_SUBSTR_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

}

WaitingType OpCatalogAllVarAttributesWithVarSubstrMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}