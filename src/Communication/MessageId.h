#ifndef __MESSAGE_ID_HPP
#define	__MESSAGE_ID_HPP

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif


/// @brief Message id type for access as 2 bytes
struct msgid_8_s
{
    uint8_t msgid_l;
    uint8_t msgid_h;
};

typedef uint16_t msgid_t;

/// @brief Message id type for access as 2 bytes or 1 word
union msgid_u
{
    struct msgid_8_s msgid_8 ;
    msgid_t msgid_16;
};


// msgids definitions
    
/** Ping packet ID */
const msgid_t MSGID_PING                          = 0x0000;
    
/** Reply to ping packet */    
const msgid_t MSGID_PING_REPLY                    = 0x0001;
    
/** Acknowledge OK packet */    
const msgid_t MSGID_ACK_OK                        = 0x0002;
    
/** Acknowledge NOK packet */
const msgid_t MSGID_ACK_NOK                       = 0x0003;

/** Broadcast sleep to put all devices into low power state 
 * The message prototype  is [MSGID_SLEEP_ALL] <uint32_t>[DURATION_US]  */
const msgid_t MSGID_SLEEP                         = 0x0004;


/**
 * @brief Request for hard reset of the device - return to factory settings 
 */
const msgid_t MSGID_RESET_HARD                    = 0x00fe;

/**
 * @brief Request for soft reset of the device 
 */
const msgid_t MSGID_RESET_SOFT                    = 0x00ff;
    
    
/**
 * @brief Obsolete - use MSGID_READ instead
 * 
 */
const msgid_t MSGID_FETCH_MEASUREMENT             = 0x0100;
    
/** Synchronisation message - sync one/all devices */
const msgid_t MSGID_SYNC                          = 0x0101;

    
    
/** Set register to a value 
 * The message prototype is <MSGID_SET> <REG_ID> <BYTE_1> ... <BYTE_N>
 */    
const msgid_t MSGID_WRITE                         = 0x0200;
    
/** Read  up to <LEN> bytes from device register, starting at <REG_ID>
 * The request prototype is <MSGID_READ> <REG_ID> <LEN>
 */            
const msgid_t MSGID_READ                          = 0x0201;
const msgid_t MSGID_READ_VALUE                    = 0x0202;
    
/** Pressure value w/o temperature*/    
const msgid_t MSGID_PRESSURE                      = 0x0400;
    
    
/** Strain value w/o temperature */    
const msgid_t MSGID_STRAIN_24BIT                  = 0x1100;
    

/** Cutter 1000P/R, 63mm wheel */
const msgid_t MSGID_PULSES                        = 0x2A01;

    
/** 2 distance values, 0-22000um, no temp */
const msgid_t MSGID_DISTANCE_22MM                 = 0x4000;
/** 2 distance values, 0-225000um, no temp */
const msgid_t MSGID_DISTANCE_225MM                = 0x4100;
    

/** 2 angle values, X, Y (-90°, 90°)*/
const msgid_t MSGID_ANGLE_DEG_XY                  = 0x3000;


#ifdef	__cplusplus
}
#endif

#endif	/* __MESSAGE_ID_HPP */