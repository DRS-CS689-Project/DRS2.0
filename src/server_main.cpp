/****************************************************************************************
 * tcp_server - program that sets up a basic TCP server for factoring large numbers
 *
 *
 ****************************************************************************************/  

#include <stdexcept>
#include <iostream>
#include <getopt.h>
#include "TCPServer.h"
#include "exceptions.h"
#include "DivFinderServer.h"

using namespace std; 

void displayHelp(const char *execname) {
   std::cout << execname << " [-f <number to factor>] [-p <portnum>] [-a <ip_addr>] [-v <verbosity 0-1>]\n";
}

// global default values
const unsigned short default_port = 9999;
const char default_IP[] = "127.0.0.1";

int main(int argc, char *argv[]) {
   unsigned short port = default_port;
   std::string ip_addr(default_IP);

   // Get the command line arguments and set params appropriately
   int c = 0;
   long portval;
   LARGEINT number = 0;
   int numNodes = 2;
   int verbosity = 0;

   while ((c = getopt(argc, argv, "f:n:a:p:v:")) != -1) {
        switch (c) {

        // Set the number to factor	    
        case 'f':
            number = static_cast<LARGEINT>( LARGEINT(optarg));
            break;

        // Set the number of nodes in the distributed system
        case 'n':
            numNodes = stoi(optarg, NULL, 10);
            break;

        // IP address to attempt to bind to
        case 'a':
            ip_addr = optarg; 
            break;

        // Port number to bind to
        case 'p':
            portval = strtol(optarg, NULL, 10);
            if ((portval < 1) || (portval > 65535)) {
                std::cout << "Invalid port. Value must be between 1 and 65535";
                exit(0);
            }
            port = (unsigned short) portval;
            break;
        
        // Set the verbosity
        case 'v':
            verbosity = stoi(optarg, NULL, 10);
            if(verbosity != 0 && verbosity != 1) {
                std::cout << "Invalid verbosity. Value must be either 0 or 1\n";
                exit(0);
            }
            break;

        case '?':
            displayHelp(argv[0]);
            return 0;

        default:
            std::cout << "Unknown command line option '" << c << "'\n";
            displayHelp(argv[0]);
            break;
        }
   }

   if(number == 0) {
       std::cout << "Number to factor is a required argument\n";
       displayHelp(argv[0]);
       return -1;
   }

   // Try to set up the server for listening
   TCPServer server(number, numNodes, verbosity);
   try {
      std::cout << "Binding server to " << ip_addr << " port " << port << endl;
      server.bindSvr(ip_addr.c_str(), port);

   } catch (invalid_argument &e) 
   {
      cerr << "Server initialization failed: " << e.what() << endl;
      return -1;
   }	   

   std::cout << "Server established. Waiting for " << numNodes << " nodes to join.\n";

   try {
      std::cout << "Listening.\n";	   
      server.runServer();
   } catch (socket_error &e) {
      cerr << "Unrecoverable socket error. Exiting.\n";
      cerr << "Error is: " << e.what() << endl;
      return -1;
   }

   server.shutdown();

   std::cout << "Server shut down\n";
   return 0;
}
