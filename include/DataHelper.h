#ifndef DATAHELPER_H
#define DATAHELPER_H

#include <netinet/in.h>
#include <vector>

/******************************************************************************************
 * DataHelper - Parent class for student's TCP server
 *
 *  	   Server(Const): right now does nothing (still should call for future portability
 *  	   ~Server(Dest): no cleanup required in parent
 *
 *  	   bind - binds to an IP address and port on the server
 *  	   listen - starts listening on a single-threaded process (non-blocking) and loops
 *                listening for connections and handling the existing connections
 *  	   shutdown - shuts down the server properly
 *  	   
 *  	   Exceptions: sub-classes should throw a std::exception with the what string field
 *  	               populated for any issues.
 *
 *****************************************************************************************/

class DataHelper { 
   public:
        DataHelper();
        ~DataHelper();

        std::vector<uint8_t>::iterator findCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &cmd);
        bool hasCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &cmd);
        bool getCmdData(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd, std::vector<uint8_t> &endcmd);
        void wrapCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd, std::vector<uint8_t> &endcmd);

        std::vector<uint8_t> c_task, c_endtask, c_num, c_endnum, c_prime, c_endprime, c_stop, c_die;
};

#endif
