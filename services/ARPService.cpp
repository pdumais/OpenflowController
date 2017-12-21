#include "ARPService.h"
#include "../logger.h"
#include "../NetworkUtils.h"
#include <string.h>

ARPService::ARPService()
{
}

std::tuple<u16,u8*> ARPService::makeARPResponse(ARPFullMessage* s, ArpConfigCallback cb)
{

    MacAddress src = extractMacAddress((u8*)&s->l2.srcMac);
    u32 target = s->l3.protoDestination;
    if (s->l3.opcode == 0x0100)
    {
        LOG("ARP query from " << getMacString(src) << " For " << getIPString(target));
        MacAddress targetHw;
        if (!cb(src,target,targetHw))
        {
            LOG("IP " << getIPString(target) << " is not network of " << getMacString(src));
            return std::make_tuple<u16,u8*>(0,0);
        }

        if (targetHw == src)
        {
            // If the target device is the same as the requester, then dnon't waste anyone's time
            // with a reply. This could happen after a DHCP offer when the host sends and
            // an ARP query to make sure no one else is using that same IP. In our case,
            // The controller guarantees that.  TODO: would there be any other ligitimate reasons
            // to send such a query? i.e.: if a device has multiple IP?  It wouldn't make sense right?
            return std::make_tuple<u16,u8*>(0,0);
        }

        ARPFullMessage* r = new ARPFullMessage();
        memset(r,0,sizeof(ARPFullMessage));

        // we write in 6 bytes and overflow over 2. More efficient than memcpy
        *((uint64_t*)&r->l3.hwDestination[0]) = *((uint64_t*)&s->l3.hwSource[0]);
        convertMacAddressToNetworkOrder(targetHw,r->l3.hwSource); 
        r->l3.protoSource = target;
        r->l3.hwType = s->l3.hwType;
        r->l3.protocol = s->l3.protocol;
        r->l3.hwLength = s->l3.hwLength;
        r->l3.protoLength = s->l3.protoLength;
        r->l3.hwType = s->l3.hwType;
        r->l3.opcode = 0x0200;

        u8 srcMac[6] = {0,0,0,0,0,0};
        Ethernet ethernet(&r->l2);
        ethernet.setEtherType(__builtin_bswap16(0x0806));
        ethernet.setSource(srcMac);
        ethernet.setDestination(s->l2.srcMac);

        LOG("Sending ARP reply to " << getMacString(src) << ". " << getIPString(target) << " is at " << getMacString(targetHw));
        return std::make_tuple<u16,u8*>(sizeof(ARPFullMessage),(u8*)r);
    }

    return std::make_tuple<u16,u8*>(0,0);
}
