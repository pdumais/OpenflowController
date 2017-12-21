#pragma once
#include <string>
#include "types.h"

#define OF_VERSION 4 // 1.3

#define OF_PORT_CONFIG_PORT_DOWN (1 << 0) /* Port is administratively down. */
#define OF_PORT_CONFIG_NO_STP (1 << 1) /* Disable 802.1D spanning tree on port. */
#define OF_PORT_CONFIG_NO_RECV (1 << 2) /* Drop all packets except 802.1D spanning tree packets. */
#define OF_PORT_CONFIG_NO_RECV_STP (1 << 3) /* Drop received 802.1D STP packets. */
#define OF_PORT_CONFIG_NO_FLOOD (1 << 4) /* Do not include this port when flooding. */
#define OF_PORT_CONFIG_NO_FWD (1 << 5) /* Drop packets forwarded to port. */
#define OF_PORT_CONFIG_NO_PACKET_IN (1 << 6) /* Do not send packet-in msgs for port. */


#define OF_PORT_STATE_STP_LISTEN 0 /* Not learning or relaying frames. */
#define OF_PORT_STATE_LINK_DOWN 1 /* No physical link present. */
#define OF_PORT_STATE_STP_LEARN 2/* Learning but not relaying frames. */
#define OF_PORT_STATE_STP_FORWARD 4 /* Learning and relaying frames. */
#define OF_PORT_STATE_STP_BLOCK 8 /* Not part of spanning tree. */
#define OF_PORT_STATE_STP_MASK 16 /* Bit mask for OFPPS_STP_* values. */



enum class OpenFlowMessageType
{
    Hello = 0,
    Error = 1,
    EchoReq = 2,
    EchoRes = 3,
    FeatureReq = 5,
    FeatureRes = 6,
    SetConfig = 9,
    PacketIn = 10,
    PortStatus = 12,
    PacketOut = 13,
    FlowMod = 14,
    TableMod = 17,
    MultipartReq = 18,
    MultipartRes = 19
};

// Openflow spec 1.0.0 page 18
enum class OpenFlowPort: uint32_t
{
    Max = 0xffffff00,
    Ingress = 0xfffffff8,
    Table = 0xfffffff9,
    Normal = 0xfffffffa,
    Flood = 0xfffffffb, // All except input and disabled by STP
    All = 0xfffffffc,   // Same as flood but including STP disabled port
    Controller = 0xfffffffd,
    Local = 0xfffffffe,
    Any = 0xffffffff
};

enum class OpenFlowOXMField
{
    InPort = 0,
    InPhyPort = 1,
    MetaData = 2,
    EthDst = 3,
    EthSrc = 4,
    EthType = 5,
    VlanId = 6,
    IpProto = 10,
    IpSrc = 11,
    IpDst = 12,
    TcpSrc = 13,
    TcpDst = 14,
    UdpSrc = 15,
    UdpDst = 16,
    ArpOp = 21,    
};

enum class OpenFlowMultiPartTypes
{
    Desc = 0,
    Flow = 1,
    Table = 3,
    PortDesc = 13,
    Experimenter = 0xFFFF
};

struct OFMessage
{
    u8 version;
    u8 type;
    u16 length;
    u32 xid;
    u8 payload[];
} __attribute__((__packed__));

struct OFPort
{
    uint32_t id;
    uint32_t pad;
    u8  addr[6];
    u16 pad2;
    char name[16];
    uint32_t config;
    uint32_t state;
    uint32_t curr;
    uint32_t advertised;
    uint32_t supported;
    uint32_t peer;
    uint32_t currentSpeed;
    uint32_t maxSpeed;
} __attribute__((__packed__));

struct OFAction
{
    u16 type;
    u16 length;
} __attribute__((__packed__));

struct OFInstruction
{
    u16 type;
    u16 length;
} __attribute__((__packed__));

struct OFOXM
{
    u16 oclass;
    u8 hashmask:1;
    u8 field:7;
    u8 length;
    u8 data[];
} __attribute__((__packed__));


struct OFPacketOutMessage
{
    OFMessage header;
    uint32_t bufferId;
    uint32_t inPort;
    u16 actionsLength;
    u8 pad[6];
} __attribute__((__packed__));

struct OFMatch
{
    u16 type;
    u16 length;
} __attribute__((__packed__));

struct OFFlowModMessage
{
    OFMessage header;
    uint64_t cookie;
    uint64_t cookieMask;
    u8  tableId;
    u8  command;
    u16 idleTimeout;
    u16 hardTimeout;
    u16 priority;
    uint32_t bufferId;
    uint32_t outPort;
    uint32_t outGroup;
    u16 flags;
    u16 pad;
    OFMatch match;
} __attribute__((__packed__));



struct OFSetConfigMessage
{
    OFMessage header;
    u16 flags;
    u16 missSendLen;
} __attribute__((__packed__));

struct OFHelloMessage
{
    OFMessage header;
} __attribute__((__packed__));

struct OFFeatureReqMessage
{
    OFMessage header;
} __attribute__((__packed__));

struct OFTableModMessage
{
    OFMessage header;
    u8 tableId;
    u8 pad[3];
    uint32_t config;
} __attribute__((__packed__));

struct OFFeatureResMessage
{
    OFMessage header;
    uint64_t datapathId;
    uint32_t nBuffers;
    u8 nTables;
    u8 auxId;
    u16 pad;
    uint32_t capabilities;
    uint32_t reserved;
} __attribute__((__packed__));

struct OFPortStatusMessage
{
    OFMessage header;
    u8 reason;
    u8 pad[7];
    OFPort  port;
} __attribute__((__packed__));

struct OFMultipartReqMessage
{
    OFMessage header;
    u16 type;
    u16 flags;
    u8 pad[4];
} __attribute__((__packed__));

struct OFMultipartResMessage
{
    OFMessage header;
    u16 type;
    u16 flags;
    u8 pad[4];
} __attribute__((__packed__));

struct OFPacketInMessage
{                          
    OFMessage header;   
    uint32_t bufferId;  
    u16 totalSize;
    u8 reason;
    u8 table;
    uint64_t cookie;
    OFMatch match[];
} __attribute__((__packed__));










void buildMessageHeader(OFMessage*m, OpenFlowMessageType type, u32 xid);
