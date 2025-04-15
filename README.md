# AquaV1 - Система мониторинга аквариума

## Описание
Система мониторинга и управления аквариумом на базе ESP32. Проект включает в себя:
- Мониторинг температуры воды (внутренний и внешний датчики)
- Мониторинг параметров помещения (температура, влажность, давление)
- Управление реле через веб-интерфейс и локальное управление
- Логирование данных
- LCD дисплей для отображения информации
- Веб-сервер для удаленного управления

## Аппаратная часть
- M5 Stack Atom Lite (ESP32)
  - 240MHz dual core
  - 520KB SRAM
  - 4MB Flash
  - Встроенный RGB LED
  - Встроенная кнопка
- LCD дисплей I2C (20x4)
- Датчики температуры DS18B20 (2 шт)
- Датчик температуры/влажности/давления BME280
- RTC модуль DS1307
- Энкодер для управления меню
- Реле для управления оборудованием

## Подключение
- ENCODER_CLK: GPIO 25
- ENCODER_DT: GPIO 32
- BUTTON_PIN: GPIO 33 (встроенная кнопка M5 Stack)
- RELAY_PIN: GPIO 23
- ONE_WIRE_BUS: GPIO 19 (для датчиков DS18B20)
- I2C_SDA: GPIO 21 (LCD, BME280, RTC)
- I2C_SCL: GPIO 22 (LCD, BME280, RTC)
- RGB LED: GPIO 27 (встроенный в M5 Stack)

## Функциональность
1. **Мониторинг температуры**
   - Датчик в аквариуме 20л (DS18B20)
   - Датчик в аквариуме 10л (DS18B20)
   - Температура помещения (BME280)

2. **Управление реле**
   - Автоматическое управление по времени (8:00-19:00)
   - Ручное управление через меню
   - Удаленное управление через веб-интерфейс
   - Сброс ручного управления в полночь

3. **Мониторинг помещения**
   - Температура (BME280)
   - Влажность (BME280)
   - Давление (BME280)

4. **Логирование данных**
   - Хранение последних 15 записей
   - Отображение логов в текстовом и графическом виде
   - Параметры: температура, влажность, давление

5. **Веб-интерфейс**
   - Отображение текущих параметров
   - Управление реле
   - Просмотр логов
   - Настройка времени работы реле

6. **Локальное управление**
   - Меню на LCD дисплее (20x4)
   - Управление энкодером и кнопкой
   - Настройка времени (RTC)

## Зависимости
- Arduino.h
- Wire.h
- LiquidCrystal_I2C
- OneWire
- DallasTemperature
- ESP32Encoder
- WiFi.h
- RTClib
- Adafruit_BME280
- WebServer.h

## Структура проекта
```
├── include/
│   ├── config.h        - Конфигурация пинов и параметров
│   ├── controls.h      - Управление энкодером и кнопкой
│   ├── display.h       - Работа с LCD дисплеем
│   ├── logger.h        - Логирование данных
│   ├── net.h           - Сетевая функциональность
│   ├── web_server.h    - Веб-сервер
│   └── secrets.h       - Конфиденциальные данные
├── src/
│   ├── main.cpp        - Основной код
│   ├── controls.cpp    - Реализация управления
│   ├── display.cpp     - Реализация дисплея
│   ├── logger.cpp      - Реализация логирования
│   ├── net.cpp         - Реализация сети
│   └── web_server.cpp  - Реализация веб-сервера
├── lib/                - Внешние библиотеки
└── platformio.ini      - Конфигурация PlatformIO
```

## Настройка
1. Скопируйте `secrets.h.example` в `secrets.h`
2. Настройте параметры WiFi в `secrets.h`:
   - WIFI_SSID - имя вашей WiFi сети
   - WIFI_PASSWORD - пароль от WiFi сети
3. Соберите схему согласно подключению
4. Загрузите прошивку на M5 Stack Atom Lite через PlatformIO

## Использование
1. Включите устройство
2. Подключитесь к WiFi сети
3. Откройте веб-интерфейс по IP адресу устройства
4. Используйте энкодер и кнопку для навигации по меню
5. Управляйте реле через веб-интерфейс или локальное меню
6. Настройте время работы реле (по умолчанию 8:00-19:00)

## Лицензия
MIT License