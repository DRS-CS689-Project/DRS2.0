#pragma once

#ifndef DIVFINDERSERVER_H
#define DIVFINDERSERVER_H

#include <list>
#include <string>
#include <thread>
#include <boost/multiprecision/cpp_int.hpp>

//namespace for boost lib to handle large numbers
//using namespace boost::multiprecision;

//number of time to run pollards rho before running the expensive prime brute force check
const unsigned int primecheck_depth = 1;

/* "Unsigned int type to hold original value and calculations" */
#define LARGEINT boost::multiprecision::uint128_t
//#define LARGEINT uint256_t uncomment for 256 bits

/* "Unsigned int twice as large as LARGEINT (bit-wise)" */
#define LARGEINT2X boost::multiprecision::uint256_t

/* "Signed int made of twice the bits as LARGEINT2X" */
#define LARGESIGNED2X boost::multiprecision::int512_t


class DivFinderServer {
public:
    DivFinderServer();
    DivFinderServer(LARGEINT input_value);
    ~DivFinderServer();

    void setVerbose(int lvl);

    LARGEINT getOrigVal() { return _orig_val; }

    LARGEINT calcPollardsRho(LARGEINT n);
 
    bool isPrimeBF(LARGEINT n, LARGEINT& divisor);
    bool isPrimeMR(LARGEINT n, LARGEINT k); 

    void factorThread(LARGEINT n);

    void setEndProcess(bool inputBool) { this->end_process = inputBool; };

    LARGEINT getPrimeDivFound() { return this->primeDivFound; };

    bool millerTest(LARGEINT2X d, LARGEINT2X n);

protected:

    LARGEINT2X modularPow(LARGEINT2X base, int exponent, LARGEINT2X modulus);
    LARGEINT2X power(LARGEINT2X x, LARGEINT2X y, LARGEINT2X p);
    
    int verbose = 0;

    LARGEINT primeDivFound = 0;

private:

    LARGEINT _orig_val;

    bool end_process = false;

};

#endif