#ifndef _DYNAMIXEL_H
#define _DYNAMIXEL_H

#include "Arduino.h"

#define FTD_BAUDRATE 1000000
#define SERIAL Serial1

typedef enum {
  FDI_PING = 0x01,
  FDI_READ_DATA = 0x02,
  FDI_WRITE_DATA = 0x03,
  FDI_REG_WRITE = 0x04,
  FDI_ACTION = 0x05,
  FDI_RESET = 0x06,
  // FDI_SYNC_WRITE = 0x83 // Not implemented
} FuntechDynamixel_Instruction;

typedef enum {
  FDR_MODEL_NUMBER = 0x00,
  FDR_FIRMWARE_VERSION = 0x02,
  FDR_ID = 0x03,
  FDR_BAUD_RATE = 0x04,
  FDR_RETURN_DELAY = 0x05,
  FDR_CW_ANGLE_LIMIT = 0x06,
  FDR_CCW_ANGLE_LIMIT = 0x08,
  FDR_MAX_TEMPERATURE = 0x0b,
  FDR_LOW_VOLTAGE_LIMIT = 0x0c,
  FDR_HIGH_VOLTAGE_LIMIT = 0x0d,
  FDR_MAX_TORQUE = 0x0e,
  FDR_STATUS_RETURN_LEVEL = 0x10,
  FDR_ALARM_LED = 0x11,
  FDR_ALARM_SHUTDOWN = 0x12,
  FDR_DOWN_CALIBRATION = 0x14,
  FDR_UP_CALIBRATION = 0x16,
  FDR_TORQUE_ENABLE = 0x18,
  FDR_LED = 0x19,
  FDR_CC_COMPLIANCE_MARGIN = 0x1a,
  FDR_CCW_COMPLIANCE_MARGIN = 0x1b,
  FDR_CW_COMPLIANCE_SLOPE = 0x1c,
  FDR_CCW_COMPLIANCE_SLOPE = 0x1d,
  FDR_GOAL_POSITION = 0x1e,
  FDR_MOVING_SPEED = 0x20,
  FDR_TORQUE_LIMIT = 0x22,
  FDR_PRESENT_POSITION = 0x24,
  FDR_PRESENT_SPEED = 0x26,
  FDR_PRESENT_LOAD = 0x28,
  FDR_PRESENT_VOLTAGE = 0x2a,
  FDR_PRESENT_TEMPERATURE = 0x2b,
  FDR_REGISTERED_INSTRUCTION = 0x2c,
  FDR_MOVING = 0x2e,
  FDR_LOCK = 0x2f,
  FDR_PUNCH = 0x30,
} FuntechDynamixel_Register;

typedef enum {
  FDE_OK = 0,
  FDE_NO_REPLY,
  FDE_BAD_REGISTER,
  FDE_BAD_CHECKSUM,
  FDE_BAD_REPLIER,
  FDE_UNDEFINED,
} FuntechDynamixel_Error;


union {
  struct {
    unsigned char instruction: 1;
    unsigned char overload: 1;
    unsigned char checksum: 1;
    unsigned char range: 1;
    unsigned char overheating: 1;
    unsigned char angle_limit: 1;
    unsigned char input_voltage: 1;
  } bits;
  unsigned int flags;
} FuntechDynamixel_Errorcode; 


class FuntechDynamixel {
  public:
    FuntechDynamixel(void);
    void setAddress(unsigned char address);
    void ping(unsigned char address);
    void enable(unsigned char address);
    void disable(unsigned char address);
    void setPosition(unsigned char address, int pos);
    // Public but use this only if you know what you do
    FuntechDynamixel_Error writeRegister(unsigned char address, FuntechDynamixel_Register reg, int value);
    FuntechDynamixel_Error readRegister(unsigned char address, FuntechDynamixel_Register reg, int *val);
  private:
    void sendInstruction(unsigned char address, FuntechDynamixel_Instruction instruction, size_t size, unsigned char *data);
    void sendPacket(size_t size, unsigned char *data);
    unsigned char checksum(size_t size, unsigned char *data);
};

#endif
