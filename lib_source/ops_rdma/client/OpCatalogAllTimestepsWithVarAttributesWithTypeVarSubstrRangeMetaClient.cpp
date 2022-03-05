

#include <OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;

OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta::OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta(opbox::net::peer_ptr_t dst, const md_catalog_all_timesteps_with_var_attributes_with_type_var_substr_range_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_START);

    // cout << "Client about to serialize a message to server \n";

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_SERIALIZE_MSG_FOR_SERVER);
    // cout << "Client about to create outgoing message to server \n";

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);

    // add_timing_point(OP_CATALOG_ALL_TIMESTEPS_WITH_VAR_ATTRIBUTES_WITH_TYPE_VAR_SUBSTR_RANGE_CREATE_MSG_FOR_SERVER);
  //Work picks up again in Server's state machine
  // cout << "Client just dispatched message to server \n";

}

WaitingType OpCatalogAllTimestepsWithVarAttributesWithTypeVarSubstrRangeMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}
