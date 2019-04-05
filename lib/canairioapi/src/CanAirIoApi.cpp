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
    //copy these strings to private class pointers for future use
    _sensorId = new char[strlen(sensorId) + 1];
    strcpy(_sensorId,sensorId); //strncpy fails for some reason
    _endpoint = new char[strlen(endpoint)+1];
    strcpy(_endpoint,endpoint); //strncpy fails for some reason
    _host = new char[strlen(host)+1];
    strcpy(_host,host);
    _port = port;
}

void CanAirIoApi::authorize(const char username[], const char password[])
{
    //copy these strings to private class pointers for future use

    _username = new char[strlen(username)+1];
    strcpy(_username,username);
    _password = new char[strlen(password)+1];
    strcpy(_password,password);
    _isAuthorised = true;

}

bool CanAirIoApi::write(uint16_t pm1, uint16_t apm25, uint16_t apm10, float humi, float tmp, float lat, float lon, float alt, float spd, int stime)
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
    http.addHeader("Content-Type", "text/plain"); // not sure what influx is looking for but this works?

    if(_isAuthorised)
    {
        http.setAuthorization(_username,_password);
    }

    char writeBuf[512]; // ¯\_(ツ)_/¯ 

    if(debug) Serial.println(writeBuf);
    _latestResponse = http.POST(writeBuf);
    http.end();
    return _latestResponse == 204;
}

int CanAirIoApi::getResponse()
{
    return _latestResponse;
}

bool CanAirIoApi::isSecure()
{
    return _isSecure;
}
