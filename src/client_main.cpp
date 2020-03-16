/****************************************************************************************
 * tcp_client - connects to a TCP server for two-way comms
 *
 *              **Students should not modify this code! Or at least you can to test your
 *                code, but your code should work with the unmodified version
 *
 ****************************************************************************************/  

#include <stdexcept>
#include <iostream>
#include <getopt.h>
#include "TCPClient.h"

using namespace std; 

void displayHelp(const char *execname) {
   std::cout << execname << " -a <ip_addr> -p <port> -v <verbosity>\n";
}


int main(int argc, char *argv[]) {
   int verbosity = 0;
   int c = 0;
   long portval = -1;
   std::string ip_addr = "127.0.0.1";
   unsigned short port = 9999;


   // Check the command line input & Get the command line arguments and set params appropriately
   while ((c = getopt(argc, argv, "a:p:v:")) != -1) {
      switch (c) {
         case 'a':
            ip_addr = optarg;
            //std::cout << "a: " << ip_addr << std::endl;
            break;

         case 'v':
            verbosity = stoi(optarg, NULL, 10);
            //std::cout << "v: " << verbosity << std::endl;
            if(verbosity != 0 && verbosity != 1)
            {
               std::cout << "Invalid verbosity. Value must be either 0 or 1\n";
               exit(0);
            }            
            break;

         case 'p':
            portval = strtol(optarg, NULL, 10);
            //std::cout << "p: " << portval << std::endl;
            if ((portval < 1) || (portval > 65535)) {
                std::cout << "Invalid port. Value must be between 1 and 65535";
                exit(0);
            }
            port = (unsigned short) portval;
            break;

         default:
            displayHelp(argv[0]);
            break;
      }
   }

   // Try to set up the server for listening
   TCPClient client(verbosity);
   try {
      cout << "Connecting to " << ip_addr << " port " << port << endl;
      client.connectTo(ip_addr.c_str(), port);

   } catch (socket_error &e)
   {
      cerr << "Connection failed: " << e.what() << endl;
      return -1;
   }	   

   cout << "Connection established.\n";

   try {
      client.handleConnection();

      client.closeConn();
      cout << "Client disconnected\n";

   } catch (runtime_error &e) {
      cerr << "Client error received: " << e.what() << endl;
      return -1;      
   }

   return 0;
}
