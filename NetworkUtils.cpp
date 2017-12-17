#include "NetworkUtils.h"
#include <sstream>
#include <iomanip>


std::string getSourceMac(Ethernet* e)
{
    std::stringstream mac;
    mac << std::hex 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[0] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[1] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[2] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[3] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[4] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->srcMac[5];
    return mac.str();
    
}

std::string getDestinationMac(Ethernet* e)
{
    std::stringstream mac;
    mac << std::hex 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[0] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[1] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[2] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[3] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[4] << ":" 
        << std::setfill('0') << std::setw(2) << (int)e->dstMac[5];
    return mac.str();
}

std::string getMacString(MacAddress macAddress)
{
    uint8_t* mac = (uint8_t*)&macAddress;
    std::stringstream str;
    str << std::hex 
        << std::setfill('0') << std::setw(2) << (int)mac[0] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[1] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[2] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[3] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[4] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[5];
    return str.str();
}

MacAddress extractMacAddress(uint8_t* addr)
{
    uint64_t raw = *((uint64_t*)addr);
    raw = __builtin_bswap64(raw);
    raw = raw >> 16;
    return (MacAddress)raw; 
}

void convertMacAddressToNetworkOrder(MacAddress mac, uint8_t* buf)
{
    mac = mac << 16;
    mac = __builtin_bswap64(mac);
    *((uint64_t*)buf) = mac;
}
