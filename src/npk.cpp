#include "npk.h"

NPK::NPK(HardwareSerial &serial, uint32_t baudrate, uint8_t dere_pin) : npkSer(serial)
{
    this->_baudrate = baudrate;
    this->_dere_pin = dere_pin;
}
NPK::~NPK() {}

void NPK::begin(void)
{
    npkSer.begin(_baudrate);
    pinMode(_dere_pin, OUTPUT);
    digitalWrite(_dere_pin, LOW);
}

uint8_t NPK::getData(uint16_t *data, uint8_t node)
{
    uint8_t error = ERROR_NONE;
    data[0] = UINT16_MAX;
    data[1] = UINT16_MAX;
    data[2] = UINT16_MAX;
    byte req[8] = {node, 0x03, 0x00, 0x1e, 0x00, 0x03, 0x00, 0x00};
    uint16_t crc = calculateCRC(req, 6);
    req[6] = lowByte(crc);
    req[7] = highByte(crc);
    digitalWrite(_dere_pin, HIGH);
    if (npkSer.write(req, 8) != 8)
    {
        error = ERROR_WRITE;
    }
    npkSer.flush();

    digitalWrite(_dere_pin, LOW);
    uint32_t timeout = millis() + WAIT_ANSWER;

    while (npkSer.available() < FRAMESIZE + 1)
    {
        if (timeout < millis())
        {
            error = ERROR_TIMEOUT;
            break;
        }
    }
    npkSer.read();
    Serial.printf("avail: %d\n", npkSer.available());
    if (error == ERROR_NONE)
    {
        byte resp[FRAMESIZE] = {255};
        if (npkSer.available() >= FRAMESIZE)
        {
            npkSer.readBytes(resp, FRAMESIZE);
            if ((calculateCRC(resp, FRAMESIZE - 2)) == ((resp[FRAMESIZE - 1] << 8) | resp[FRAMESIZE - 2]))
            {
                data[0] = resp[3] << 8 | resp[4];
                data[1] = resp[5] << 8 | resp[6];
                data[2] = resp[7] << 8 | resp[8];
            }
            else
            {
                error = ERROR_CRC;
            }
        }
        else
        {
            error = ERROR_NOT_ENOUGH_DATA;
        }
    }

    flush(WAIT_ANSWER);
    if (npkSer.available())
    {
        error = ERROR_SPAM;
    }
    return error;
}

uint8_t NPK::getNitrogen(uint16_t *data, uint8_t node)
{
    uint16_t d[3];
    uint8_t e = getData(d, node);
    *data = d[0];
    return e;
}
uint8_t NPK::getPhosphorus(uint16_t *data, uint8_t node)
{
    uint16_t d[3];
    uint8_t e = getData(d, node);
    *data = d[1];
    return e;
}
uint8_t NPK::getPotassium(uint16_t *data, uint8_t node)
{
    uint16_t d[3];
    uint8_t e = getData(d, node);
    *data = d[2];
    return e;
}

uint16_t NPK::calculateCRC(uint8_t *array, uint8_t num)
{
    uint16_t _crc, _flag;
    _crc = 0xFFFF;
    for (uint8_t i = 0; i < num; i++)
    {
        _crc ^= (uint16_t)array[i];
        for (uint8_t j = 8; j; j--)
        {
            _flag = _crc & 0x0001;
            _crc >>= 1;
            if (_flag)
                _crc ^= 0xA001;
        }
    }
    return _crc;
}
void NPK::flush(uint32_t flushtime)
{
    unsigned long _flushtime = millis() + flushtime;
    while (npkSer.available() || _flushtime >= millis())
    {
        if (npkSer.available()) // read serial if any old data is available
            npkSer.read();
        delay(1);
    }
}