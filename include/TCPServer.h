#ifndef TCPSERVER_H
#define TCPSERVER_H

#include <list>
#include <memory>
#include "Server.h"
#include "FileDesc.h"
#include "TCPConn.h"
#include "DivFinderServer.h"
#include <chrono>
#include <boost/optional.hpp>
#include <vector>

class TCPServer : public Server 
{
public:
   TCPServer(LARGEINT number, int numNodes, int verbosity);
   ~TCPServer();

   virtual void bindSvr(const char *ip_addr, unsigned short port);
   void listenSvr();
   virtual void runServer();

   void shutdown();

   TCPConn *handleSocket();
   virtual bool handleConnections();

   int nodes = 1;
   unsigned int serverTask = 0;

   std::chrono::system_clock::time_point start;

   bool initTime = true;
   // stores the resulting prime factors
   std::vector<LARGEINT> primeFactorsVector; 

private:
   // Class to manage the server socket
   SocketFD _sockfd;
 
   // List of TCPConn objects to manage connections
   std::list<std::unique_ptr<TCPConn>> _connlist;
   // The number to find the prime factors of
   LARGEINT number;
   // The number of nodes in this system
   int numOfNodes = 2;

   int verbosity = 0;
};


#endif
