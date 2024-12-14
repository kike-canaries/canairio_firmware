#include <WiFi.h>
#include <MQTT.h>
#include <Sensors.hpp>
#include "Batterylib.hpp"
#include "mqtt_config.h"
#include "ConfigApp.hpp"

#define ANAIRE_HOST "mqtt.anaire.org"
#define ANAIRE_TOPIC "measurement"
#define ANAIRE_PORT 80

void anaireLoop ();
void anaireInit ();
bool anaireIsConnected();