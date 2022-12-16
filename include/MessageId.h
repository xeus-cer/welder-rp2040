#ifndef __MESSAGE_ID_HPP
#define	__MESSAGE_ID_HPP

#include <stdint.h>

#ifdef	__cplusplus
extern "C" {
#endif


//msgid structure:    
struct msgid_8_s
{
    uint8_t msgid_l;
    uint8_t msgid_h;
};

typedef uint16_t msgid_t;

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
    
    
/**
 * Request to send measurements
 */
const msgid_t MSGID_FETCH_MEASUREMENT             = 0x0100;
    
/** Synchronisaton message */
const msgid_t MSGID_SYNC                          = 0x0101;
    
    
/** Set register to a value 
 * The message prototype is <MSGID_SET> <REG_ID> <LEN> <BYTE_1> ... <BYTE_N>
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