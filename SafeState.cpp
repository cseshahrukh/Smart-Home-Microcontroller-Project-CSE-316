/**
   Arduino Electronic Safe

   Copyright (C) 2020, Uri Shaked.
   Released under the MIT License.
*/

#include <Arduino.h>
#include <EEPROM.h>
#include "SafeState.h"

/* Safe state */
#define EEPROM_ADDR_LOCKED   0
#define EEPROM_ADDR_CODE_LEN 1
#define EEPROM_ADDR_CODE     2
#define EEPROM_EMPTY         0xff

#define SAFE_STATE_OPEN (char)0
#define SAFE_STATE_LOCKED (char)1



SafeState::SafeState() {
  this->_locked = true;
}

void SafeState::lock() {
  this->setLock(true);
}

bool SafeState::locked() {
  return this->_locked;
}

bool SafeState::hasCode() {
  return code.length() != 0;
}

void SafeState::setCode(String newCode) {
  this->code = newCode;
}

String SafeState::getCode() {
  return code;
}

bool SafeState::unlock(String code) {
  boolean flag = true;
  if (this->code.length() != code.length())
    return false;
  else {
    for (int i = 0; i < this->code.length(); i++) {
      if (this->code[i] != code[i]) {
        flag = false;
        break;
      }
    }
  }
  this->setLock(!flag);
  return flag;
}

void SafeState::setLock(bool locked) {
  this->_locked = locked;
}
