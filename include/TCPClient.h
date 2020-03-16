#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <string>
#include "Client.h"
#include "FileDesc.h"
#include "DivFinderServer.h"
#include "DataHelper.h"

// The amount to read in before we send a packet
const unsigned int socket_bufsize = 100;

class TCPClient : public Client
{
public:
   TCPClient();
   ~TCPClient();

   virtual void connectTo(const char *ip_addr, unsigned short port);
   virtual void handleConnection();

   virtual void closeConn();
   bool getData(std::vector<uint8_t> &buf);

   DivFinderServer d;
   std::unique_ptr<std::thread> th;
   std::string inputNum;

   bool initMessage = true;
   bool activeThread = false;

private:
   // Stores the user's typing
   std::string _in_buf;

   // Class to manage our client's network connection
   SocketFD _sockfd;

   unsigned int clientTask = 0;

   //helper class that deals with tagging
   DataHelper dataHelper;

};


#endif
