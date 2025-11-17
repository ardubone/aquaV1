#include "temperature.h"
#include "config.h"

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

DeviceAddress tank20SensorAddr = {0x28, 0x80, 0xF2, 0x53, 0x00, 0x00, 0x00, 0x81};
DeviceAddress tank10SensorAddr = {0x28, 0x29, 0x1F, 0x52, 0x00, 0x00, 0x00, 0xF2};

void initTemperatureSensors() {
    sensors.begin();
}

void requestTemperatures() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return; // Не запрашиваем температуру, если датчики не инициализированы
    }
#endif
    sensors.requestTemperatures();
}

float getTank20Temperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank20Temperature();
    }
#endif
    return sensors.getTempC(tank20SensorAddr);
}

float getTank10Temperature() {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return getMockTank10Temperature();
    }
#endif
    return sensors.getTempC(tank10SensorAddr);
}

bool isSensorConnected(uint8_t index) {
#ifdef DEBUG_MODE
    if (!isDs18b20Initialized) {
        return false;
    }
#endif
    return sensors.getDeviceCount() > index;
} 