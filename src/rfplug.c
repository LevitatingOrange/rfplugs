#include <wiringPi.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// see https://github.com/JoakimWesslen/Tx433_Proove/blob/master/tx433_proove.cpp

const int RF_TX_PORT = 0;

const int REPEAT_COUNT = 5;

// pulse timings
const int PULSE_HIGH = 250;
const int PULSE_ONE_LOW = 250;
const int PULSE_ZERO_LOW = 1250;
const int PULSE_SYNC_LOW = 2500;
const int PULSE_PAUSE_LOW = 10000;

// bitfield offsets
//  Proove packet structure (32 bits):
//  HHHH HHHH HHHH HHHH HHHH HHHH HHGO CCEE
const int HOUSE_CODE_OFFSET = 6;
const long HOUSE_CODE_MASK = 0xFFFFFFC0;
const int GROUP_SWITCH_OFFSET = 5;
const int GROUP_SWITCH_MASK = 0x20;
const int DEVICE_SWITCH_OFFSET = 4;
const int DEVICE_SWITCH_MASK = 0x10;
const int CHANNEL_OFFSET = 2;
const int CHANNEL_MASK = 0xC;
const int UNIT_OFFSET = 0;
const int UNIT_MASK = 0x3;

void sendPulse(bool one) {
  digitalWrite(RF_TX_PORT, HIGH);
  delayMicroseconds(PULSE_HIGH);
  digitalWrite(RF_TX_PORT, LOW);
  delayMicroseconds(one ? PULSE_ONE_LOW : PULSE_ZERO_LOW);
}

void sendBit(char bit) {
  // 0 is encoded as 01 and 1 as 10
  if (bit) {
    sendPulse(true);
    sendPulse(false);
  } else {
    sendPulse(false);
    sendPulse(true);
  }
}

void sendSync() {
  digitalWrite(RF_TX_PORT, HIGH);
  delayMicroseconds(PULSE_HIGH);
  digitalWrite(RF_TX_PORT, LOW);
  delayMicroseconds(PULSE_SYNC_LOW);
}

void sendPause() {
  digitalWrite(RF_TX_PORT, HIGH);
  delayMicroseconds(PULSE_HIGH);
  digitalWrite(RF_TX_PORT, LOW);
  delayMicroseconds(PULSE_PAUSE_LOW);
}

void sendPacket(long packet) {
  sendSync();
  for (int i = 31; i >= 0; i--) {
    sendBit((packet >> i) & 1);
  }
  sendPause();
}

void sendMessage(long houseCode, bool groupSwitch, bool deviceSwitch, char channel, char unit) {
  long packet = 0;
  packet |= ((houseCode << HOUSE_CODE_OFFSET) & HOUSE_CODE_MASK);
  packet |= ((!groupSwitch << GROUP_SWITCH_OFFSET) & GROUP_SWITCH_MASK);
  packet |= ((!deviceSwitch << DEVICE_SWITCH_OFFSET) & DEVICE_SWITCH_MASK);
  packet |= ((channel << CHANNEL_OFFSET) & CHANNEL_MASK);
  packet |= ((unit << UNIT_OFFSET) & UNIT_MASK);

  for (int i = 0; i < REPEAT_COUNT; i++) {
    sendPacket(packet);
  }
}



int main(int argc, char *argv[])
{
  if (argc != 5) {
    printf("Usage: %s <group id> <device id> <activate whole group (1/0)> <on/off (1/0)>\n", argv[0]);
    exit(-1);
  }
  int gid = strtol(argv[1], NULL, 10);
  int did = strtol(argv[2], NULL, 10);
  int gswitch = strtol(argv[3], NULL, 10);
  int dswitch = strtol(argv[4], NULL, 10);

  if ((dswitch != 0 && dswitch != 1) || (gswitch != 0 && gswitch != 1)) {
    printf("Wrong input\n");
    exit(-1);
  }

  wiringPiSetup();
  pinMode(RF_TX_PORT, OUTPUT);
  sendMessage(gid, gswitch, dswitch, (did >> 2) & 0b11, did & 0b11);
  return 0;
}