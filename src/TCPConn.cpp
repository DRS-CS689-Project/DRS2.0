#include <stdexcept>
#include <strings.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>
#include <iostream>
#include <fstream>
#include <memory>
#include "TCPConn.h"
#include "strfuncts.h"
#include <boost/lexical_cast.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include "DivFinderServer.h"

TCPConn::TCPConn(LARGEINT number) { 
   dataHelper = DataHelper();
   this->number = number;
}


TCPConn::~TCPConn() {

}

/**********************************************************************************************
 * accept - simply calls the acceptFD FileDesc method to accept a connection on a server socket.
 *
 *    Params: server - an open/bound server file descriptor with an available connection
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

bool TCPConn::accept(SocketFD &server) {
   return _connfd.acceptFD(server);
}

/**********************************************************************************************
 * handleConnection - performs a check of the connection, looking for data on the socket and
 *                    handling it based on the _status, or stage, of the connection
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

bool TCPConn::handleConnection() {

   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 100000000;

   bool foundPrime = false;

   try {
      std::vector<uint8_t> buf;
      switch (_status) {
         // send current number to be factored to client
         case s_sendNumber:    
            if (getData(buf))
            {
               std::string bufStr(buf.begin(), buf.end());
               buf.clear();
            }
            sendNumber();
            if (getData(buf))
            {
               std::string bufStr(buf.begin(), buf.end());
               buf.clear();
            }
            break;

         // wait for the divisor to come back from client
         // if waitForDivisor() returns true it means that this TCPConn object
         // returned a prime and tells TCPServer to reset all connections
         case s_waitForReply:
            foundPrime = waitForDivisor();
            break;

         // send the stop command to client and reset status to send the current
         // number to be factored
         case s_sendStop:
            sendData(dataHelper.c_stop);

            if (getData(buf))
            {
               std::string bufStr(buf.begin(), buf.end());
               buf.clear();
            }

            _status = s_sendNumber;
            break;

         default:
            throw std::runtime_error("Invalid connection status!");
            break;
      }
   } catch (socket_error &e) {
      std::cout << "Socket error, disconnecting.";
      disconnect();
      return false;
   }

   nanosleep(&sleeptime, NULL);
   return foundPrime;
}

/**********************************************************************************************
 * sendNumber - sends the current number to be factored to the client
 *
 **********************************************************************************************/

void TCPConn::sendNumber() {
   //updates task number
   this->tcpConnTask++;
   // Convert it to a string
   std::string bignumstr  = boost::lexical_cast<std::string>(this->number);

   // put it into a byte vector for transmission
   std::vector<uint8_t> buf(bignumstr.begin(), bignumstr.end());

   // wrap with num tags and send to client
   dataHelper.wrapCmd(buf, dataHelper.c_num, dataHelper.c_endnum);

   sendData(buf);

   // wait for reply
   _status = s_waitForReply;  
}

bool TCPConn::waitForDivisor(){
   if (_connfd.hasData()) {
      std::vector<uint8_t> task;
      std::vector<uint8_t> buf;

      if (!getData(buf))
         return false;

      std::string rawMesgStr(buf.begin(), buf.end());

      task = buf;

      if (!dataHelper.getCmdData(task, dataHelper.c_task, dataHelper.c_endtask)) 
         return false;
      

      std::string taskStr(task.begin(), task.end());

      unsigned int taskNumber = std::stoul(taskStr);

      if (!dataHelper.getCmdData(buf, dataHelper.c_prime, dataHelper.c_endprime)) 
         return false;
      

      if (taskNumber != this->tcpConnTask)
         return false;
      

      std::string primeStr(buf.begin(), buf.end());
      LARGEINT prime(primeStr);

      if (prime == 563){
         int val = 1;
      } 

      this->primeFactor = prime;
      this->number = this->number / prime;

      DivFinderServer df;
      LARGEINT l;

      if(this->number == 1) 
         foundAllPrimeFactors = true;
      else 
         _status = s_sendNumber;
      
      return true;
   }
   return false;
}

