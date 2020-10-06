#ifndef Status_hpp
#define Status_hpp

#include <bitset>

#define MAX_ERROR_LIFE_CYCLE  5


class Status {

    public:
    bool dataSendToggle;
    const int bit_sensor = 0;  // sensor
    const int bit_paired = 1;  // bluetooth
    const int bit_wan = 2;     // internet access
    const int bit_cloud = 3;   // publish cloud
    const int bit_code0 = 4;   // error code bit 0
    const int bit_code1 = 5;   // error code bit 1
    const int bit_code2 = 6;   // error code bit 2
    const int bit_code3 = 7;   // error code bit 3

    const int ecode_sensor_ok = 0;
    const int ecode_sensor_read_fail = 1;
    const int ecode_sensor_timeout = 2;
    const int ecode_wifi_fail = 3;
    const int ecode_ifdb_write_fail = 4;
    const int ecode_ifdb_dns_fail = 5;
    const int ecode_json_parser_error = 6;
    const int ecode_invalid_config = 7;
    const int ecode_api_write_fail = 8;

 /***
 * Activate error code bits section with
 * the next possible errors:
 * ecode_sensor_ok          =   0;
 * ecode_sensor_read_fail   =   1;
 * ecode_sensor_timeout     =   2;
 * ecode_wifi_fail          =   3;
 * ecode_ifdb_write_fail    =   4;
 * ecode_ifdb_dns_fail      =   5;
 * ecode_json_parser_error  =   6;
 * ecode_invalid_config     =   7;
 * ecode_api_write_fail     =   8;
 **/

    void statusOn(int bit);
    void statusOff(int bit);
    void setErrorCode(unsigned int error);
    unsigned int getErrorCode(void);
    void updateStatusError(void);
};

#if !defined(NO_GLOBAL_INSTANCES) && !defined(NO_GLOBAL_STATUSHANDLER)
extern Status st;
#endif

#endif

