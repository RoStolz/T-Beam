#include <SPI.h>
#include <LoRa.h>
#include <Wire.h>
#include "axp20x.h"
#include "SSD1306.h"
#include <TimeLib.h>
#include "FS.h"
#include "SPIFFS.h"

#define AXP192_SLAVE_ADDRESS    0x34

#define TIME_HEADER  'T'   // Header tag for serial time sync message
// Pin definetion of WIFI LoRa 32
// HelTec AutoMation 2017 support@heltec.cn
#define SCK     5    // GPIO5  -- SX127x's SCK
#define MISO    19   // GPIO19 -- SX127x's MISO
#define MOSI    27   // GPIO27 -- SX127x's MOSI
#define SS      18   // GPIO18 -- SX127x's CS
#define RST     23   // GPIO14 -- SX127x's RESET
#define DI00    26   // GPIO26 -- SX127x's IRQ(Interrupt Request)

#define BAND    868E6  //you can set band here directly,e.g. 868E6,915E6
#define PABOOST true

SSD1306 display(0x3c, 21, 22);
String rssi = "RSSI --";
String packSize = "--";
String packet ;
String snr;
int anzahlPakete = 0;

AXP20X_Class axp;

const uint8_t i2c_sda = 21;
const uint8_t i2c_scl = 22;

