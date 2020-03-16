#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdexcept>
#include <strings.h>
#include <stropts.h>
#include <string.h>
#include <sys/select.h>
#include <stdio.h>
#include <stdexcept>
#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <boost/multiprecision/cpp_int.hpp>
#include <boost/integer/common_factor.hpp>
#include <thread>


#include "TCPClient.h"
#include "DivFinderServer.h"
#include "strfuncts.h"
//#include "threadSafeRandGen.h"


/**********************************************************************************************
 * TCPClient (constructor) - Creates a Stdin file descriptor to simplify handling of user input. 
 *
 **********************************************************************************************/

TCPClient::TCPClient() {
   dataHelper = DataHelper();
}

/**********************************************************************************************
 * TCPClient (destructor) - No cleanup right now
 *
 **********************************************************************************************/

TCPClient::~TCPClient() {

}

/**********************************************************************************************
 * connectTo - Opens a File Descriptor socket to the IP address and port given in the
 *             parameters using a TCP connection.
 *
 *    Throws: socket_error exception if failed. socket_error is a child class of runtime_error
 **********************************************************************************************/

void TCPClient::connectTo(const char *ip_addr, unsigned short port) {
   if (!_sockfd.connectTo(ip_addr, port))
      throw socket_error("TCP Connection failed!");

}

/**********************************************************************************************
 * handleConnection - Performs a loop that checks if the connection is still open, then 
 *                    looks for user input and sends it if available. Finally, looks for data
 *                    on the socket and sends it.
 * 
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::handleConnection() {
      bool connected = true;
   int sin_bufsize = 0;
   ssize_t rsize = 0;

   timespec sleeptime;
   sleeptime.tv_sec = 0;
   sleeptime.tv_nsec = 1000000;

   // Loop while we have a valid connection
   while (connected) {

      // If the connection was closed, exit
      if (!_sockfd.isOpen())
         break;

      // Read any data from the socket and display to the screen and handle errors
      
      if (_sockfd.hasData()) {
         std::vector<uint8_t> buf;

         if (!getData(buf)) {
            continue;
         }

         std::string rawMesgStr(buf.begin(), buf.end());
         std::cout << "Client Recieved Raw Message: " << rawMesgStr << std::endl;

         if (!(dataHelper.findCmd(buf, dataHelper.c_die) == buf.end()))
         {
            closeConn();
         }

         if (!(dataHelper.findCmd(buf, dataHelper.c_stop) == buf.end()))
         {

            this->d.setEndProcess(true);
            this->activeThread = false;
            if (this->th != nullptr)
            {
               this->th->join();
               this->th.reset(nullptr);
            }

         }

         if (dataHelper.getCmdData(buf, dataHelper.c_num, dataHelper.c_endnum)) {
            std::string numStr(buf.begin(), buf.end());

            this->inputNum = numStr;
            std::cout << "recieved : " << numStr << std::endl;

            //Used 563, 197, 197, 163, 163, 41, 41, 
            uint128_t num = static_cast<uint128_t>(this->inputNum);
            
            this->d = DivFinderServer(num);
            this->d.setVerbose(0);

            this->clientTask++;

            //runs pollards rho on the number to find the divisor in the separate thread
            this->th = std::make_unique<std::thread>(&DivFinderServer::factorThread, &this->d, num);

            //set flag to alert program is active
            this->activeThread = true;
         }
      }

      //check if thread was spawned
      if (this->activeThread){
         //send the found divisor to the main server
         if(this->d.getPrimeDivFound() != 0){
            std::cout << "Prime Divisor Found: " << this->d.getPrimeDivFound() << std::endl;
            this->activeThread = false;

            std::string taskStr = std::to_string(this->clientTask);
            std::vector<uint8_t> task(taskStr.begin(), taskStr.end());
            
            dataHelper.wrapCmd(task, dataHelper.c_task, dataHelper.c_endtask);

            std::string bignumstr  = boost::lexical_cast<std::string>(d.getPrimeDivFound());

            // put it into a byte vector for transmission
            std::vector<uint8_t> buf(bignumstr.begin(), bignumstr.end());

            dataHelper.wrapCmd(buf, dataHelper.c_prime, dataHelper.c_endprime);
               
            std::string mesg = static_cast<std::string>(d.getPrimeDivFound());
            mesg = mesg + "\n"; 
            std::cout << "Sending: " << this->clientTask << " " << mesg << std::endl;
            //std::this_thread::sleep_for(std::chrono::seconds(1));
            
            if (this->th != nullptr)
            {
               this->th->join();
               this->th.reset(nullptr);
            }

            std::vector<uint8_t> finalBuf;
            finalBuf.reserve(task.size() + buf.size());
            finalBuf.insert(finalBuf.end(), task.begin(), task.end() );
            finalBuf.insert(finalBuf.end(), buf.begin(), buf.end() );

            _sockfd.writeBytes<uint8_t>(finalBuf);
         }
      }
   }
}

/**********************************************************************************************
 * closeConnection - Your comments here
 *
 *    Throws: socket_error for recoverable errors, runtime_error for unrecoverable types
 **********************************************************************************************/

void TCPClient::closeConn() {
    _sockfd.closeFD(); 
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

bool TCPClient::getData(std::vector<uint8_t> &buf) {
   std::vector<uint8_t> readbuf;
   size_t count = 0;

   buf.clear();

   while (_sockfd.hasData()) {
      // read the data on the socket up to 1024
      count += _sockfd.readBytes<uint8_t>(readbuf, 1024);

      // check if we lost connection
      if (readbuf.size() == 0) 
         return false;
      
      buf.insert(buf.end(), readbuf.begin(), readbuf.end());

   }
   return true;
}