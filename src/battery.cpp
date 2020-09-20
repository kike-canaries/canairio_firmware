/******************************************************************************
*   B A T T E R Y   C H A R G E   S T A T U S   M E T H O D S
******************************************************************************/
// IP5306_2 = pin 27 ESP32, pin 2 IP5306
// IP5306_3 = pin 26 ESP32, pin 3 IP5306

// Battery level
unsigned int chargeLevel = 0;

#ifdef TTGO_TQ
    const int IP5306_2 = 27;  // PIN2 IP5306
    const int IP5306_3 = 14;  // PIN3 IP5306
    unsigned int Rdelay = 0;
#endif



void batteryloop() {
#ifdef TTGO_TQ
  Rdelay = 0;
  while (digitalRead(IP5306_2) == HIGH)
  {
    delayMicroseconds(100); // Sincronization in 1
  }
  delayMicroseconds(50); // Probably double shoot in 0
  while (digitalRead(IP5306_2) == HIGH)
  {
    delayMicroseconds(100); // Sincronization in 1
  }
  while (digitalRead(IP5306_2) == LOW && Rdelay < 56)
  {
    delayMicroseconds(100); // Sincronization in 0
    Rdelay = Rdelay + 1;
  }
  if (Rdelay > 52)
  {
    chargeLevel = 0; // 0%
    return;
  }
  delayMicroseconds(1600);
  if (digitalRead(IP5306_2) == HIGH)
  {
    delayMicroseconds(100);
    if (digitalRead(IP5306_2) == HIGH)
    {
      chargeLevel = 100; // 100%
      return;
    }
  }
  if (digitalRead(IP5306_3) == LOW)
  {
    delayMicroseconds(100);
    if (digitalRead(IP5306_3) == LOW)
    {
      chargeLevel = 25; // 25%
      return;
    }
  }
  delayMicroseconds(1100);
  if (digitalRead(IP5306_3) == HIGH)
  {
    delayMicroseconds(100);
    if (digitalRead(IP5306_3) == HIGH)
    {
      chargeLevel = 75; // 75%
      return;
    }
  }
  if (digitalRead(IP5306_3) == LOW)
  {
    chargeLevel = 50; // 50%
    return;
  }
#endif
}