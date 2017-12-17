This is a simple openflow controller that I've built. It is built from scratch with no 3rd party openflow framework.
I've implemented this by reading the openflow 1.3 spec and with a lot of help of the excellent http://flowgrammable.org website.

The goal was to experiment and uderstand how openflow works. 


Files:
* OF*: those are the openflow handlers that parses the data received from the switch
* Server.cpp: The socket server listening for incomming connection
* Switch.cpp: Class that wraps a L2 switch functionality. Contains no openflow logic. This is used to make forwarding decisions.
* OpenFlowSwitch.cpp: Base class that does common functionnalities of an openflow controller
* SimpleLearningSwitch.cpp: This is the actual implementation of a controller application. It is the core class to implement
                              the behaviour of an L2 switch supporting vlans.
* testnet.txt: Commands used to create my testing environment

For more details about the SimpleLearningSwitch implementation: http://www.dumais.io/index.php?article=42968ec40e79c2eb3632de0b0f68af87
