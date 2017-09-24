#include <SPI.h>
#include <Mirf.h>
#include <nRF24L01.h>
#include <MirfHardwareSpiDriver.h>

#define BATPIN A0
#define PIRPIN 2
#define LEDPIN 3

#define TXPERIOD_DETECTED 500
#define TXPERIOD_NOT_DETECTED 10000

#define LOWBAT 3650
#define HIGHBAT 4000

#define IDENTIFIER 2

int bat;

void setup() {
  Serial.begin(9600);
  pinMode(LEDPIN, OUTPUT);
  rfInit();
}

void loop() {
  batteryMonitor();
  
  int txPeriod;
  static long lastTX;
  byte NRFTX[1] = {0};
  
  // lecture PIR
  if(digitalRead(PIRPIN) == HIGH)
  {
    txPeriod = TXPERIOD_DETECTED;
    NRFTX[0] |= (1<<7);
  }
  else
  {
    txPeriod = TXPERIOD_NOT_DETECTED;
    NRFTX[0] &= ~(1<<7);
  }
  
  // TX
  if(millis()-lastTX > txPeriod && bat > LOWBAT)
  {
    NRFTX[0] |= (constrain(IDENTIFIER,0x00,0x07)<<4); // ID on bits 6-4
    NRFTX[0] |= constrain(map(bat,LOWBAT,HIGHBAT,0x00,0x0f),0x00,0x0f); // Battery level on 4 bits LSB
    
    lastTX = millis();
    Mirf.send(NRFTX);
    while(Mirf.isSending());
  }
}

void rfInit() {
  Mirf.spi = &MirfHardwareSpi;
  Mirf.init();
  Mirf.setRADDR((byte *)"msu01");
  Mirf.setTADDR((byte *)"msr01");
  Mirf.payload = 1;
  Mirf.channel = 125;
  Mirf.config();
}

void batteryMonitor() {
  bat = map(analogRead(BATPIN),0,275,0,3830);
  if(bat<LOWBAT && (millis()/500)%2) digitalWrite(LEDPIN,LOW);
  else digitalWrite(LEDPIN,HIGH);
}
