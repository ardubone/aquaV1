#pragma once

#include <Arduino.h>
#include "config.h"

// Инициализация камеры OV7670
bool initCamera();

// Получение кадра для MJPEG стрима
// Возвращает указатель на буфер кадра, или nullptr при ошибке
void* getCameraFrame(size_t* len);

// Освобождение буфера кадра
void releaseCameraFrame(void* fb);

// Проверка готовности камеры
bool isCameraReady();

