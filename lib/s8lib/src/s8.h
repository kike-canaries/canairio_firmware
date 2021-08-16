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


	Example of packet (send, hexadecimal bytes):
	<FE> <04> <00> <03> <00> <01> <D5> <C5>

	<FE> -> Any address (1 byte = 8 bits)
	<04> -> Function code  (1 byte)
	<00> <03> -> Input Register 4 (IR4, #3) (2 bytes = 16 bits), get CO2 measure
	<00> <01> -> Read 1 Word (2 bytes)
	<D5> <C5> -> CRC (2 bytes)


	Answer:
	<FE> <04> <02> <01> <90> <AC> <D8>

	<FE> -> Any address (1 byte)
	<04> -> Function code  (1 byte)
	<02> -> Length (number of bytes)
	<01> <90> -> CO2 value (2 bytes)
	<AC> <D8> -> CRC (2 bytes)


	Reference: "Modbus on Senseair S8" https://rmtplusstoragesenseair.blob.core.windows.net/docs/Dev/publicerat/TDE2067.pdf

***************************************************************************************************************************/


#ifndef _SENSEAIR_S8
    #define _SENSEAIR_S8

    #if defined ARDUINO_ARCH_SAMD || defined ARDUINO_ARCH_SAM21D || defined ARDUINO_ARCH_ESP32 || defined ARDUINO_SAM_DUE || ARDUINO_ARCH_APOLLO3
        #undef USE_SOFTWARE_SERIAL
    #else
        #define USE_SOFTWARE_SERIAL
    #endif

    #include "Arduino.h"

    #ifdef USE_SOFTWARE_SERIAL       
        #include <SoftwareSerial.h>
    #endif

    #define S8_LOG_LEVEL_NONE       (0)
    #define S8_LOG_LEVEL_ERROR      (1)
    #define S8_LOG_LEVEL_WARN       (2)
    #define S8_LOG_LEVEL_INFO       (3)
    #define S8_LOG_LEVEL_DEBUG      (4)
    #define S8_LOG_LEVEL_VERBOSE    (5)

    #ifdef CORE_DEBUG_LEVEL
        #define S8_LOG_LEVEL CORE_DEBUG_LEVEL
    #else
        #define S8_LOG_LEVEL S8_LOG_LEVEL_NONE
    #endif


    #if (S8_LOG_LEVEL > S8_LOG_LEVEL_NONE)

        /* Serial port for debug */

        // Uncomment if you use softwareserial for debug
        //#define S8_DEBUG_SOFTWARE_SERIAL
        
        #ifdef S8_DEBUG_SOFTWARE_SERIAL
            /* Modify if you use softwareserial for debug */
            #define S8_DEBUG_SERIAL_RX 13
            #define S8_DEBUG_SERIAL_TX 15
        #else
            /* Modify if you use hardware serial for debug */
            #define S8_DEBUG_SERIAL Serial
        #endif

        /* Debug format */
        #define S8_LOG(format, ...) S8_DEBUG_SERIAL.printf(format, ##__VA_ARGS__)
    #else
        #define S8_LOG(format, ...)
    #endif


    #define S8_BAUDRATE 9600         // Device to S8 Serial baudrate (should not be changed)
    #define S8_TIMEOUT  5            // Timeout for communication
    #define S8_LEN_BUF_MSG  20       // Max length of buffer for communication with the sensor

    #define S8_LEN_FIRMVER  10       // Length of software version


    // Modbus
    #define MODBUS_ANY_ADDRESS                  0XFE    // S8 uses any address
    #define MODBUS_FUNC_READ_HOLDING_REGISTERS  0X03    // Read holding registers (HR)
    #define MODBUS_FUNC_READ_INPUT_REGISTERS    0x04    // Read input registers (IR)
    #define MODBUS_FUNC_WRITE_SINGLE_REGISTER   0x06    // Write single register (SR)


    // Input registers for S8
	#define MODBUS_IR1             0x0000  // MeterStatus
	#define MODBUS_IR2             0x0001  // AlarmStatus
	#define MODBUS_IR3             0x0002  // OutputStatus
	#define MODBUS_IR4             0x0003  // Space CO2
	#define MODBUS_IR22            0x0015  // PWM Output
    #define MODBUS_IR26            0x0019  // Sensor Type ID High
    #define MODBUS_IR27            0x001A  // Sensor Type ID Low
    #define MODBUS_IR28            0x001B  // Memory Map version
    #define MODBUS_IR29            0x001C  // FW version Main.Sub
    #define MODBUS_IR30            0x001D  // Sensor ID High
    #define MODBUS_IR31            0x001E  // Sensor ID Low


    // Holding registers for S8
    #define MODBUS_HR1             0x0000  // Acknowledgement Register
    #define MODBUS_HR2             0x0001  // Special Command Register
	#define MODBUS_HR32            0x001F  // ABC Period


    // Meter status
    #define S8_MASK_METER_FATAL_ERROR                    0x0001   // Fatal error
    #define S8_MASK_METER_OFFSET_REGULATION_ERROR        0x0002   // Offset regulation error
    #define S8_MASK_METER_ALGORITHM_ERROR                0x0004   // Algorithm error
    #define S8_MASK_METER_OUTPUT_ERROR                   0x0008   // Output error
    #define S8_MASK_METER_SELF_DIAG_ERROR                0x0010   // Self diagnostics error
    #define S8_MASK_METER_OUT_OF_RANGE                   0x0020   // Out of range
    #define S8_MASK_METER_MEMORY_ERROR                   0x0040   // Memory error

    // Output status
    #define S8_MASK_OUTPUT_ALARM                         0x0001   // Alarm output status (inverted due to Open Collector)
    #define S8_MASK_OUTPUT_PWM                           0x0002   // PWM output status (=1 -> full output)


    // Acknowledgement flags
    #define S8_MASK_CO2_BACKGROUND_CALIBRATION   0x0020   // CO2 Background calibration performed = 1
    #define S8_MASK_CO2_NITROGEN_CALIBRATION     0x0040   // CO2 Nitrogen calibration performed = 1    

    // Calibration definitions for special command
    #define S8_CO2_BACKGROUND_CALIBRATION        0x7C06   // CO2 Background calibration
    #define S8_CO2_ZERO_CALIBRATION              0x7C07   // CO2 Zero calibration


    struct S8_sensor {
        char firmver[S8_LEN_FIRMVER + 1];
        int16_t co2;
    };

    class S8
    {
        public:
            S8(Stream &serial);                                                  // Initialize
            void get_firmware_version(char firmwver[]);                          // Get firmware version
            int16_t get_co2();                                                   // Get CO2 value in ppm
            int16_t get_ABC_period();                                            // Get ABC period in hours
            bool set_ABC_period(int16_t period);                                 // Set ABC period (4 - 4800 hours, 0 to disable)
            int16_t get_acknowledgement();                                       // Get acknowledgement flags
            bool send_special_command(int16_t command);                          // Send special command
            int16_t get_meter_status();                                          // Get meter status
            int16_t get_alarm_status();                                          // Get alarm status            
            int16_t get_output_status();                                         // Get output status
            int16_t get_PWM_output();                                            // Get PWM output
            int32_t get_sensor_type_ID();                                        // Get sensor type ID
            int32_t get_sensor_ID();                                             // Get sensor ID
            int16_t get_memory_map_version();                                    // Get memory map version

        private:
            Stream* mySerial;                                                    // Communication serial with the sensor
            uint8_t buf_msg[S8_LEN_BUF_MSG];                                     // Buffer for communication messages with the sensor

            void serial_write_bytes(uint8_t size);                               // Send bytes to sensor
            uint8_t serial_read_bytes(uint8_t max_bytes, int timeout_seconds);   // Read received bytes from sensor
            bool valid_response(uint8_t func, uint8_t nb);                       // Check if response is valid according to sent command
            bool valid_response_len(uint8_t func, uint8_t nb, uint8_t len);      // Check if response is valid according to sent command and checking expected total length
            void send_cmd(uint8_t func, uint16_t cmd, uint16_t value);           // Send command
            void print_buffer(uint8_t size);                                     // Show buffer in hex bytes
            void print_binary(int16_t number);                                   // Show number in bits
    };

#endif
