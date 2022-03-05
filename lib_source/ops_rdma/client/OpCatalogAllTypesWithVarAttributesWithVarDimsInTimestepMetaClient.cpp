

#include <OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMetaCommon.hh>

#include <OpCoreClient.hh>
using namespace std;

OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta::OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta(opbox::net::peer_ptr_t dst, const md_catalog_all_types_with_var_attributes_with_var_dims_in_timestep_args &args) 
    : OpCore(true) {
    peer = dst;
    // add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_START);

    std::string serial_str = serializeMsgToServer(args);    
    add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_SERIALIZE_MSG_FOR_SERVER);

    createOutgoingMessage(opbox::net::ConvertPeerToNodeID(dst),
                      GetAssignedMailbox(), 
                      0, 
                      serial_str);
    // add_timing_point(OP_CATALOG_ALL_TYPES_WITH_VAR_ATTRIBUTES_WITH_VAR_DIMS_IN_TIMESTEP_CREATE_MSG_FOR_SERVER);
}

WaitingType OpCatalogAllTypesWithVarAttributesWithVarDimsInTimestepMeta::handleMessage(bool placeholder, message_t *incoming_msg) {
    OpCore::handleMessage(placeholder, incoming_msg);
}