void setup() {
  Serial.begin(115200);

  setSyncProvider(0);
  Serial.write("Synchronisieren");
  Wire.begin(i2c_sda, i2c_scl);
  scanI2Cdevice();

  int ret = axp.begin(Wire, 0x34);

  if (ret == AXP_FAIL) {
    Serial.println("AXP Power begin failed");
    //while (1);
  }

  Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");

  Serial.println("----------------------------------------");

  axp.setPowerOutPut(AXP192_LDO2, AXP202_ON);
  axp.setPowerOutPut(AXP192_LDO3, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC2, AXP202_ON);
  axp.setPowerOutPut(AXP192_EXTEN, AXP202_ON);
  axp.setPowerOutPut(AXP192_DCDC1, AXP202_ON);
  axp.setDCDC1Voltage(3300);

  Serial.printf("DCDC1: %s\n", axp.isDCDC1Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("DCDC2: %s\n", axp.isDCDC2Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("LDO2: %s\n", axp.isLDO2Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("LDO3: %s\n", axp.isLDO3Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("DCDC3: %s\n", axp.isDCDC3Enable() ? "ENABLE" : "DISABLE");
  Serial.printf("Exten: %s\n", axp.isExtenEnable() ? "ENABLE" : "DISABLE");

  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_10);
  delay(1500);
  display.clear();
  SPI.begin(SCK, MISO, MOSI, SS);
  LoRa.setPins(SS, RST, DI00);

  if (!LoRa.begin(BAND)) {
    display.drawString(0, 0, "Starting LoRa failed!");
    display.display();
    while (1);
  }
  if (!SPIFFS.begin()) {
    Serial.println("SPIFFS Mount Failed");
    display.drawString(0, 10, "Starting FAT failed!");
    return;
  }

  display.drawString(0, 0, "LoRa Initial success!");
  display.drawString(0, 10, "Wait for incoming data...");
  display.drawString(0, 20, "FAT gemountet");
  display.display();


  delay(1000);
  //LoRa.onReceive(cbk);
  LoRa.receive();
  display.clear();
  Serial.println(axp.getDCDC1Voltage());
  Serial.println(axp.getDCDC2Voltage());
  Serial.println(axp.getDCDC3Voltage());
  //Serial.println(axp.getLDO1Voltage());
  Serial.println(axp.getLDO2Voltage());
  Serial.println(axp.getLDO3Voltage());
  Serial.println(axp.getLDO4Voltage());

  Serial.println(axp.getAcinVoltage());
  Serial.println(axp.getAcinCurrent());
  Serial.println(axp.getVbusVoltage());
  Serial.println(axp.getVbusCurrent());
  Serial.println(axp.getTemp());
  Serial.println(axp.getTSTemp());
  Serial.println(axp.getGPIO0Voltage());
  Serial.println(axp.getGPIO1Voltage());
  Serial.println(axp.getBattInpower());
  Serial.println(axp.getBattVoltage());
  Serial.println(axp.getBattChargeCurrent());
  Serial.println(axp.getBattDischargeCurrent());
  Serial.println(axp.getSysIPSOUTVoltage());
  Serial.println(axp.getSettingChargeCurrent());
}

void loop() {


  if (Serial.available()) {
    processSyncMessage();
  }
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    cbk(packetSize);
  }
  delay(100);
  display.clear();
  String buffer = String(hour()) + ":" + String(minute()) + "  " + String(second());
  display.drawString(0, 0, buffer);
  buffer = "Pakete: " + String(anzahlPakete);
  display.drawString(0, 25, buffer);
  display.display();
}

void scanI2Cdevice(void)
{
  byte err, addr;
  int nDevices = 0;
  for (addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    err = Wire.endTransmission();
    if (err == 0) {
      Serial.print("I2C device found at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.print(addr, HEX);
      Serial.println(" !");
      nDevices++;
      if (addr == AXP192_SLAVE_ADDRESS) {
        Serial.println("axp192 PMU found");
      }
    } else if (err == 4) {
      Serial.print("Unknow error at address 0x");
      if (addr < 16)
        Serial.print("0");
      Serial.println(addr, HEX);
    }
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void logo() {
  display.clear();
  // display.drawXbm(0,5,logo_width,logo_height,logo_bits);
  display.display();
}

String printDigits(int digits) {
  if (digits < 10)
    return "0" + String(digits);
  return String(digits);
}

void loraData() {
  packSize = "Größe: " +  packSize + " Byte ";
  packet = "Received packet: " + packet;
  String zeit = printDigits(hour()) + ":" + printDigits(minute()) + " ";
  display.clear();
  display.setTextAlignment(TEXT_ALIGN_LEFT);
  display.setFont(ArialMT_Plain_10);
  display.drawString(0, 0, snr);
  display.drawString(0, 10, rssi);
  display.drawString(0 , 20 , packSize);
  display.drawStringMaxWidth(0 , 30 , 128, packet);
  display.display();

  Serial.println(zeit + packSize + rssi + snr + packet);

  File file = SPIFFS.open("/test.txt", FILE_APPEND);
  if (file) {
    file.print(zeit + packSize + rssi + snr);
    file.print(packet + "\r\n");
    file.close();
  }


  anzahlPakete++;
}

void cbk(int packetSize) {
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += String(LoRa.read(), HEX);
  }
  rssi = "RSSI " + String(LoRa.packetRssi(), DEC) + " ";
  snr = "SNR " + String(LoRa.packetSnr(), DEC) + " ";
  loraData();
}


void testData(int packetSize) {
  packet = "";
  packSize = String(packetSize, DEC);
  for (int i = 0; i < packetSize; i++) {
    packet += String(random(0, 10));
  }
  rssi = "RSSI " + String(random(-120, -90), DEC) + " ";
  snr = "SNR " + String(random(9, 15), DEC) + " ";
}

void processSyncMessage() {
  unsigned long pctime;
  const unsigned long DEFAULT_TIME = 1357041600; // Jan 1 2013
  char c = Serial.read();
  if (c == TIME_HEADER) {
    pctime = Serial.parseInt();
    if ( pctime >= DEFAULT_TIME) { // check the integer is a valid time (greater than Jan 1 2013)
      setTime(pctime); // Sync Arduino clock to the time received on the serial port
      Serial.print("neue Zeit: ");
    }
  } else if (c == 'D') {
    Serial.printf("Reading file: test.txt\r\n");

    File file = SPIFFS.open("/test.txt");
    if (!file || file.isDirectory()) {
      Serial.println("- failed to open file for reading");
    } else {
      Serial.println("- read from file:");
      while (file.available()) {
        Serial.write(file.read());
      }
    }
  } else if (c == 'R') {
    Serial.printf("Deleting file: %s\r\n", "/test.txt");
    if (SPIFFS.remove("/test.txt")) {
      Serial.println("- file deleted");
      anzahlPakete = 0;
    } else {
      Serial.println("- delete failed");
    }

  } else if (c == 'h' || c == 'H') {
    Serial.println("T123456789A <- 10 Stellige Zeitnummer");
    Serial.println("D Daten Anzeigen");
    Serial.println("R Daten Reseten");
    Serial.println("E Test Daten erzeugen");
    Serial.println("F Fat formatieren. Alle Daten werden gelöscht!");
  } else if (c == 'E') {
    testData(random(0, 200));
    loraData();
  } else if (c == 'U') {
    SPIFFS.end();
  } else if (c == 'M') {
    if (!SPIFFS.begin()) {
      Serial.println("SPIFFS Mount Failed");
    } else {
      Serial.println("gemountet");
    }
  } else if (c == 'F') {
    SPIFFS.format();
  }
}






void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
  Serial.print(" ");
  Serial.print(day());
  Serial.print(" ");
  Serial.print(month());
  Serial.print(" ");
  Serial.print(year());
  Serial.println();
}
