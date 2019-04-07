#include <HTTPClient.h>
#include "CanAirIoApi.hpp"

CanAirIoApi::CanAirIoApi(bool debug)
{
}

CanAirIoApi::~CanAirIoApi()
{
}

void CanAirIoApi::configure(const char sensorId[], const char endpoint[],const char host[],const uint16_t port)
{   
    _sensorId = new char[strlen(sensorId) + 1];
    strcpy(_sensorId,sensorId); 
    _endpoint = new char[strlen(endpoint)+1];
    strcpy(_endpoint,endpoint);
    _host = new char[strlen(host)+1];
    strcpy(_host,host);
    _port = port;
    if(debug)Serial.println("id:"+String(_sensorId)+" target:"+String(_endpoint)+" host:"+String(_host)+":"+String(port));
}

void CanAirIoApi::authorize(const char username[], const char password[])
{
    _username = new char[strlen(username)+1];
    strcpy(_username,username);
    _password = new char[strlen(password)+1];
    strcpy(_password,password);
    _isAuthorised = true;
    if(debug)Serial.println("usr:"+String(_username)+" pss:"+String(_password));
}

bool CanAirIoApi::write(uint16_t pm1, uint16_t pm25, uint16_t pm10, float hum, float tmp, float lat, float lon, float alt, float spd, int stime)
{   
    HTTPClient http;
    char uri[32];
    sprintf(uri, "/%s", _endpoint);

    if(_isSecure)
    {
       http.begin(_host, _port, uri, _cert);
    }
    else
    {
        http.begin(_host, _port, uri);
    }
    http.addHeader("Content-Type","application/json");
    http.addHeader("cache-control","no-cache");
        if(_isAuthorised)
    {
        http.setAuthorization(_username,_password);
    }

    const int capacity = JSON_ARRAY_SIZE(1) + JSON_OBJECT_SIZE(2) + JSON_OBJECT_SIZE(11);
    StaticJsonDocument <capacity> doc;

    JsonObject data = doc.createNestedObject();
    data["id"] = _sensorId;
    StaticJsonDocument<JSON_OBJECT_SIZE(11)> fields;
    fields["pm1"] = pm1;
    fields["pm25"] = pm25;
    fields["pm10"] = pm10;
    fields["hum"] = hum;
    fields["tmp"] = tmp;
    fields["lat"] = lat;
    fields["lon"] = lon;
    fields["alt"] = alt;
    fields["spd"] = spd;
    fields["stime"] = stime;
    String sfields;
    serializeJson(fields,sfields);
    data["fields"] = sfields;
    if(debug) serializeJsonPretty(doc, Serial);
    String writeBuf;
    serializeJson(doc,writeBuf);
    _latestResponse = http.POST(writeBuf.c_str());
    http.end();
    return _latestResponse == 204;
}

/******************************************************************************
*   C A N A I R I O  P U B L I S H   M E T H O D S
******************************************************************************/

// void canairioWrite(const char *measurement,const char *tagString,const char *fieldString) {
//   Serial.println("\n-->[API] publish..");
//   HTTPClient http;
//   http.begin("http://canairio.herokuapp.com/points/save/");
//   http.setAuthorization(ifusr.c_str(),ifpss.c_str());
//   //http.addHeader("Content-Type","application/json");
//   http.addHeader("Content-Type", "text/plain"); // not sure what influx is looking for but this works?

//   char writeBuf[512]; // ¯\_(ツ)_/¯
//   if (strlen(tagString) > 0){
//     sprintf(writeBuf, "%s,%s %s", measurement, tagString, fieldString); //no comma between tags and fields
//   }
//   else { //no tags
//     sprintf(writeBuf, "%s %s", measurement, fieldString); //no comma between tags and fields
//   }
//   Serial.println(writeBuf);
//   // String payload = "[{"measurement": "cpu_load_short","tags": {"host": "server01","region": "us-west"},"time": "2018-08-10T23:00:00Z","fields": {"value": 0.64}}]";
//   int httpCode = http.POST(writeBuf);

//   if (httpCode == 204) { //Check for the returning code
//     String payload = http.getString();
//     Serial.print(payload);
//   }
//   else{
//     Serial.println("-->[API] Error HTTP: ");
//     Serial.println(httpCode);
//   }
//   Serial.println("-->[API] end");

//   http.end();
// }

int CanAirIoApi::getResponse()
{
    return _latestResponse;
}

bool CanAirIoApi::isSecure()
{
    return _isSecure;
}
