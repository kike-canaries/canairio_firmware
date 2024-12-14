#include <WiFi.h>
#include <MQTT.h>
#include <Sensors.hpp>
#include "Batterylib.hpp"
#include "ConfigApp.hpp"
#include "mqtt_config.h"
#include "wifi.hpp"

#define HPREFIX "homeassistant/"
#define HCOMP "sensor/"
#define TOPIC_CONF "config"
#define TOPIC_STATE "state"
#define TOPIC_STATUS "status"
#define TOPIC_ATTRIBUTES "attributes"

void hassLoop ();
void hassInit ();
bool hassIsConnected ();
