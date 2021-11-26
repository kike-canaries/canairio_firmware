#define HASS_HOST "192.168.178.88"
#define HASS_TOPIC "measurement"
#define HASS_PORT 1883

#define HPREFIX "homeassistant/"
#define HCOMP "sensor/"
#define TOPIC_CONF "config"
#define TOPIC_STATE "state"
#define TOPIC_ATTRIBUTES "attributes"

void hassLoop ();
void hassInit ();