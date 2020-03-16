#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdexcept>
#include <strings.h>
#include <vector>
#include <iostream>
#include <memory>
#include <sstream>
#include "TCPServer.h"
#include "ALMgr.h"
#include "DivFinderServer.h"
#include <chrono>
#include <thread>

TCPServer::TCPServer(LARGEINT number, int numNodes, int verbosity){
   this->number = number;
   this->numOfNodes = numNodes;
   this->verbosity = verbosity;
}


TCPServer::~TCPServer() {

}

/**********************************************************************************************
 * bindSvr - Creates a network socket and sets it nonblocking so we can loop through looking for
 *           data. Then binds it to the ip address and port
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::bindSvr(const char *ip_addr, short unsigned int port) {

   struct sockaddr_in servaddr;

   // Set the socket to nonblocking
   _sockfd.setNonBlocking();

   _sockfd.setReusable();

   // Load the socket information to prep for binding
   _sockfd.bindFD(ip_addr, port);
 
}

/**********************************************************************************************
 * listenSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *             them. Also loops through the list of connections and handles data received and
 *             sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::listenSvr() {
   _sockfd.listenFD(5);
}


/**********************************************************************************************
 * runSvr - Performs a loop to look for connections and create TCPConn objects to handle
 *          them. Also loops through the list of connections and handles data received and
 *          sending of data. 
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::runServer() {
   bool online = true;
   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;

   // Start the server socket listening
   listenSvr();

   while (online) {
      // wait until all nodes have connected
      if(_connlist.size() != numOfNodes)
         handleSocket();

      // if all nodes have connected, start processing
      if(_connlist.size() == numOfNodes)
         // if handleConnections returns a true it means all primes have been
         // found so shut down server
         if(handleConnections()) {
            std::cout << "Prime factors: " << std::endl;
            for (auto& pf : this->primeFactorsVector){
               std::cout << pf << std::endl;
            }
            
            online = false;
         }
         
      // So we're not chewing up CPU cycles unnecessarily
      nanosleep(&sleeptime, NULL);
   }
}


/**********************************************************************************************
 * handleSocket - Checks the socket for incoming connections and validates against the whitelist.
 *                Accepts valid connections and adds them to the connection list.
 *
 *    Returns: pointer to a new connection if one was found, otherwise NULL
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

TCPConn *TCPServer::handleSocket() {
  
   // The socket has data, means a new connection 
   if (_sockfd.hasData()) {

      struct sockaddr_in cliaddr;
      socklen_t len = sizeof(cliaddr);

      if (_sockfd.hasData()) {
         TCPConn *new_conn = new TCPConn(this->number);
         //if can't accept return
         if (!new_conn->accept(_sockfd)) 
            return NULL;
         
         // Get their IP Address string to use in logging
         std::string ipaddr_str;
         new_conn->getIPAddrStr(ipaddr_str);

         // Check the whitelist
         ALMgr al("whitelist");
         if (!al.isAllowed(new_conn->getIPAddr()))
         {
            // Disconnect the user
            std::cout << "Connection from unauthorized IP address" << std::endl;
            new_conn->sendText("Not Authorized To Log into System\n");
            new_conn->sendDie();

            return NULL;
         }

         std::cout << "Got a connection from " << ipaddr_str << std::endl;

         _connlist.push_back(std::unique_ptr<TCPConn>(new_conn));

         if(_connlist.size() != numOfNodes) 
            new_conn->sendText("Waiting for all nodes to connect...\n");
         if(_connlist.size() == numOfNodes)
            std::cout << "All nodes have joined...finding prime factors.\n";

         new_conn->node = nodes;
         nodes++;

         return new_conn;
      }
   }
   return NULL;
}

/**********************************************************************************************
 * handleConnections - Loops through the list of clients, running their functions to handle the
 *                     clients input/output.
 *
 *    Returns: number of new connections accepted
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPServer::handleConnections() {
      std::list<std::unique_ptr<TCPConn>>::iterator tptr = _connlist.begin();
      if(initTime) {
         start = std::chrono::high_resolution_clock::now();
         initTime = false;
      }
      // Loop through our connections, handling them
      while (tptr != _connlist.end())
      {
         if((*tptr)->foundAllPrimeFactors) {
            auto stop = std::chrono::high_resolution_clock::now();
            std::chrono::duration<double> elapsed_seconds = stop - start;
            std::cout << "Execution Time: " << elapsed_seconds.count() << " seconds\n";

            //if we have found all prime factors tell all connections to disconnect
            std::list<std::unique_ptr<TCPConn>>::iterator tptr3 = _connlist.begin();
            for(; tptr3 != _connlist.end(); tptr3++) {
               (*tptr3)->sendDie();
            }
            std::this_thread::sleep_for(std::chrono::seconds(1));
            return true;
         }

         // If the user lost connection
         if (!(*tptr)->isConnected()) {
            // Remove them from the connect list
            tptr = _connlist.erase(tptr);
            if(verbosity)
               std::cout << "Connection disconnected.\n";
            continue;
         }

         // Process any user inputs
         if((*tptr)->handleConnection()) {
            auto primeFound = (*tptr)->getPrimeFactor();
            if(verbosity)
               std::cout << "In TCPServer - primeFound: " << primeFound << std::endl;
            this->primeFactorsVector.emplace_back(primeFound);

            //a prime was found so send stop message to all other connections
            std::list<std::unique_ptr<TCPConn>>::iterator tptr2 = _connlist.begin();
            for(; tptr2 != _connlist.end(); tptr2++){
               if (tptr2 != tptr)
                  (*tptr2)->stopProcessing(primeFound);
            }  
         }
         // Increment our iterator
         tptr++;
      }
      return false;
}

/**********************************************************************************************
 * shutdown - Cleanly closes the socket FD.
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPServer::shutdown() {
   _sockfd.closeFD();
}