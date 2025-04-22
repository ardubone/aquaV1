#include "temperature.h"

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress tank20SensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};
DeviceAddress tank10SensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};

void initTemperatureSensors() {
    sensors.begin();
}

void requestTemperatures() {
    sensors.requestTemperatures();
}

float getTank20Temperature() {
    return sensors.getTempC(tank20SensorAddr);
}

float getTank10Temperature() {
    return sensors.getTempC(tank10SensorAddr);
}

bool isSensorConnected(uint8_t index) {
    return sensors.getDeviceCount() > index;
} 