/**********************************************************************************************
 * getData - Reads in data from the socket and checks to see if there's an end command to the
 *           message to confirm we got it all
 *
 *    Params: None - data is stored in _inputbuf for retrieval with GetInputData
 *
 *    Returns: true if the data is ready to be read, false if they lost connection
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

bool TCPConn::getData(std::vector<uint8_t> &buf) {

   std::vector<uint8_t> readbuf;
   size_t count = 0;

   buf.clear();

   while (_connfd.hasData()) {
      // read the data on the socket up to 1024
      count += _connfd.readBytes<uint8_t>(readbuf, 1024);

      // check if we lost connection
      if (readbuf.size() == 0) {
         return false;
      }

      buf.insert(buf.end(), readbuf.begin(), readbuf.end());
   }
   return true;
}

/**********************************************************************************************
 * sendData - sends the data in the parameter to the socket
 *
 *    Params:  msg - the string to be sent
 *             size - if we know how much data we should expect to send, this should be populated
 *
 *    Throws: runtime_error for unrecoverable errors
 **********************************************************************************************/

bool TCPConn::sendData(std::vector<uint8_t> &buf) {
   _connfd.writeBytes<uint8_t>(buf);
   return true;
}


/**********************************************************************************************
 * stopProcessing - sets the status of this TCPConn object to s_sendStop to force it to send
 *                  stop command to client.
 *
 **********************************************************************************************/
void TCPConn::stopProcessing(LARGEINT newNum) {
   this->number = this->number/newNum;
   _status = s_sendStop;
}

/**********************************************************************************************
 * getUserInput - Gets user data and includes a buffer to look for a carriage return before it is
 *                considered a complete user input. Performs some post-processing on it, removing
 *                the newlines
 *
 *    Params: cmd - the buffer to store commands - contents left alone if no command found
 *
 *    Returns: true if a carriage return was found and cmd was populated, false otherwise.
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/

bool TCPConn::getUserInput(std::string &cmd) {
   std::string readbuf;

   // read the data on the socket
   _connfd.readFD(readbuf);

   // concat the data onto anything we've read before
   _inputbuf += readbuf;

   // If it doesn't have a carriage return, then it's not a command
   int crpos;
   if ((crpos = _inputbuf.find("\n")) == std::string::npos)
      return false;

   cmd = _inputbuf.substr(0, crpos);
   _inputbuf.erase(0, crpos+1);

   // Remove \r if it is there
   clrNewlines(cmd);

   return true;
}

/**********************************************************************************************
 * disconnect - cleans up the socket as required and closes the FD
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
void TCPConn::disconnect() {
   _connfd.closeFD();
}

/**********************************************************************************************
 * isConnected - performs a simple check on the socket to see if it is still open 
 *
 *    Throws: runtime_error for unrecoverable issues
 **********************************************************************************************/
bool TCPConn::isConnected() {
   return _connfd.isOpen();
}

/**********************************************************************************************
 * getIPAddrStr - gets a string format of the IP address and loads it in buf
 *
 **********************************************************************************************/
void TCPConn::getIPAddrStr(std::string &buf) {
   return _connfd.getIPAddrStr(buf);
}

/**********************************************************************************************
 * sendText - simply calls the sendText FileDesc method to send a string to this FD
 *
 *    Params:  msg - the string to be sent
 *             size - if we know how much data we should expect to send, this should be populated
 *
 *    Throws: runtime_error for unrecoverable errors
 **********************************************************************************************/

int TCPConn::sendText(const char *msg) {
   return sendText(msg, strlen(msg));
}

int TCPConn::sendText(const char *msg, int size) {
   if (_connfd.writeFD(msg, size) < 0) {
      return -1;  
   }
   return 0;
}

bool TCPConn::isNewIPAllowed(std::string inputIP){
   std::ifstream whitelistFile("whitelist");
   if(!whitelistFile){
      std::cout << "whitelist file not found" << std::endl;
      return false;
   }
   
   if (whitelistFile.is_open()){
      std::string line;
      while(whitelistFile >> line){
         //std::cout << "IP: " << inputIP << ", line: " << line << std::endl;//
         if (inputIP == line){
            std::cout << "New connection IP: "<< inputIP << " , authorized from whitelist" << std::endl;
            return true;
         }
      }
   }

   std::cout << "Match NOT FOUND!" << std::endl;
   return false;

}

/**********************************************************************************************
 * sendDie - sends the die command to the client telling it to disconnect
 *
 **********************************************************************************************/
void TCPConn::sendDie() {
   sendData(dataHelper.c_die);
}