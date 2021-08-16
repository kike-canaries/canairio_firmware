#include <Arduino.h>
#include <s8.h>


#define S8_RX_PIN 14         // Rx pin which the S8 Tx pin is attached to
#define S8_TX_PIN 12         // Tx pin which the S8 Rx pin is attached to


SoftwareSerial S8_serial(S8_RX_PIN, S8_TX_PIN);

S8 *sensor_S8;
S8_sensor sensor;

void setup() {

  // Init serial port for debug
  Serial.begin(74880); 
  Serial.println("");
  Serial.println("Init");

  // Initialize S8 sensor
  S8_serial.begin(S8_BAUDRATE);
  sensor_S8 = new S8(S8_serial);

  // Check if S8 is available
  sensor_S8->get_firmware_version(sensor.firmver);
  int len = strlen(sensor.firmver);
  if (len == 0) {
      Serial.println("S8 not found!");
      while (1) { delay(1); };
  }

  // Show S8 sensor info
  Serial.println(">>> SenseAir S8 NDIR CO2 sensor <<<");
  Serial.printf("Software version: %s\n", sensor.firmver);
  Serial.printf("Sensor type: 0x%08x\n", sensor_S8->get_sensor_type_ID());
  Serial.printf("Sensor ID:  %08x\n", sensor_S8->get_sensor_ID());
  Serial.printf("Memory map version: 0x%04x\n", sensor_S8->get_memory_map_version());
  Serial.printf("ABC period (0 = disabled): %d hours\n", sensor_S8->get_ABC_period());

  Serial.println("Disable ABC period");
  sensor_S8->set_ABC_period(0);
  delay(1000);
  Serial.printf("ABC period (0 = disabled): %d hours\n", sensor_S8->get_ABC_period());

  Serial.println("ABC period set to 4900 hours");
  sensor_S8->set_ABC_period(4900);
  delay(1000);
  Serial.printf("ABC period (0 = disabled): %d hours\n", sensor_S8->get_ABC_period());

  while (1) { delay(1);}

  Serial.println("ABC period set to 180 hours");
  sensor_S8->set_ABC_period(180);
  delay(1000);
  Serial.printf("ABC period (0 = disabled): %d hours\n", sensor_S8->get_ABC_period());


  sensor_S8->get_meter_status();
  sensor_S8->get_alarm_status();
  sensor_S8->get_output_status();

  sensor_S8->get_acknowledgement();
}


void loop() {

  // Get CO2 measure
  Serial.printf("CO2 value = %d ppm\n", sensor_S8->get_co2());
  Serial.printf("PWM output = %0.0f ppm\n", (sensor_S8->get_PWM_output() / 16383.0) * 2000.0);

  while (1) { delay(1);}

  // Wait 5 second for next measure
  delay(5000);
}
