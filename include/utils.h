/*! \file utils.h
 *  \brief Util methods for RPC modules
 *  \author Mykhailo Dalchenko <mykhailo.dalchenko@cern.ch>
 *  \author Brian Dorney <brian.l.dorney@cern.ch>
 */

#ifndef UTILS_H
#define UTILS_H

#include "moduleapi.h"
//#include <libmemsvc.h>
#include "memhub.h"
#include "lmdb_cpp_wrapper.h"
#include "xhal/utils/XHALXMLParser.h"
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <fstream>
#include <vector>
#include <iterator>
#include <cstdio>

memsvc_handle_t memsvc; /// \var global memory service handle required for registers read/write operations

struct ParamCalPulse{
    bool enable; //true (false) turn on (off) calpulse
    bool isCurrent; //true (false) is current injection (voltage pulse)

    uint32_t duration; //duration in BX's (CFG_CAL_DUR)
    uint32_t extVoltStep; //External voltage step 0->disable; 1->enable (CFG_CAL_EXT)
    uint32_t height; //height of calpulse (CFG_CAL_DAC)
    uint32_t phase; //phase of calpulse (CFG_CAL_PHI)
    uint32_t polarity; //polarity of calpulse 0->pos; 1->neg (CFG_CAL_SEL_POL)
    uint32_t scaleFactor; //current pulse scale factor (CFG_CAL_FS)

    ParamCalPulse(){
        enable = false;
        isCurrent = false;

        duration = 0x1ff;
        extVoltStep = 0x0;
        height = 0x0;
        phase = 0x0;
        polarity = 0x0;
        scaleFactor = 0x0;
    }
}; //End ParamCalPulse

struct ParamScan{
    //Hardware selection
    uint32_t ohN;
    uint32_t ohMask;
    uint32_t vfatN;
    uint32_t vfatMask;
    uint32_t chan;      //channel of interest

    //Params
    bool useUltra;
    bool useExtTrig;

    uint32_t dacMax;    //Maximum dac value
    uint32_t dacMin;    //Minimum dac value
    uint32_t dacSelect; //DAC to use for Monitoring
    uint32_t dacStep;   //step size for dac
    uint32_t nevts;     //Number of events
    uint32_t waitTime;  //unit of time; uints depend on function

    std::string scanReg;     //Register to scan against

    ParamScan(){
        vfatMask = 0x0;
        ohMask = 0xfff;

        useUltra = true;
        useExtTrig = false;

        dacMax=254;
        dacMin=0;
        dacStep=1;
        nevts=100;
    }
}; //End ParamScan

struct ParamTtcGen{
    bool enable;

    uint32_t L1Ainterval;
    uint32_t mode;
    uint32_t nPulses;
    uint32_t pulseDelay;
    uint32_t pulseRate;
    uint32_t type;

    ParamTtcGen(){
        enable = false;

        L1Ainterval = 250;
        pulseDelay = 40;
        pulseRate = 40079000 / L1Ainterval;
    }

    uint32_t calcRate(){
        if(L1Ainterval > 0){
            pulseRate = 40079000 / L1Ainterval;
        }
        else{
            pulseRate = 0;
        }

        return pulseRate;
    }
}; //End ParamTtcGen

/*! \struct localArgs
 *  Contains arguments required to execute the method locally
 */
struct localArgs {
    lmdb::txn & rtxn; /*!< LMDB transaction handle */
    lmdb::dbi & dbi; /*!< LMDB individual database handle */
    RPCMsg *response; /*!< RPC response message */
};


template<typename Out>
void split(const std::string &s, char delim, Out result) {
    std::stringstream ss;
    ss.str(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        *(result++) = item;
    }
}
std::vector<std::string> split(const std::string &s, char delim) {
    std::vector<std::string> elems;
    split(s, delim, std::back_inserter(elems));
    return elems;
}

std::string serialize(xhal::utils::Node n) {
  return std::to_string((uint32_t)n.real_address)+"|"+n.permission+"|"+std::to_string((uint32_t)n.mask);
}

