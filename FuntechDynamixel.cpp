#include "FuntechDynamixel.h"

/* PUBLIC */

FuntechDynamixel::FuntechDynamixel(void) {
}

void FuntechDynamixel::setAddress(unsigned char address) {
  this->writeRegister(0xFE, FDR_ID, address);
}

void FuntechDynamixel::ping(unsigned char address) {
  this->sendInstruction(address, FDI_PING, 0, (void*)(0));
}

void FuntechDynamixel::enable(unsigned char address) {
  this->writeRegister(address, FDR_TORQUE_ENABLE, 1);
}

void FuntechDynamixel::disable(unsigned char address) {
  this->writeRegister(address, FDR_TORQUE_ENABLE, 0);
}

void FuntechDynamixel::setPosition(unsigned char address, int pos) {
  for(int i = 0; i < 3; i++) {
    if(this->writeRegister(address, FDR_GOAL_POSITION, pos) == FDE_OK) return FDE_OK;
  }

  return FDE_UNDEFINED;
}

/*FuntechDynamixel_Error FuntechDynamixel::getPosition(char address, int *pos) {
  return this->readRegister(address, FDR_GOAL_POSITION, pos) == FDE_OK) return FDE_OK;
}*/

FuntechDynamixel_Error FuntechDynamixel::writeRegister(unsigned char address, FuntechDynamixel_Register reg, int value) {
  unsigned char data[3];
  unsigned char rx_buffer[15];
  
  data[0] = reg;
  
  switch(reg) {
    // One byte registers
    case FDR_ID:
    case FDR_BAUD_RATE:
    case FDR_RETURN_DELAY:
    case FDR_MAX_TEMPERATURE:
    case FDR_LOW_VOLTAGE_LIMIT:
    case FDR_HIGH_VOLTAGE_LIMIT:
    case FDR_STATUS_RETURN_LEVEL:
    case FDR_ALARM_LED:
    case FDR_ALARM_SHUTDOWN:
    case FDR_TORQUE_ENABLE:
    case FDR_LED:
    case FDR_CC_COMPLIANCE_MARGIN:
    case FDR_CCW_COMPLIANCE_MARGIN:
    case FDR_CW_COMPLIANCE_SLOPE:
    case FDR_CCW_COMPLIANCE_SLOPE:
    case FDR_PRESENT_VOLTAGE:
    case FDR_PRESENT_TEMPERATURE:
    case FDR_REGISTERED_INSTRUCTION:
    case FDR_LOCK:
      data[1] = (char)value;
      this->sendInstruction(address, FDI_WRITE_DATA, 2, data);
      break;

    // Two bytes registers
    case FDR_MODEL_NUMBER:
    case FDR_FIRMWARE_VERSION:
    case FDR_CW_ANGLE_LIMIT:
    case FDR_CCW_ANGLE_LIMIT:
    case FDR_MAX_TORQUE:
    case FDR_GOAL_POSITION:
    case FDR_MOVING_SPEED:
    case FDR_TORQUE_LIMIT:
    case FDR_PUNCH:
      data[1] = value & 0xff;
      data[2] = value >> 8;
      this->sendInstruction(address, FDI_WRITE_DATA, 3, data);
      break;

    default: return FDE_BAD_REGISTER;
  }

  // Waiting for response
  delay(1);

  // Get response
  size_t n = SERIAL.available();

  if(n == 0) {
    return FDE_NO_REPLY;
  }
  
  SERIAL.readBytes(rx_buffer, n);

  // Verify address
  if(address != rx_buffer[2] && address != 0xFE) {
    return FDE_BAD_REPLIER;
  }
  
  // Verify checksum
  if(this->checksum(n-3, rx_buffer + 2) != rx_buffer[n - 1]) {
    return FDE_BAD_CHECKSUM;
  }

  return FDE_OK;
}

