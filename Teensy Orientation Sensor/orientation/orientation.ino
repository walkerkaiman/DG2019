#include <NXPMotionSense.h>
#include <Wire.h>
#include <EEPROM.h>

NXPMotionSense imu;
NXPSensorFusion filter;

void setup() {
  Serial.begin(9600);
  imu.begin();
  filter.begin(100);
}

void loop() {
  float ax, ay, az;
  float gx, gy, gz;
  float mx, my, mz;
  
  if (imu.available()) {
    imu.readMotionSensor(ax, ay, az, gx, gy, gz, mx, my, mz);
    filter.update(gx, gy, gz, ax, ay, az, mx, my, mz);
    
    if(Serial.available() > 0 && Serial.read() == 'r') {
      Serial.println(filter.getYaw());
    }
  }
}