/*! \brief This macro is used to terminate a function if an error occurs. It logs the message, write it to the `error` RPC key and returns the `error_code` value.
 *  \param response A pointer to the RPC response object.
 *  \param message The `std::string` error message.
 *  \param error_code Value which is passed to the `return` statement.
 */
#define EMIT_RPC_ERROR(response, message, error_code){ \
    LOGGER->log_message(LogManager::ERROR, message); \
    response->set_string("error", message); \
    return error_code; }

/*! \fn uint32_t getNumNonzeroBits(uint32_t value)
 *  \brief returns the number of nonzero bits in an integer
 *  \param value integer to check the number of nonzero bits
 */
uint32_t getNumNonzeroBits(uint32_t value);

/*! \fn uint32_t getMask(localArgs * la, const std::string & regName)
 *  \brief Returns the mask for a given register
 *  \param la Local arguments structure
 *  \param regName Register name
 */
uint32_t getMask(localArgs * la, const std::string & regName);

/*! \fn void writeRawAddress(uint32_t address, uint32_t value, RPCMsg *response)
 *  \brief Writes a value to a raw register address. Register mask is not applied
 *  \param address Register address
 *  \param value Value to write
 *  \param response RPC response message
 */
void writeRawAddress(uint32_t address, uint32_t value, RPCMsg *response);

/*! \fn uint32_t readRawAddress(uint32_t address, RPCMsg *response)
 *  \brief Reads a value from raw register address. Register mask is not applied
 *  \param address Register address
 *  \param response RPC response message
 */
uint32_t readRawAddress(uint32_t address, RPCMsg* response);

/*! \fn uint32_t getAddress(localArgs * la, const std::string & regName)
 *  \brief Returns an address of a given register
 *  \param la Local arguments structure
 *  \param regName Register name
 */
uint32_t getAddress(localArgs * la, const std::string & regName);

/*! \fn void writeAddress(lmdb::val & db_res, uint32_t value, RPCMsg *response)
 *  \brief Writes given value to the address. Register mask is not applied
 *  \param db_res LMDB call result
 *  \param value Value to write
 *  \param response RPC response message
 */
void writeAddress(lmdb::val & db_res, uint32_t value, RPCMsg *response);

/*! \fn uint32_t readAddress(lmdb::val & db_res, RPCMsg *response)
 *  \brief Reads given value to the address. Register mask is not applied
 *  \param db_res LMDB call result
 *  \param response RPC response message
 */
uint32_t readAddress(lmdb::val & db_res, RPCMsg *response);

/*! \fn void writeRawReg(localArgs * la, const std::string & regName, uint32_t value)
 *  \brief Writes a value to a raw register. Register mask is not applied
 *  \param la Local arguments structure
 *  \param regName Register name
 *  \param value Value to write
 */
void writeRawReg(localArgs * la, const std::string & regName, uint32_t value);

/*! \fn uint32_t uint32_t readRawReg(localArgs * la, const std::string & regName)
 *  \brief Reads a value from raw register. Register mask is not applied
 *  \param la Local arguments structure
 *  \param regName Register name
 */
uint32_t readRawReg(localArgs * la, const std::string & regName);

/*! \fn uint32_t applyMask(uint32_t data, uint32_t mask)
 *  \brief Returns the data with register mask applied
 *  \param data Register data
 *  \param mask Register mask
 */
uint32_t applyMask(uint32_t data, uint32_t mask);

/*! \fn uint32_t readReg(localArgs * la, const std::string & regName)
 *  \brief Reads a value from register. Register mask is applied. Will return 0xdeaddead if register is no accessible
 *  \param la Local arguments structure
 *  \param regName Register name
 */
uint32_t readReg(localArgs * la, const std::string & regName);

/*! \fn void writeReg(localArgs * la, const std::string & regName, uint32_t value)
 *  \brief Writes a value to a register. Register mask is applied
 *  \param la Local arguments structure
 *  \param regName Register name
 *  \param value Value to write
 */
void writeReg(localArgs * la, const std::string & regName, uint32_t value);

#endif
