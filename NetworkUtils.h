#pragma once
#include <string>
#include "Ethernet.h"


std::string getMacString(MacAddress mac);
std::string getCIDRString(u32 ip,u32 mask);
std::string getIPString(u32 ip);

MacAddress extractMacAddress(u8* addr);
void convertMacAddressToNetworkOrder(MacAddress mac, u8* buf);

