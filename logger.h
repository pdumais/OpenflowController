#include <sstream>
#define LOG(x) {std::stringstream logss; logss << __FUNCTION__ <<": " << x; printf("%s\n",logss.str().c_str());}
