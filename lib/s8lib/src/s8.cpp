/***************************************************************************************************************************

	SenseAir S8 Library for Serial Modbus Communication

	Copyright (c) 2021 Josep Comas

	Permission is hereby granted, free of charge, to any person obtaining a copy
	of this software and associated documentation files (the "Software"), to deal
	in the Software without restriction, including without limitation the rights
	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
	copies of the Software, and to permit persons to whom the Software is
	furnished to do so, subject to the following conditions:

	The above copyright notice and this permission notice shall be included in all
	copies or substantial portions of the Software.

	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
	LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
	SOFTWARE.

***************************************************************************************************************************/

#include "s8.h"
#include "modbus_crc.h"

#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
    #ifdef S8_DEBUG_SOFTWARE_SERIAL
        SoftwareSerial S8_DEBUG_SERIAL(S8_DEBUG_SERIAL_RX, S8_DEBUG_SERIAL_TX);
    #endif        
#endif

/* Initialize */
S8::S8(Stream &serial)
{
    mySerial = &serial;
}


/* Get firmware version */
void S8::get_firmware_version(char firmver[]) {

    if (firmver == NULL) {
        return;
    }

    strcpy(firmver, "");

    // Ask software version
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR29, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        snprintf(firmver, S8_LEN_FIRMVER, "%0u.%0u", buf_msg[3], buf_msg[4]);
        S8_LOG("DEBUG: Firmware version: %s\n", firmver);
    } else {
        S8_LOG("DEBUG: Firmware version not available!\n");
    }
    
}


/* Get CO2 value in ppm */
int16_t S8::get_co2() {

    int16_t co2 = 0;

    // Ask CO2 value
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR4, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        co2 = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: CO2 value = %d ppm\n", co2);
    } else {
        S8_LOG("DEBUG: Error getting CO2 value!\n");
    }
    return co2;
}


/* Read ABC period */
int16_t S8::get_ABC_period() {

    int16_t period = 0;

    // Ask ABC period
    send_cmd(MODBUS_FUNC_READ_HOLDING_REGISTERS, MODBUS_HR32, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_HOLDING_REGISTERS, nb, 7)) {
        period = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: ABC period = %d hours\n", period);
    } else {
        S8_LOG("DEBUG: Error getting ABC period!\n");
    }
    return period;
}


/* Setup ABC period, default 180 hours (7.5 days) */
bool S8::set_ABC_period(int16_t period) {
    uint8_t buf_msg_sent[8]; 
    bool result = false;

    if (period >= 0 && period <= 4800) {   // 0 = disable ABC algorithm

        // Ask set ABC period
        send_cmd(MODBUS_FUNC_WRITE_SINGLE_REGISTER, MODBUS_HR32, period);

        // Save bytes sent
        memcpy(buf_msg_sent, buf_msg, 8);

        // Wait response
        memset(buf_msg, 0, S8_LEN_BUF_MSG);
        serial_read_bytes(8, S8_TIMEOUT);

        // Check response
        if (memcmp(buf_msg_sent, buf_msg, 8) == 0) {
            result = true;
            S8_LOG("DEBUG: Successful setting of ABC period\n");
        } else {
            S8_LOG("DEBUG: Error in setting of ABC period!\n");
        }

    } else {
        S8_LOG("DEBUG: Invalid ABC period!\n");
    }

    return result;
}


/* Read acknowledgement flags */
int16_t S8::get_acknowledgement() {

    int16_t flags = 0;

    // Ask acknowledgement flags
    send_cmd(MODBUS_FUNC_READ_HOLDING_REGISTERS, MODBUS_HR1, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_HOLDING_REGISTERS, nb, 7)) {
        flags = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: Acknowledgement flags = b");
#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
        print_binary(flags);
#endif
        S8_LOG("\n");        
    } else {
        S8_LOG("DEBUG: Error getting acknowledgement flags!\n");
    }
    return flags;
}


/* Send special command (high = command, low = parameter) */
/*
   Command = 0x7C, 
   Parameter = 0x06 CO2 background calibration
   Parameter = 0x07 CO2 zero calibration
*/
bool S8::send_special_command(int16_t command) {
    uint8_t buf_msg_sent[8]; 
    bool result = false;

    // Ask set user special command
    send_cmd(MODBUS_FUNC_WRITE_SINGLE_REGISTER, MODBUS_HR2, command);

    // Save bytes sent
    memcpy(buf_msg_sent, buf_msg, 8);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    serial_read_bytes(8, S8_TIMEOUT);

    // Check response
    if (memcmp(buf_msg_sent, buf_msg, 8) == 0) {
        result = true;
        S8_LOG("DEBUG: Successful setting user special command\n");
    } else {
        S8_LOG("DEBUG: Error in setting user special command!\n");
    }

    return result;
}


