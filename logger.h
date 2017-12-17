#include <sstream>
#define LOG(x) {std::stringstream logss; logss << x; printf("%s\n",logss.str().c_str());}
