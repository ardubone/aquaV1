#include "atom_button.h"
#include "config.h"

#define ATOM_BUTTON_PIN 39
#define DEBOUNCE_DELAY_MS 50
#define DOUBLE_CLICK_TIMEOUT_MS 2000

static bool lastButtonState = false;
static unsigned long lastDebounceTime = 0;
static bool buttonState = false;
static bool singleClickFlag = false;
static bool doubleClickFlag = false;
static unsigned long lastClickTime = 0;
static bool waitingForDoubleClick = false;

void initAtomButton() {
    pinMode(ATOM_BUTTON_PIN, INPUT_PULLUP);
    lastButtonState = digitalRead(ATOM_BUTTON_PIN) == LOW;
    buttonState = lastButtonState;
    Serial.print(F("[ATOM_BUTTON] Инициализация кнопки на GPIO 39, начальное состояние: "));
    Serial.println(buttonState ? F("нажата") : F("отпущена"));
}

void updateAtomButton() {
    bool currentState = digitalRead(ATOM_BUTTON_PIN) == LOW;
    
    // Дебаунс
    if (currentState != lastButtonState) {
        lastDebounceTime = millis();
    }
    
    if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY_MS) {
        if (currentState != buttonState) {
            bool previousState = buttonState;
            buttonState = currentState;
            
            // Обработка нажатия (переход с false на true - кнопка нажата)
            if (buttonState && !previousState) {
                unsigned long currentTime = millis();
                
                Serial.print(F("[ATOM_BUTTON] Нажатие! waitingForDoubleClick="));
                Serial.print(waitingForDoubleClick);
                if (waitingForDoubleClick) {
                    unsigned long timeDiff = currentTime - lastClickTime;
                    Serial.print(F(", timeDiff="));
                    Serial.print(timeDiff);
                    Serial.print(F(" мс, timeout="));
                    Serial.print(DOUBLE_CLICK_TIMEOUT_MS);
                    Serial.println(F(" мс"));
                } else {
                    Serial.println();
                }
                
                // Проверка на двойной клик
                if (waitingForDoubleClick) {
                    unsigned long timeDiff = currentTime - lastClickTime;
                    if (timeDiff < DOUBLE_CLICK_TIMEOUT_MS) {
                        // Двойной клик обнаружен при втором нажатии
                        doubleClickFlag = true;
                        waitingForDoubleClick = false;
                        // Отменяем одиночный клик, если он еще не был обработан
                        singleClickFlag = false;
                        Serial.print(F("[ATOM_BUTTON] ✓ Двойной клик обнаружен! Интервал: "));
                        Serial.print(timeDiff);
                        Serial.println(F(" мс"));
                    } else {
                        // Прошло слишком много времени, это новый первый клик
                        Serial.print(F("[ATOM_BUTTON] Таймаут первого клика истек ("));
                        Serial.print(timeDiff);
                        Serial.println(F(" мс), регистрируем как новый первый клик"));
                        waitingForDoubleClick = true;
                        lastClickTime = currentTime;
                        singleClickFlag = false;
                        doubleClickFlag = false;
                    }
                } else {
                    // Первый клик, ждем возможный второй
                    waitingForDoubleClick = true;
                    lastClickTime = currentTime;
                    // Сбрасываем флаги, чтобы не было ложных срабатываний
                    singleClickFlag = false;
                    doubleClickFlag = false;
                    Serial.println(F("[ATOM_BUTTON] Первый клик зарегистрирован, ждем второй..."));
                }
            }
            
            // Обработка отпускания (переход с true на false - кнопка отпущена)
            if (!buttonState && previousState) {
                // Кнопка отпущена - ничего не делаем, ждем таймаут или второй клик
                if (waitingForDoubleClick) {
                    Serial.print(F("[ATOM_BUTTON] Кнопка отпущена, ждем второй клик. Прошло: "));
                    Serial.print(millis() - lastClickTime);
                    Serial.println(F(" мс"));
                }
            }
        }
    }
    
    lastButtonState = currentState;
    
    // Проверка таймаута двойного клика (если прошло достаточно времени после первого клика)
    if (waitingForDoubleClick && (millis() - lastClickTime) >= DOUBLE_CLICK_TIMEOUT_MS) {
        // Таймаут - это был одиночный клик
        // Устанавливаем флаг только если двойной клик еще не был обнаружен
        if (!doubleClickFlag) {
            singleClickFlag = true;
            Serial.print(F("[ATOM_BUTTON] Одиночный клик обнаружен (таймаут), прошло: "));
            Serial.print(millis() - lastClickTime);
            Serial.println(F(" мс"));
        } else {
            Serial.println(F("[ATOM_BUTTON] Таймаут, но doubleClickFlag уже установлен, пропускаем"));
        }
        waitingForDoubleClick = false;
    }
}

bool getAtomButtonSingleClick() {
    if (singleClickFlag) {
        singleClickFlag = false;
        Serial.println(F("[ATOM_BUTTON] Одиночный клик обнаружен"));
        return true;
    }
    return false;
}

bool getAtomButtonDoubleClick() {
    if (doubleClickFlag) {
        doubleClickFlag = false;
        // Убеждаемся, что одиночный клик не будет обработан после двойного
        singleClickFlag = false;
        waitingForDoubleClick = false;
        Serial.println(F("[ATOM_BUTTON] Двойной клик возвращен (getAtomButtonDoubleClick)"));
        return true;
    }
    return false;
}

