#include "FuntechDynamixel.h"

FuntechDynamixel dyna = FuntechDynamixel();

#define ALL 0xFE // broadcast address

#define MOTORS 3 // number of servos
char addresses[] = {0x04, 0x02, 0x05}; // addresses of the dynamixels

#define SLOTS 10 // number of slots for saving positions
unsigned int positions[SLOTS][MOTORS];


void setup() {
  SERIAL.begin(FTD_BAUDRATE);
  Serial.begin(FTD_BAUDRATE);

  Serial.println("Welcome to this FunctechDynamixel demonstration!\n");
  Serial.println("=== Instructions ===");
  Serial.println("E: enable servos");
  Serial.println("D: disable servos");
  Serial.println("--------------------");
  Serial.println("S<slot>: save servo positions (0 <= slot <= 9)");
  Serial.println("M<slot>: move servo to position (0 <= slot <= 9)");
  Serial.println("--------------------");
  Serial.println("W<add>,<reg>,<val>: write <val> on <reg> at <add>");
  Serial.println("R<add>,<reg>: read value on <reg> at <add>");
  Serial.println("====================\n");
}

void loop() {
  static int add = 0x02, reg, val;
  
  if(Serial.available()) {
    char instruction = Serial.read();
    char add, reg, slot;
    unsigned int val;

    switch(instruction) {
      case 'E':
        dyna.enable(ALL);
        Serial.println("Enabled");
        break;
      case 'D':
        dyna.disable(ALL);
        Serial.println("Disabled");
        break;

      case 'S':
        slot = Serial.parseInt();
        
        if(slot < 0 || 9 < slot) {
          Serial.println("Slot error. 0 to 9 available.");
          break;
        }
        
        for(int i = 0; i < MOTORS; i++) {
          while(dyna.readRegister(addresses[i], FDR_PRESENT_POSITION, &val) != FDE_OK) {
            delay(1);
          }
          Serial.println(val);
          positions[slot][i] = val;
        }
        
        Serial.print("Saved positions in slot ");
        Serial.println((int)slot);
        break;

      case 'M':
        slot = Serial.parseInt();
        
        for(int i = 0; i < MOTORS; i++) {
          while(dyna.writeRegister(addresses[i], FDR_GOAL_POSITION, positions[slot][i]) != FDE_OK) {
            delay(1);
          }
          Serial.print("Set motor "); Serial.print((int)addresses[i]); Serial.print(" to "); Serial.println(positions[slot][i]);
        }

        Serial.print("Play position from slot ");
        Serial.println((int)slot);
        break;

      case 'W':
        add = Serial.parseInt();
        reg = Serial.parseInt();
        val = Serial.parseInt();
        Serial.print("Write "); Serial.print(val); Serial.print(" on "); Serial.print((int)reg); Serial.print(" at "); Serial.println((int)add); 
        Serial.println(dyna.writeRegister(add, reg, val));
        break;
        
      case 'R':
        add = Serial.parseInt();
        reg = Serial.parseInt();
        Serial.print("Read "); Serial.print((int)reg); Serial.print(" from "); Serial.println((int)add); 
        Serial.println(dyna.readRegister(add, reg, &val));
        Serial.print("Value: "); Serial.println(val);
        break;
    }
    
    /*add = Serial.parseInt();
    reg = Serial.parseInt();
    val = Serial.parseInt();
    dyna.writeRegister(add, reg, val);
    while(Serial.available()) Serial.read();*/
  }

  if(SERIAL.available()) {
    while(SERIAL.available()) {
      Serial.print((int)SERIAL.read(), HEX);
      Serial.print(", ");
    }
    Serial.print("\n");
  }

  //dyna.ping(0xFE);

  delay(500);
}
