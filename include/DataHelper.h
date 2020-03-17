#ifndef DATAHELPER_H
#define DATAHELPER_H

#include <netinet/in.h>
#include <vector>

// Utility class created to make working with data and tags associated with server/client
// communication easier. This class also specifies the types of tags that are avaliable for
// use in communications

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
