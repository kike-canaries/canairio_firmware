#ifndef InfluxArduino_hpp
#define InfluxArduino_hpp

class InfluxArduino
{
public:
  InfluxArduino();
  ~InfluxArduino();

  //specify the database, IP address and port (optional) for the database
  void configure(const char database[], const char host[], const uint16_t port = 8086);

  //if you require authorization to write to InfluxDB, set the username and password here
  void authorize(const char username[],const char password[]);

  //if you are using HTTPS, specify your CA certificate:
  void addCertificate(const char ROOT_CERT[]);

  /*
   * write tags and fields to the measurement. These will likely be made by sprintf,
   * or the Arduino String class (which I don't particularly like, but each to their own)
   * The example .ino file might be useful to see.
   * for both tags and fields, the format is <key=value>.
   * If you have multiple values, separate each pair with a comma,
   * but do not put any spaces in anywhere.
   * example:
   *  tags: "tag_1=A,another_tag=bee"
   *  fields: "x=0.9823401,y=-0.1"
   */
  bool write(const char *measurement, const char *tagString, const char *fieldString);

  //tags are optional: you can just specify a measurement plus fields
  bool write(const char *measurement, const char *fieldString);
  
  /*
   * return the HTTPClient response
   * if POST is unsuccessful a negative number is returned:
   * you can find what this means in the HTTPClient.h header file.
   * Otherwise, it returns an HTTP code, e.g. 204 or 403)
   */
  int getResponse();

  //a bool to check if https is being used.
  bool isSecure();

/*
 * TODO: method ping() to test configuration values.
 * curl -i -XPOST http://localhost:8086/query --data-urlencode "q=CREATE DATABASE mydb"
 */

/*
 * TODO: method to create a database.
 * curl -i -XPOST http://localhost:8086/query --data-urlencode "q=CREATE DATABASE mydb"
 */

private:
  char* _database;
  char* _host;
  uint16_t _port;

  char* _username;
  char* _password;
  char* _cert;
  bool _isAuthorised = false;
  bool _isSecure = false;
  int _latestResponse; //storing the latest response in case user wants to inspect
};

#endif
