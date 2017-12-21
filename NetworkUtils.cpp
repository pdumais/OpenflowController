#include "NetworkUtils.h"
#include <sstream>
#include <iomanip>
#include <netinet/ip.h>
#include <arpa/inet.h>

std::string getMacString(MacAddress macAddress)
{
    u8* mac = (u8*)&macAddress;
    std::stringstream str;
    str << std::hex 
        << std::setfill('0') << std::setw(2) << (int)mac[5] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[4] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[3] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[2] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[1] << ":" 
        << std::setfill('0') << std::setw(2) << (int)mac[0];
    return str.str();
}

MacAddress extractMacAddress(u8* addr)
{
    u64 raw = *((u64*)addr);
    raw = __builtin_bswap64(raw);
    raw = raw >> 16;
    return (MacAddress)raw; 
}

void convertMacAddressToNetworkOrder(MacAddress mac, u8* buf)
{
    mac = mac << 16;
    mac = __builtin_bswap64(mac);
    *((u64*)buf) = mac;
}

std::string getCIDRString(u32 ip,u32 mask)
{
    //Warning, ip and mask are assumed to be in network byte order
    std::stringstream str;
    str << (int)(ip&0xFF) << "." << (int)((ip>>8)&0xFF) << "." << (int)((ip>>16)&0xFF) << "." << (int)((ip>>24)&0xFF);
    if (mask == 0)
    {
        str << "/0";
    }
    else
    {
        str << "/" << (32-__builtin_clz(mask));
    }
    return str.str();
}

std::string getIPString(u32 ip)
{
    //Warning, ip and mask are assumed to be in network byte order
    std::stringstream str;
    str << (int)(ip&0xFF) << "." << (int)((ip>>8)&0xFF) << "." << (int)((ip>>16)&0xFF) << "." << (int)((ip>>24)&0xFF);
    return str.str();
}
