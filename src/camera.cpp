#include "camera.h"
#include "config.h"
#include "esp_camera.h"

#ifdef DEBUG_MODE
#include "debug_mocks.h"
#endif

// Конфигурация камеры OV7670 по I2C
camera_config_t camera_config;

bool initCamera() {
#if !ENABLE_CAMERA
    // Камера отключена в конфигурации
    Serial.println(F("[CAMERA] Камера отключена в конфигурации (ENABLE_CAMERA = 0)"));
    isCameraInitialized = false;
    return false;
#else
    // Настройка конфигурации камеры для OV7670
    // ВАЖНО: OV7670 требует параллельный интерфейс для данных (D0-D7, VSYNC, HREF, PCLK)
    // Если эти пины не подключены, инициализация вызовет ошибку GPIO
    
    camera_config.pin_pwdn = -1;
    camera_config.pin_reset = -1;
    camera_config.pin_xclk = -1;  // XCLK может быть необходим для некоторых модулей
    camera_config.pin_sccb_sda = I2C_SDA;  // SCCB (I2C) для конфигурации камеры
    camera_config.pin_sccb_scl = I2C_SCL;  // SCCB (I2C) для конфигурации камеры
    
    // Параллельные пины данных (D0-D7) - ОБЯЗАТЕЛЬНЫ для OV7670
    // Установите правильные GPIO пины здесь, если камера подключена
    camera_config.pin_d7 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d6 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d5 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d4 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d3 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d2 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d1 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_d0 = -1;  // TODO: установите правильный GPIO
    camera_config.pin_vsync = -1;  // TODO: установите правильный GPIO
    camera_config.pin_href = -1;   // TODO: установите правильный GPIO
    camera_config.pin_pclk = -1;   // TODO: установите правильный GPIO

    camera_config.xclk_freq_hz = 20000000;
    camera_config.ledc_timer = LEDC_TIMER_0;
    camera_config.ledc_channel = LEDC_CHANNEL_0;
    
    // OV7670 поддерживает различные форматы, но JPEG может требовать конвертацию
    // Если JPEG не работает, попробуйте PIXFORMAT_RGB565 или PIXFORMAT_YUV422
    camera_config.pixel_format = PIXFORMAT_JPEG;
    camera_config.frame_size = FRAMESIZE_VGA;  // 640x480
    camera_config.jpeg_quality = 12;
    camera_config.fb_count = 1;  // Один буфер для экономии памяти
    camera_config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
    camera_config.fb_location = CAMERA_FB_IN_PSRAM;  // Требует PSRAM для VGA

    // Инициализация камеры
    esp_err_t err = esp_camera_init(&camera_config);
    if (err != ESP_OK) {
#ifdef DEBUG_MODE
        Serial.println(F("[CAMERA] Инициализация не удалась - продолжаем в DEBUG_MODE"));
        isCameraInitialized = false;
        return false;
#else
        Serial.print(F("[CAMERA] Ошибка инициализации: "));
        Serial.println(err);
        isCameraInitialized = false;
        return false;
#endif
    }

    Serial.println(F("[CAMERA] Камера инициализирована успешно"));
    isCameraInitialized = true;
    return true;
#endif
}

void* getCameraFrame(size_t* len) {
#if !ENABLE_CAMERA
    return nullptr;
#else
    if (!isCameraInitialized) {
        return nullptr;
    }

    camera_fb_t* fb = esp_camera_fb_get();
    if (!fb) {
        return nullptr;
    }

    *len = fb->len;
    return (void*)fb;
#endif
}

void releaseCameraFrame(void* fb) {
#if ENABLE_CAMERA
    if (fb) {
        esp_camera_fb_return((camera_fb_t*)fb);
    }
#endif
}

bool isCameraReady() {
#if ENABLE_CAMERA
    return isCameraInitialized;
#else
    return false;
#endif
}