FuntechDynamixel_Error FuntechDynamixel::readRegister(unsigned char address, FuntechDynamixel_Register reg, int *val) {
  unsigned char data[2];
  unsigned char rx_buffer[15];
  size_t len = 0;

  data[0] = reg;
  
  switch(reg) {
    // One byte registers
    case FDR_ID:
    case FDR_BAUD_RATE:
    case FDR_RETURN_DELAY:
    case FDR_MAX_TEMPERATURE:
    case FDR_LOW_VOLTAGE_LIMIT:
    case FDR_HIGH_VOLTAGE_LIMIT:
    case FDR_STATUS_RETURN_LEVEL:
    case FDR_ALARM_LED:
    case FDR_ALARM_SHUTDOWN:
    case FDR_TORQUE_ENABLE:
    case FDR_LED:
    case FDR_CC_COMPLIANCE_MARGIN:
    case FDR_CCW_COMPLIANCE_MARGIN:
    case FDR_CW_COMPLIANCE_SLOPE:
    case FDR_CCW_COMPLIANCE_SLOPE:
    case FDR_PRESENT_VOLTAGE:
    case FDR_PRESENT_TEMPERATURE:
    case FDR_REGISTERED_INSTRUCTION:
    case FDR_MOVING:
    case FDR_LOCK:
      len = 0x01;
      break;

    // Two bytes registers
    case FDR_MODEL_NUMBER:
    case FDR_FIRMWARE_VERSION:
    case FDR_CW_ANGLE_LIMIT:
    case FDR_CCW_ANGLE_LIMIT:
    case FDR_MAX_TORQUE:
    case FDR_DOWN_CALIBRATION:
    case FDR_UP_CALIBRATION:
    case FDR_GOAL_POSITION:
    case FDR_MOVING_SPEED:
    case FDR_TORQUE_LIMIT:
    case FDR_PRESENT_POSITION:
    case FDR_PRESENT_SPEED:
    case FDR_PRESENT_LOAD:
    case FDR_PUNCH:
      len = 0x02;
      break;

    default: return FDE_BAD_REGISTER;
  }

  data[1] = len;
  this->sendInstruction(address, FDI_READ_DATA, 2, data);

  // Waiting for response
  delay(1);

  // Get response
  size_t n = SERIAL.available();

  if(n == 0) {
    return FDE_NO_REPLY;
  }
  
  SERIAL.readBytes(rx_buffer, n);

  // Verify address
  if(address != rx_buffer[2] && address != 0xFE) {
    return FDE_BAD_REPLIER;
  }
  
  // Verify checksum
  if(this->checksum(n-3, rx_buffer + 2) != rx_buffer[n - 1]) {
    return FDE_BAD_CHECKSUM;
  }

  // Return the value
  switch(n) {
    case 7:
      *val = rx_buffer[5];
      break;
    case 8:
      *val = rx_buffer[5] + ((rx_buffer[6] & 0x03) << 8);
      break;
  }
  
  return FDE_OK;
}


/* PRIVATE */

void FuntechDynamixel::sendPacket(size_t size, unsigned char *data) {
  /* A packet is a command, length, args, without checksum */
  char buffer[10];
  char checksum = this->checksum(size, data);

  // Cleaning buffer
  SERIAL.readBytes(buffer, SERIAL.available());
  
  SERIAL.write(0xFF);
  SERIAL.write(0xFF);
  SERIAL.write(data, size);
  SERIAL.write(checksum);
  SERIAL.flush();
  SERIAL.readBytes(buffer, SERIAL.available());
}

void FuntechDynamixel::sendInstruction(unsigned char address, FuntechDynamixel_Instruction instruction, size_t size, unsigned char *data) {
  char packet[10];
 
  packet[0] = address;
  packet[1] = size + 2;
  packet[2] = instruction;

  memcpy(&(packet[3]), data, size);

  this->sendPacket(size + 3, packet);
}

unsigned char FuntechDynamixel::checksum(size_t size, unsigned char *data) {
  unsigned char c = 0;
  for (int i = 0; i < size; i++) {
    c += data[i];
  }
  return ~c;
}
