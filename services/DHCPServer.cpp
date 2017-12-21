#include "DHCPServer.h"
#include "../logger.h"
#include "../NetworkUtils.h"
#include <string.h>


DHCPServer::DHCPServer()
{
}

DHCPServer::~DHCPServer()
{
}
    
std::tuple<u16,u8*> DHCPServer::makeDHCPResponse(DHCPFullMessage* s, DhcpConfigCallback cb)
{

    u16 size = __builtin_bswap16(s->l3.length) - sizeof(IPPacket);

    MacAddress src = extractMacAddress((u8*)&s->l2.srcMac);
    if (s->l7.opcode == 1)
    {
        u32 ipaddr;
        u32 mask;
        u32 gw;
        u32 dns;
        if (!cb(src,ipaddr,mask,gw,dns))
        {
            LOG("Host " << getMacString(src) <<" not found. DHCP request ignored");
            return std::make_tuple<u16,u8*>(0,0); 
        }

        DHCPFullMessage* r = new DHCPFullMessage();
        memset(r,0,sizeof(DHCPFullMessage));
        DHCP d(&r->l7,0);
        DHCP dhcp(&s->l7,size);

        u8 msgType = dhcp.getDHCPType();
        if (msgType == 1)
        {
            LOG("DHCP DISCOVER from "<<getMacString(src) <<". Will offer " << getCIDRString(ipaddr,mask));
            d.setDHCPType(2); // OFFER
        } 
        else if (msgType == 3)
        {
            LOG("DHCP REQUEST from "<<getMacString(src) <<". Will ack " << getCIDRString(ipaddr,mask));
            //TODO: we dont even check the request and just ACK it. should verify that
            d.setDHCPType(5); // ACK
        }
        else 
        {
            delete r;
            return std::make_tuple<u16,u8*>(0,0); 
        }
    

        d.setMask(mask);
        d.setRouter(gw);
        d.setLeaseTime(__builtin_bswap32(86400));
        d.setServer(0xFFFFFFFF);
        d.setDNS(dns);
        u16 optionsSize = d.setOptions();

        r->l7.opcode = 2;
        r->l7.type = 1;
        r->l7.hwLength = 6;
        r->l7.transactionID = s->l7.transactionID;
        r->l7.flags = __builtin_bswap16(0x8000);
        r->l7.clientIP = 0; 
        r->l7.yourIP = ipaddr;
        r->l7.serverIP = 0xFFFFFFFF;
        r->l7.gwIP = 0;
        memcpy(&r->l7.clientHardwareAddress,&s->l7.clientHardwareAddress,16);
        r->l7.magic = __builtin_bswap32(0x63825363);
       
        u8 srcMac[6] = {0,0,0,0,0,0};
        Ethernet ethernet(&r->l2);
        ethernet.setEtherType(__builtin_bswap16(0x0800));
        ethernet.setSource(srcMac);
        ethernet.setDestination(s->l2.srcMac);

        IP ip(&r->l3);
        ip.setSource(0);
        ip.setDestination(0xFFFFFFFF);
        ip.setProtocol(17); //TCP
        ip.setLength(__builtin_bswap16(sizeof(IPPacket)+sizeof(UDPSegment)+sizeof(DHCPPacket)+optionsSize));
        ip.prepare();

        UDP udp(&r->l4);
        udp.setSource(__builtin_bswap16(67));
        udp.setDestination(__builtin_bswap16(68));
        udp.setLength(__builtin_bswap16(sizeof(UDPSegment)+sizeof(DHCPPacket)+optionsSize));
        return std::make_tuple<u16,u8*>((sizeof(DHCPFullMessage)+optionsSize-DHCP_BUFFER_EXTRA),(u8*)r);
    }

    return std::make_tuple<u16,u8*>(0,0); 
}

