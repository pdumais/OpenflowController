#pragma once
#include <string>

typedef uint64_t MacAddress;

struct Ethernet
{
    uint8_t dstMac[6];
    uint8_t srcMac[6];
    uint16_t etherType;
    
}__attribute__((__packed__));

struct EthernetVlan
{
    uint8_t dstMac[6];
    uint8_t srcMac[6];
    uint16_t t8100;
    uint16_t vlan;
    uint16_t etherType;
}__attribute__((__packed__));
    

std::string getMacString(MacAddress mac);
std::string getSourceMac(Ethernet* e);
std::string getDestinationMac(Ethernet* e);
MacAddress extractMacAddress(uint8_t* addr);
void convertMacAddressToNetworkOrder(MacAddress mac, uint8_t* buf);
