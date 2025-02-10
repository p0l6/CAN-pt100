#include <ESP32-TWAI-CAN.hpp>
#include <Adafruit_MAX31865.h>


#define CAN_TX 9
#define CAN_RX 10


#define MAX31865_CS 7    // Chip Select (CS)
#define MAX31865_MOSI 5  // Data Input (MOSI)
#define MAX31865_MISO 6  // Data Output (MISO)
#define MAX31865_SCK 4   // Clock (SCK)


Adafruit_MAX31865 thermo = Adafruit_MAX31865(MAX31865_CS, MAX31865_MOSI, MAX31865_MISO, MAX31865_SCK);
#define RREF 430.0
#define RNOMINAL 100.0

CanFrame txFrame;  

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Inicjalizacja");


  ESP32Can.setPins(CAN_TX, CAN_RX);
  if (ESP32Can.begin(ESP32Can.convertSpeed(500))) {
    Serial.println("Uruchomione");
  } else {
    Serial.println("Blad");
  }


  thermo.begin(MAX31865_2WIRE);
}

void loop() {
  uint16_t rtd = thermo.readRTD();

  Serial.print("RTD value: ");
  Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  Serial.print("Ratio = ");
  Serial.println(ratio, 8);
  Serial.print("Resistance = ");
  Serial.println(RREF * ratio, 8);
  Serial.print("Temperature = ");
  Serial.println(thermo.temperature(RNOMINAL, RREF));

  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold");
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold");
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias");
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage");
    }
    thermo.clearFault();
  }
  Serial.println();

  CAN(rtd);
  delay(1000);
}

void CAN(float temp) {
  Serial.println("Wysylanie");

  txFrame.identifier = 0x12;
  txFrame.extd = 0;
  txFrame.data_length_code = 4;  

  memcpy(txFrame.data, &temp, sizeof(float));

  ESP32Can.writeFrame(txFrame);
  Serial.println("Wyslane.");
}
