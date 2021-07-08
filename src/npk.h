#include <Arduino.h>

#ifndef NPK_h
#define NPK_h

#define FRAMESIZE 11
#define WAIT_ANSWER 200

#define ERROR_NONE 0
#define ERROR_TIMEOUT 1
#define ERROR_NOT_ENOUGH_DATA 2
#define ERROR_CRC 3
#define ERROR_SPAM 4
#define ERROR_WRITE 5

class NPK
{
public:
    NPK(HardwareSerial &serial, uint32_t baudrate, uint8_t dere_pin);
    virtual ~NPK();

    void begin(void);
    uint8_t getData(uint16_t *data, uint8_t node);
    uint8_t getNitrogen(uint16_t *data, uint8_t node);
    uint8_t getPhosphorus(uint16_t *data, uint8_t node);
    uint8_t getPotassium(uint16_t *data, uint8_t node);

private:
    HardwareSerial &npkSer;
    uint32_t _baudrate;
    uint8_t _dere_pin;
    uint16_t calculateCRC(uint8_t *array, uint8_t num);
    void flush(uint32_t flushtime = 0);
};

#endif