/* Read meter status */
int16_t S8::get_meter_status() {

    int16_t status = 0;

    // Ask meter status
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR1, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        status = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: Meter status = b");
#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
        print_binary(status);
#endif
        S8_LOG("\n");        
    } else {
        S8_LOG("DEBUG: Error getting meter status!\n");
    }
    return status;
}


/* Read alarm status */
int16_t S8::get_alarm_status() {

    int16_t status = 0;

    // Ask alarm status
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR2, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        status = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: Alarm status = b");
#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
        print_binary(status);
#endif
        S8_LOG("\n");        
    } else {
        S8_LOG("DEBUG: Error getting alarm status!\n");
    }
    return status;
}


/* Read output status */
int16_t S8::get_output_status() {

    int16_t status = 0;

    // Ask output status
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR3, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        status = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: Output status = b");
#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
        print_binary(status);
#endif
        S8_LOG("\n");
    } else {
        S8_LOG("DEBUG: Error getting output status!\n");
    }
    return status;
}


/* Read PWM output (0x3FFF = 100%) */
int16_t S8::get_PWM_output() {

    int16_t pwm = 0;

    // Ask PWM output
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR22, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        pwm = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: PWM output = %d\n", pwm);
    } else {
        S8_LOG("DEBUG: Error getting PWM output!\n");
    }
    return pwm;
}


/* Read sensor type ID */
int32_t S8::get_sensor_type_ID() {

    int32_t sensorType = 0;

    // Ask sensor type ID (high)
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR26, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {

        // Save sensor type ID (high)
        sensorType = ((buf_msg[4] << 16) & 0x00FF0000);
        //sensorType = ((buf_msg[3] << 24) & 0xFF000000) | ((buf_msg[4] << 16) & 0x00FF0000);

        // Ask sensor type ID (low)
        send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR27, 0x0001);

        // Wait response
        memset(buf_msg, 0, S8_LEN_BUF_MSG);
        nb = serial_read_bytes(7, S8_TIMEOUT);

        // Check response and get data
        if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {

            sensorType |= ((buf_msg[3] << 8) & 0x0000FF00) | (buf_msg[4] & 0x000000FF);
            S8_LOG("DEBUG: Sensor type ID = 0x%08x\n", sensorType);

        } else {
            S8_LOG("DEBUG: Error getting sensor type ID (low)!\n");
        }        

    } else {
        S8_LOG("DEBUG: Error getting sensor type ID (high)!\n");
    }

    return sensorType;
}


/* Read sensor ID */
int32_t S8::get_sensor_ID() {

    int32_t sensorID = 0;

    // Ask sensor ID (high)
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR30, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {

        // Save sensor ID (high)
        sensorID = ((buf_msg[3] << 24) & 0xFF000000) | ((buf_msg[4] << 16) & 0x00FF0000);

        // Ask sensor ID (low)
        send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR31, 0x0001);

        // Wait response
        memset(buf_msg, 0, S8_LEN_BUF_MSG);
        nb = serial_read_bytes(7, S8_TIMEOUT);

        // Check response and get data
        if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {

            sensorID |= ((buf_msg[3] << 8) & 0x0000FF00) | (buf_msg[4] & 0x000000FF);
            S8_LOG("DEBUG: Sensor ID = 0x%08x\n", sensorID);

        } else {
            S8_LOG("DEBUG: Error getting sensor ID (low)!\n");
        }        
        
    } else {
        S8_LOG("DEBUG: Error getting sensor ID (high)!\n");
    }

    return sensorID;
}


/* Read memory map version */
int16_t S8::get_memory_map_version() {

    int16_t mmVersion = 0;

    // Ask memory map version
    send_cmd(MODBUS_FUNC_READ_INPUT_REGISTERS, MODBUS_IR28, 0x0001);

    // Wait response
    memset(buf_msg, 0, S8_LEN_BUF_MSG);
    uint8_t nb = serial_read_bytes(7, S8_TIMEOUT);

    // Check response and get data
    if (valid_response_len(MODBUS_FUNC_READ_INPUT_REGISTERS, nb, 7)) {
        mmVersion = ((buf_msg[3] << 8) & 0xFF00) | (buf_msg[4] & 0x00FF);
        S8_LOG("DEBUG: Memory map version = 0x%04x\n", mmVersion);
    } else {
        S8_LOG("DEBUG: Error getting memory map version!\n");
    }
    return mmVersion;
}


