#include <Wire.h>
#include "axp20x.h"

#define AXP192_SLAVE_ADDRESS    0x34


AXP20X_Class axp;

const uint8_t i2c_sda = 21;
const uint8_t i2c_scl = 22;

void setup() {
  Serial.begin(115200);
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


  
}

void loop() {
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


  delay(1000);
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
