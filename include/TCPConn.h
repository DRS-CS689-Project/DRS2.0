#ifndef TCPCONN_H
#define TCPCONN_H

#include "FileDesc.h"
#include "DataHelper.h"
#include "DivFinderServer.h"

const int max_attempts = 2;

// Methods and attributes to manage a network connection and prime factorization
class TCPConn 
{
public:
   TCPConn(LARGEINT number);
   ~TCPConn();

   bool accept(SocketFD &server);

   int sendText(const char *msg);
   int sendText(const char *msg, int size);

   bool handleConnection();
   
   bool getUserInput(std::string &cmd);

   void disconnect();
   bool isConnected();

   unsigned long getIPAddr() { return _connfd.getIPAddr(); };
   void getIPAddrStr(std::string &buf);

   bool getData(std::vector<uint8_t> &buf);
   bool sendData(std::vector<uint8_t> &buf);
  
   void sendNumber();
   bool waitForDivisor();
   void stopProcessing(LARGEINT newNum);
   void sendDie();

   bool foundAllPrimeFactors = false;

   LARGEINT getPrimeFactor(){ return this->primeFactor;};
   LARGEINT getNumber(){ return this->number;};
   
   int node = 0;
   int dbNum = 0;

   unsigned int tcpConnTask = 0;
private:
   LARGEINT primeFactor;

   LARGEINT refNumber;   

   LARGEINT number;

   enum statustype { s_connected, s_sendNumber, s_waitForReply, s_primeFound, s_sendStop};

   statustype _status = s_sendNumber;

   SocketFD _connfd;

   std::string _inputbuf;

   //helper class that deals with tagging
   DataHelper dataHelper;
};


#endif
