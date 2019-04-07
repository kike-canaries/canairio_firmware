#ifndef CanAirIoApi_hpp
#define CanAirIoApi_hpp

#include <ArduinoJson.h>

class CanAirIoApi
{
  private:
  char* _sensorId;
  char* _endpoint;
  char* _host;
  uint16_t _port;

  char* _username;
  char* _password;
  char* _cert;
  bool _isAuthorised = false;
  bool _isSecure = false;
  int _latestResponse; //storing the latest response

  public:

  bool _debug;

  CanAirIoApi(bool debug = false);

  ~CanAirIoApi();

  void configure(const char sensorId[], const char endpoint[], const char host[], const uint16_t port = 80);

  void authorize(const char username[],const char password[]);

  bool write(uint16_t pm1, uint16_t pm25, uint16_t pm10, float hum, float tmp, float lat, float lon, float alt, float spd, int stime);

  int getResponse();

  bool isSecure();

};

#endif

