#ifndef CanAirIoApi_hpp
#define CanAirIoApi_hpp

#include <ArduinoJson.h>

class CanAirIoApi
{
  private:
  
  char* _username;
  char* _password;
  char* _cert;
  bool _isAuthorised = false;
  bool _isSecure = false;
  int _latestResponse; //storing the latest response
  uint16_t _port;

  public:
  char* id;
  char* devId;
  char* url;
  char* ip;
  bool dev;

  CanAirIoApi(bool debug = false);

  ~CanAirIoApi();
 
  void configure(const char nameId[], const char deviceId[], const char target[] = "points/save/", const char host[] = "api.canair.io", const uint16_t port = 80);

  void authorize(const char username[],const char password[]);

  bool write(uint16_t pm1 = 0, uint16_t pm25 = 0, uint16_t pm10 = 0, float hum = 0.0f, float tmp = 0.0f, float lat = 0.0f, float lon = 0.0f, float alt = 0.0f, float spd = 0.0f, int stime = 5, int tstp = 0);

  int getResponse();

  bool isSecure();

};

#endif