/* Check valid response and length of received message */
bool S8::valid_response_len(uint8_t func, uint8_t nb, uint8_t len) {
    bool result = false;

    if (nb == len) {
        result = valid_response(func, nb);
    } else {
        S8_LOG("DEBUG: Unexpected length\n");
    }

    return result;
}


/* Check if it is a valid message response of the sensor */
bool S8::valid_response(uint8_t func, uint8_t nb) {

    uint16_t crc16;
    bool result = false;

    if (nb >= 7) {
        crc16 = modbus_CRC16(buf_msg, nb-2);
        if ((buf_msg[nb-2] == (crc16 & 0x00FF)) && (buf_msg[nb-1] == ((crc16 >> 8) & 0x00FF))) {

            if (buf_msg[0] == MODBUS_ANY_ADDRESS && (buf_msg[1] == MODBUS_FUNC_READ_HOLDING_REGISTERS || buf_msg[1] == MODBUS_FUNC_READ_INPUT_REGISTERS) && buf_msg[2] == nb-5) {
                S8_LOG("DEBUG: Valid response\n");
                result = true;

            } /* else if (buf_msg[0] == CM1106_MSG_NAK && nb == 4) {
                S8_LOG("DEBUG: Response with error 0x%02x\n", buf_msg[2]);
                // error 0x02 = cmd not recognised, invalid checksum...
                // If invalid length then no response.
            } */

        } else {
            S8_LOG("DEBUG: Checksum/length is invalid\n");
        }

    } else {
        S8_LOG("DEBUG: Invalid length\n");
    }
    
    return result;
}


/* Send command */
void S8::send_cmd( uint8_t func, uint16_t cmd, uint16_t value) {

    uint16_t crc16;

    if (((func == MODBUS_FUNC_READ_HOLDING_REGISTERS || func == MODBUS_FUNC_READ_INPUT_REGISTERS) && value >= 1) || (func == MODBUS_FUNC_WRITE_SINGLE_REGISTER)) {
        buf_msg[0] = MODBUS_ANY_ADDRESS;                // Address
        buf_msg[1] = func;                              // Function
        buf_msg[2] = (cmd >> 8) & 0x00FF;               // High-input register
        buf_msg[3] = cmd & 0x00FF;                      // Low-input register
        buf_msg[4] = (value >> 8) & 0x00FF;             // High-word to read or setup
        buf_msg[5] = value & 0x00FF;                    // Low-word to read or setup
        crc16 = modbus_CRC16(buf_msg, 6);
        //Serial.printf("CRC value: 0x%04x\n", crc16);
        buf_msg[6] = crc16 & 0x00FF;
        buf_msg[7] = (crc16 >> 8) & 0x00FF;
        serial_write_bytes(8);        
    }
}


/* Send bytes to sensor */
void S8::serial_write_bytes(uint8_t size) {

#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)     
    S8_LOG("DEBUG: Bytes to send => ");
    print_buffer(size);
#endif

    mySerial->write(buf_msg, size);
    mySerial->flush();
}


/* Read answer of sensor */
uint8_t S8::serial_read_bytes(uint8_t max_bytes, int timeout_seconds) {

    time_t start_t, end_t;
    //double diff_t;
    time(&start_t); end_t = start_t;
    bool readed = false;

    uint8_t nb = 0;
    if (max_bytes > 0 && timeout_seconds > 0) {

        S8_LOG("DEBUG: Bytes received => ");

        while ((difftime(end_t, start_t) <= timeout_seconds) && !readed) {
            if(mySerial->available()) {
                nb = mySerial->readBytes(buf_msg, max_bytes);
                readed = true;
            }            
            time(&end_t);
        }

#if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)
        print_buffer(nb);
#endif

    } else {
        S8_LOG("DEBUG: Invalid parameters!\n");
    }

    return nb;
}


/* Show buffer in hex bytes */
void S8::print_buffer(uint8_t size) {

    for (int i = 0; i < size; i++) {
        S8_LOG("0x%02x ", buf_msg[i]);
    }
    S8_LOG("(%u bytes)\n", size);
}


/* Show number in binary */
void S8::print_binary(int16_t number) {
    int16_t k;

    for (int8_t c = 15; c >= 0; c--)
    {
        k = number >> c;

        if (k & 1)
            S8_LOG("1");
        else
            S8_LOG("0");
    }
}
