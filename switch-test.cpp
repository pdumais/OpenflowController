#include "Switch.h"
#include <string>
#include <iomanip>
#include <iostream>
#include <sstream>

#define MAC1 0x010203040501
#define MAC2 0x010203040502
#define MAC3 0x010203040503
#define MAC4 0x010203040504
#define MAC5 0x010203040505
#define MAC6 0x010203040506
#define BROADCAST 0xFFFFFFFFFF

#define ERROR() {printf("ERROR %i\r\n",__LINE__);}
#define LOG(x) {std::stringstream logss; logss << x; printf("%s\n",logss.str().c_str());}

Switch* newSwitch(int upCount)
{
    Switch* s = new Switch();
    s->addPort(1,"p1");
    s->addPort(2,"p2");
    s->addPort(3,"p3");
    s->addPort(4,"p4");
    s->addPort(5,"p5");
    s->addPort(6,"p6");
    for (int i=1;i<=upCount;i++) s->setPortState(i,SwitchPortState::Up);

    return s;
}

bool testOutPort(Switch *s, u32 in, u16 tag, MacAddress mac, std::vector<OutPortInfo> expected)
{
    OutPortResult ret = s->getOutPorts(in,tag,mac);

    if (ret.ports.size() != expected.size())
    {
        LOG("returned list = "<<ret.ports.size() << ", expected " << expected.size() <<"\r\n");
        return false;
    }
    for (int i = 0; i < ret.ports.size(); i++)
    {
        if (ret.ports[i].port != expected[i].port) return false;
        if (ret.ports[i].vlanTag != expected[i].vlanTag) return false;
    }

    return true;
}

int main(int argc, char** argv)
{
    std::cout << "Testing Switch\r\n";

    Switch* s;
    s = newSwitch(3);    


    //Flood with some ports down, no vlans initialized, no vlan tag 
    if (!testOutPort(s,1,0,MAC1,{{2,0},{3,0}})) ERROR();

    // sending from down port
    if (!testOutPort(s,4,0,MAC1,{})) ERROR();
    
    // Learn with no tag on an access port
    s->learn(2,0,MAC2);
    if (!testOutPort(s,1,0,MAC2,{{2,0}})) ERROR();

    // Learn with a tag on an access port. Tag should be ignored
    delete s; s = newSwitch(3);    
    s->learn(2,500,MAC2);
    if (!testOutPort(s,1,0,MAC2,{{2,0}})) ERROR();

    // Learn and then put port down, should flood to other ports
    delete s; s = newSwitch(3);    
    s->learn(2,0,MAC2);
    s->setPortState(2,SwitchPortState::Down);    
    if (!testOutPort(s,1,0,MAC2,{{3,0}})) ERROR();
    
    // Test that we cant learn broadcast, and test broadcast at the same time
    delete s; s = newSwitch(4);    
    s->learn(4,0,BROADCAST);
    if (!testOutPort(s,1,0,BROADCAST,{{2,0},{3,0},{4,0}})) ERROR();

    // Test flooding when there are 2 vlans
    delete s; s = newSwitch(6);    
    s->setPortModeAccess(1,100);
    s->setPortModeAccess(2,100);
    s->setPortModeAccess(3,100);
    s->setPortModeAccess(4,200);
    s->setPortModeAccess(5,200);
    s->setPortModeAccess(6,200);
    if (!testOutPort(s,1,0,BROADCAST,{{2,0},{3,0}})) ERROR();
    if (!testOutPort(s,4,0,BROADCAST,{{5,0},{6,0}})) ERROR();


    // Test flooding with 2 vlans and 2 trunk ports
    delete s; s = newSwitch(5);    
    s->setPortModeAccess(1,100);
    s->setPortModeAccess(2,100);
    s->setPortModeAccess(3,200);
    s->setPortModeAccess(4,200);
    s->setPortModeTrunk(5,{100,200},300);
    if (!testOutPort(s,1,0,BROADCAST,{{2,0},{5,100}})) ERROR();
    if (!testOutPort(s,3,0,BROADCAST,{{4,0},{5,200}})) ERROR();
    if (!testOutPort(s,5,100,BROADCAST,{{1,0},{2,0}})) ERROR();
    if (!testOutPort(s,5,200,BROADCAST,{{3,0},{4,0}})) ERROR();

    // test trunk flooding on another trunk
    s->setPortModeTrunk(4,{200,300},500);
    s->setPortModeAccess(1,300);
    if (!testOutPort(s,5,200,BROADCAST,{{3,0},{4,200}})) ERROR();

    // test untagged traffic comming on trunk (default vlan)
    if (!testOutPort(s,5,0,BROADCAST,{{1,0},{4,300}})) ERROR();

    // send tag that trunk is not member of. Should drop
    if (!testOutPort(s,5,150,BROADCAST,{})) ERROR();
    
    // send to mac that is not on the same vlan (should flood on own vlan)
    delete s; s = newSwitch(6);    
    s->setPortModeAccess(1,100);
    s->setPortModeAccess(2,100);
    s->setPortModeAccess(3,100);
    s->setPortModeAccess(4,200);
    s->setPortModeAccess(5,200);
    s->setPortModeAccess(6,200);
    s->learn(2,0,MAC2);
    s->learn(5,0,MAC5);
    if (!testOutPort(s,1,0,MAC2,{{2,0}})) ERROR();
    if (!testOutPort(s,1,0,MAC5,{{2,0},{3,0}})) ERROR(); // Flood behaviour
    if (!testOutPort(s,4,0,MAC5,{{5,0}})) ERROR();
    if (!testOutPort(s,4,0,MAC2,{{5,0},{6,0}})) ERROR(); // Flood behaviour

    // remove port of learned mac and send to that mac (should flood)
    s->removePort(2);
    if (!testOutPort(s,1,0,MAC2,{{3,0}})) ERROR(); // Flood behaviour

    // learned mac starts sending from another port
    delete s; s = newSwitch(6);    
    s->setPortModeAccess(1,100);
    s->setPortModeAccess(2,100);
    s->setPortModeAccess(3,100);
    s->setPortModeAccess(4,200);
    s->setPortModeAccess(5,200);
    s->setPortModeAccess(6,200);
    s->learn(2,0,MAC2);
    if (!testOutPort(s,1,0,MAC2,{{2,0}})) ERROR();
    s->learn(3,0,MAC2);
    if (!testOutPort(s,1,0,MAC2,{{3,0}})) ERROR();

    // If host changes port, but that port is on another vlan, 
    // both vlans should contain that mac. In that case,
    // the switch would just that two devices with same mac
    // are connected on the same switch but with different vlans
    // the one on the old vlan would then just be a stale entry
    s->learn(4,0,MAC2);
    if (!testOutPort(s,1,0,MAC2,{{3,0}})) ERROR();
    if (!testOutPort(s,5,0,MAC2,{{4,0}})) ERROR();


}
