#include "DataHelper.h"
#include <algorithm>

DataHelper::DataHelper() {
    uint8_t slash = (uint8_t) '/';

    c_task.push_back((uint8_t) '<');
    c_task.push_back((uint8_t) 'T');
    c_task.push_back((uint8_t) 'S');
    c_task.push_back((uint8_t) 'K');
    c_task.push_back((uint8_t) '>');

    c_endtask = c_task;
    c_endtask.insert(c_endtask.begin()+1, 1, slash);

    c_num.push_back((uint8_t) '<');
    c_num.push_back((uint8_t) 'N');
    c_num.push_back((uint8_t) 'U');
    c_num.push_back((uint8_t) 'M');
    c_num.push_back((uint8_t) '>');

    c_endnum = c_num;
    c_endnum.insert(c_endnum.begin()+1, 1, slash);

    c_prime.push_back((uint8_t) '<');
    c_prime.push_back((uint8_t) 'P');
    c_prime.push_back((uint8_t) 'R');
    c_prime.push_back((uint8_t) 'I');
    c_prime.push_back((uint8_t) 'M');
    c_prime.push_back((uint8_t) 'E');
    c_prime.push_back((uint8_t) '>');

    c_endprime = c_prime;
    c_endprime.insert(c_endprime.begin()+1, 1, slash);

    c_stop.push_back((uint8_t) '<');
    c_stop.push_back((uint8_t) 'S');
    c_stop.push_back((uint8_t) 'T');
    c_stop.push_back((uint8_t) 'O');
    c_stop.push_back((uint8_t) 'P');
    c_stop.push_back((uint8_t) '>');

    c_die.push_back((uint8_t) '<');
    c_die.push_back((uint8_t) 'D');
    c_die.push_back((uint8_t) 'I');
    c_die.push_back((uint8_t) 'E');
    c_die.push_back((uint8_t) '>');
}

DataHelper::~DataHelper() {

}


/**********************************************************************************************
 * findCmd - returns an iterator to the location of a string where a command starts
 * hasCmd - returns true if command was found, false otherwise
 *
 *    Params: buf = the data buffer to look for the command within
 *            cmd - the command string to search for in the data
 *
 *    Returns: iterator - points to cmd position if found, end() if not found
 *
 **********************************************************************************************/

std::vector<uint8_t>::iterator DataHelper::findCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &cmd) {
   return std::search(buf.begin(), buf.end(), cmd.begin(), cmd.end());
}

bool DataHelper::hasCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &cmd) {
   return !(findCmd(buf, cmd) == buf.end());
}

/**********************************************************************************************
 * getCmdData - looks for a startcmd and endcmd and returns the data between the two 
 *
 *    Params: buf = the string to search for the tags
 *            startcmd - the command at the beginning of the data sought
 *            endcmd - the command at the end of the data sought
 *
 *    Returns: true if both start and end commands were found, false otherwisei
 *
 **********************************************************************************************/

bool DataHelper::getCmdData(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd, 
                                                    std::vector<uint8_t> &endcmd) {
   std::vector<uint8_t> temp = buf;
   auto start = findCmd(temp, startcmd);
   auto end = findCmd(temp, endcmd);

   if ((start == temp.end()) || (end == temp.end()) || (start == end))
      return false;

   buf.assign(start + startcmd.size(), end);
   return true;
}


/**********************************************************************************************
 * wrapCmd - wraps the command brackets around the passed-in data
 *
 *    Params: buf = the string to wrap around
 *            startcmd - the command at the beginning of the data
 *            endcmd - the command at the end of the data
 *
 **********************************************************************************************/

void DataHelper::wrapCmd(std::vector<uint8_t> &buf, std::vector<uint8_t> &startcmd,
                                                    std::vector<uint8_t> &endcmd) {
   std::vector<uint8_t> temp = startcmd;
   temp.insert(temp.end(), buf.begin(), buf.end());
   temp.insert(temp.end(), endcmd.begin(), endcmd.end());

   buf = temp;
}
