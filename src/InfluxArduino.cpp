#include <HTTPClient.h>
#include "InfluxArduino.hpp"

InfluxArduino::InfluxArduino()
{
}

InfluxArduino::~InfluxArduino()
{
}

void InfluxArduino::configure(const char database[],const char host[],const uint16_t port)
{   
    //copy these strings to private class pointers for future use

    _database = new char[strlen(database)+1];
    strcpy(_database,database); //strncpy fails for some reason
    _host = new char[strlen(host)+1];
    strcpy(_host,host);
    _port = port;
}

void InfluxArduino::addCertificate(const char cert[])
{
    //copy these strings to private class pointers for future use

    _cert = new char[strlen(cert)+1];
    strcpy(_cert,cert);
    _isSecure = true;
}

void InfluxArduino::authorize(const char username[], const char password[])
{
    //copy these strings to private class pointers for future use

    _username = new char[strlen(username)+1];
    strcpy(_username,username);
    _password = new char[strlen(password)+1];
    strcpy(_password,password);
    _isAuthorised = true;

}

bool InfluxArduino::write(const char *measurement,const char *fieldString)
{
    write(measurement,"",fieldString);
}

bool InfluxArduino::write(const char *measurement,const char *tagString,const char *fieldString)
{   
    HTTPClient http;
    char uri[32];
    sprintf(uri, "/write?db=%s", _database);

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
    if(strlen(tagString) > 0)
    {
        sprintf(writeBuf,"%s,%s %s",measurement,tagString,fieldString); //no comma between tags and fields
    }

    else
    {
        //no tags
        sprintf(writeBuf,"%s %s",measurement,fieldString); //no comma between tags and fields
    }
    Serial.println(writeBuf);
    _latestResponse = http.POST(writeBuf);
    http.end();
    return _latestResponse == 204;
}

int InfluxArduino::getResponse()
{
    return _latestResponse;
}

bool InfluxArduino::isSecure()
{
    return _isSecure;
}