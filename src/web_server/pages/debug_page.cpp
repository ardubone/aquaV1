// debug_page.cpp
#include "debug_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/config.h"

extern WebServer server;

void handleDebugPage() {
    String html = htmlHeader("Дебаг");
    html += generateNavMenu("debug");
    
    // Вкладки для переключения между секциями
    html += "<ul class=\"nav nav-tabs mb-3\" id=\"debugTabs\" role=\"tablist\">\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link active\" id=\"pcf8574-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#pcf8574\" type=\"button\" role=\"tab\"><i class=\"bi bi-cpu\"></i> PCF8574</button>\n";
    html += "</li>\n";
    html += "<li class=\"nav-item\" role=\"presentation\">\n";
    html += "<button class=\"nav-link\" id=\"temperature-tab\" data-bs-toggle=\"tab\" data-bs-target=\"#temperature\" type=\"button\" role=\"tab\"><i class=\"bi bi-thermometer-half\"></i> Датчики температуры</button>\n";
    html += "</li>\n";
    html += "</ul>\n";
    
    html += "<div class=\"tab-content\" id=\"debugTabContent\">\n";
    
    // Секция PCF8574
    html += "<div class=\"tab-pane fade show active\" id=\"pcf8574\" role=\"tabpanel\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header bg-warning text-dark\"><i class=\"bi bi-cpu\"></i> Дебаг PCF8574</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped\">\n";
    html += "<thead class=\"table-dark\">\n";
    html += "<tr>\n";
    html += "<th>Пин</th>\n";
    html += "<th>Название</th>\n";
    html += "<th>Тип</th>\n";
    html += "<th>Состояние</th>\n";
    html += "<th>Управление</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"pinsTableBody\">\n";
    html += "<tr><td colspan=\"5\" class=\"text-center\">Загрузка...</td></tr>\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n";
    
    html += "</div></div></div>\n";
    
    // Секция датчиков температуры
    html += "<div class=\"tab-pane fade\" id=\"temperature\" role=\"tabpanel\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header bg-info text-dark\"><i class=\"bi bi-thermometer-half\"></i> Датчики температуры DS18B20</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Текущие назначения
    html += "<div class=\"row mb-3\">\n";
    html += "<div class=\"col-md-6\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\">Текущие назначения</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p><strong>Tank20:</strong> <span id=\"tank20Address\">Загрузка...</span></p>\n";
    html += "<p><strong>Tank10:</strong> <span id=\"tank10Address\">Загрузка...</span></p>\n";
    html += "</div></div></div>\n";
    html += "<div class=\"col-md-6\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\">Текущие температуры</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<p><strong>Tank20:</strong> <span id=\"tank20Temp\">-</span> °C</p>\n";
    html += "<p><strong>Tank10:</strong> <span id=\"tank10Temp\">-</span> °C</p>\n";
    html += "</div></div></div>\n";
    html += "</div>\n";
    
    // Форма редактирования
    html += "<div class=\"card mb-3\">\n";
    html += "<div class=\"card-header\">Назначить датчики</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<div class=\"row\">\n";
    html += "<div class=\"col-md-6 mb-3\">\n";
    html += "<label for=\"tank20Select\" class=\"form-label\">Tank20:</label>\n";
    html += "<select class=\"form-select\" id=\"tank20Select\">\n";
    html += "<option value=\"\">Выберите датчик...</option>\n";
    html += "</select>\n";
    html += "</div>\n";
    html += "<div class=\"col-md-6 mb-3\">\n";
    html += "<label for=\"tank10Select\" class=\"form-label\">Tank10:</label>\n";
    html += "<select class=\"form-select\" id=\"tank10Select\">\n";
    html += "<option value=\"\">Выберите датчик...</option>\n";
    html += "</select>\n";
    html += "</div>\n";
    html += "</div>\n";
    html += "<button class=\"btn btn-primary\" onclick=\"saveSensorAddresses()\"><i class=\"bi bi-save\"></i> Сохранить</button>\n";
    html += "<button class=\"btn btn-secondary ms-2\" onclick=\"scanSensors()\"><i class=\"bi bi-arrow-clockwise\"></i> Сканировать датчики</button>\n";
    html += "</div></div>\n";
    
    // Таблица всех найденных датчиков
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\">Все найденные датчики</div>\n";
    html += "<div class=\"card-body\">\n";
    html += "<div class=\"table-responsive\">\n";
    html += "<table class=\"table table-striped\">\n";
    html += "<thead class=\"table-dark\">\n";
    html += "<tr>\n";
    html += "<th>Адрес</th>\n";
    html += "<th>Температура</th>\n";
    html += "<th>Назначение</th>\n";
    html += "</tr>\n";
    html += "</thead>\n";
    html += "<tbody id=\"sensorsTableBody\">\n";
    html += "<tr><td colspan=\"3\" class=\"text-center\">Загрузка...</td></tr>\n";
    html += "</tbody>\n";
    html += "</table>\n";
    html += "</div>\n";
    html += "</div></div>\n";
    
    html += "</div></div></div>\n";
    html += "</div>\n"; // закрываем tab-content
    
    // JavaScript
    html += "<script>\n";
    
    // Функции для PCF8574
    html += "function updatePinsStatus() {\n";
    html += "  fetch('/debug/pcf8574/status')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      const tbody = document.getElementById('pinsTableBody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      \n";
    html += "      data.pins.forEach(pin => {\n";
    html += "        const row = tbody.insertRow();\n";
    html += "        \n";
    html += "        const cellPin = row.insertCell(0);\n";
    html += "        cellPin.textContent = pin.pin;\n";
    html += "        \n";
    html += "        const cellName = row.insertCell(1);\n";
    html += "        cellName.textContent = pin.name;\n";
    html += "        \n";
    html += "        const cellType = row.insertCell(2);\n";
    html += "        cellType.innerHTML = pin.isOutput ? '<span class=\"badge bg-primary\">Выход</span>' : '<span class=\"badge bg-secondary\">Вход</span>';\n";
    html += "        \n";
    html += "        const cellState = row.insertCell(3);\n";
    html += "        const stateBadge = pin.state ? '<span class=\"badge bg-success\">HIGH</span>' : '<span class=\"badge bg-danger\">LOW</span>';\n";
    html += "        cellState.innerHTML = stateBadge;\n";
    html += "        \n";
    html += "        const cellControl = row.insertCell(4);\n";
    html += "        if (pin.isOutput) {\n";
    html += "          const btnOn = pin.state ? '' : '<button class=\"btn btn-sm btn-success me-2\" onclick=\"setPin(' + pin.pin + ', true)\"><i class=\"bi bi-power\"></i> ВКЛ</button>';\n";
    html += "          const btnOff = pin.state ? '<button class=\"btn btn-sm btn-danger\" onclick=\"setPin(' + pin.pin + ', false)\"><i class=\"bi bi-power\"></i> ВЫКЛ</button>' : '';\n";
    html += "          cellControl.innerHTML = btnOn + btnOff;\n";
    html += "        } else {\n";
    html += "          cellControl.innerHTML = '<span class=\"text-muted\">Только чтение</span>';\n";
    html += "        }\n";
    html += "      });\n";
    html += "    })\n";
    html += "    .catch(error => {\n";
    html += "      console.error('Error:', error);\n";
    html += "      document.getElementById('pinsTableBody').innerHTML = '<tr><td colspan=\"5\" class=\"text-center text-danger\">Ошибка загрузки данных</td></tr>';\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "function setPin(pin, state) {\n";
    html += "  const formData = new URLSearchParams();\n";
    html += "  formData.append('pin', pin);\n";
    html += "  formData.append('state', state ? 'true' : 'false');\n";
    html += "  \n";
    html += "  fetch('/debug/pcf8574/setpin', {\n";
    html += "    method: 'POST',\n";
    html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n";
    html += "    body: formData\n";
    html += "  })\n";
    html += "  .then(response => response.json())\n";
    html += "  .then(data => {\n";
    html += "    if (data.success) {\n";
    html += "      updatePinsStatus();\n";
    html += "    } else {\n";
    html += "      alert('Ошибка: ' + (data.error || 'Неизвестная ошибка'));\n";
    html += "    }\n";
    html += "  })\n";
    html += "  .catch(error => {\n";
    html += "    console.error('Error:', error);\n";
    html += "    alert('Ошибка при установке состояния пина');\n";
    html += "  });\n";
    html += "}\n";
    html += "\n";
    
    // Функции для датчиков температуры
    html += "function formatAddress(address) {\n";
    html += "  return '0x' + address.map(b => ('0' + (b & 0xFF).toString(16)).slice(-2)).join(' ');\n";
    html += "}\n";
    html += "\n";
    html += "function addressToString(address) {\n";
    html += "  return address.map(b => ('0' + (b & 0xFF).toString(16)).slice(-2)).join('');\n";
    html += "}\n";
    html += "\n";
    html += "function stringToAddress(str) {\n";
    html += "  const bytes = [];\n";
    html += "  for (let i = 0; i < str.length; i += 2) {\n";
    html += "    bytes.push(parseInt(str.substr(i, 2), 16));\n";
    html += "  }\n";
    html += "  return bytes;\n";
    html += "}\n";
    html += "\n";
    html += "function updateTemperatureStatus() {\n";
    html += "  fetch('/debug/temperature/status')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      // Обновляем текущие назначения\n";
    html += "      document.getElementById('tank20Address').textContent = formatAddress(data.tank20Address);\n";
    html += "      document.getElementById('tank10Address').textContent = formatAddress(data.tank10Address);\n";
    html += "      \n";
    html += "      // Обновляем температуры\n";
    html += "      document.getElementById('tank20Temp').textContent = data.tank20Temp.toFixed(1);\n";
    html += "      document.getElementById('tank10Temp').textContent = data.tank10Temp.toFixed(1);\n";
    html += "      \n";
    html += "      // Обновляем выпадающие списки\n";
    html += "      const tank20Select = document.getElementById('tank20Select');\n";
    html += "      const tank10Select = document.getElementById('tank10Select');\n";
    html += "      \n";
    html += "      tank20Select.innerHTML = '<option value=\"\">Выберите датчик...</option>';\n";
    html += "      tank10Select.innerHTML = '<option value=\"\">Выберите датчик...</option>';\n";
    html += "      \n";
    html += "      data.sensors.forEach(sensor => {\n";
    html += "        const addrStr = addressToString(sensor.address);\n";
    html += "        const option20 = document.createElement('option');\n";
    html += "        option20.value = addrStr;\n";
    html += "        option20.textContent = formatAddress(sensor.address) + ' (' + sensor.temp.toFixed(1) + '°C)';\n";
    html += "        if (addressToString(data.tank20Address) === addrStr) option20.selected = true;\n";
    html += "        tank20Select.appendChild(option20);\n";
    html += "        \n";
    html += "        const option10 = document.createElement('option');\n";
    html += "        option10.value = addrStr;\n";
    html += "        option10.textContent = formatAddress(sensor.address) + ' (' + sensor.temp.toFixed(1) + '°C)';\n";
    html += "        if (addressToString(data.tank10Address) === addrStr) option10.selected = true;\n";
    html += "        tank10Select.appendChild(option10);\n";
    html += "      });\n";
    html += "      \n";
    html += "      // Обновляем таблицу датчиков\n";
    html += "      const tbody = document.getElementById('sensorsTableBody');\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      \n";
    html += "      data.sensors.forEach(sensor => {\n";
    html += "        const row = tbody.insertRow();\n";
    html += "        const cellAddr = row.insertCell(0);\n";
    html += "        cellAddr.textContent = formatAddress(sensor.address);\n";
    html += "        \n";
    html += "        const cellTemp = row.insertCell(1);\n";
    html += "        cellTemp.textContent = sensor.temp.toFixed(1) + ' °C';\n";
    html += "        \n";
    html += "        const cellAssign = row.insertCell(2);\n";
    html += "        const addrStr = addressToString(sensor.address);\n";
    html += "        let assignment = '';\n";
    html += "        if (addressToString(data.tank20Address) === addrStr) assignment += '<span class=\"badge bg-primary\">Tank20</span> ';\n";
    html += "        if (addressToString(data.tank10Address) === addrStr) assignment += '<span class=\"badge bg-success\">Tank10</span>';\n";
    html += "        cellAssign.innerHTML = assignment || '<span class=\"text-muted\">Не назначен</span>';\n";
    html += "      });\n";
    html += "    })\n";
    html += "    .catch(error => {\n";
    html += "      console.error('Error:', error);\n";
    html += "      document.getElementById('sensorsTableBody').innerHTML = '<tr><td colspan=\"3\" class=\"text-center text-danger\">Ошибка загрузки данных</td></tr>';\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "function saveSensorAddresses() {\n";
    html += "  const tank20Addr = document.getElementById('tank20Select').value;\n";
    html += "  const tank10Addr = document.getElementById('tank10Select').value;\n";
    html += "  \n";
    html += "  if (!tank20Addr || !tank10Addr) {\n";
    html += "    alert('Выберите датчики для обоих резервуаров');\n";
    html += "    return;\n";
    html += "  }\n";
    html += "  \n";
    html += "  const formData = new URLSearchParams();\n";
    html += "  formData.append('tank20', tank20Addr);\n";
    html += "  formData.append('tank10', tank10Addr);\n";
    html += "  \n";
    html += "  fetch('/debug/temperature/setaddress', {\n";
    html += "    method: 'POST',\n";
    html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n";
    html += "    body: formData\n";
    html += "  })\n";
    html += "  .then(response => response.json())\n";
    html += "  .then(data => {\n";
    html += "    if (data.success) {\n";
    html += "      alert('Адреса датчиков сохранены');\n";
    html += "      updateTemperatureStatus();\n";
    html += "    } else {\n";
    html += "      alert('Ошибка: ' + (data.error || 'Неизвестная ошибка'));\n";
    html += "    }\n";
    html += "  })\n";
    html += "  .catch(error => {\n";
    html += "    console.error('Error:', error);\n";
    html += "    alert('Ошибка при сохранении адресов');\n";
    html += "  });\n";
    html += "}\n";
    html += "\n";
    html += "function scanSensors() {\n";
    html += "  updateTemperatureStatus();\n";
    html += "}\n";
    html += "\n";
    html += "// Обновляем состояние PCF8574 каждые 500мс\n";
    html += "setInterval(updatePinsStatus, 500);\n";
    html += "// Обновляем состояние датчиков каждые 5 секунд\n";
    html += "updateTemperatureStatus();\n";
    html += "setInterval(updateTemperatureStatus, 5000);\n";
    html += "</script>\n";
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}

