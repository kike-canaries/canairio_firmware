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
 
  void configure(const char nameId[], const char deviceId[], const char target[] = "points/save/", const char host[] = "canairio.herokuapp.com", const uint16_t port = 80);

  void authorize(const char username[],const char password[]);

  bool write(uint16_t pm1, uint16_t pm25, uint16_t pm10, uint32_t gas, float aqs, float iaq, float hum, float tmp, float prs, float lat, float lon, float alt, float spd, int stime = 5);

  int getResponse();

  bool isSecure();

};

#endif

