// debug_pcf8574_page.cpp
#include "debug_pcf8574_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/config.h"

extern WebServer server;

void handleDebugPcf8574Page() {
    String html = htmlHeader("Дебаг PCF8574");
    html += generateNavMenu("debug_pcf8574");
    
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header bg-warning text-dark\"><i class=\"bi bi-tools\"></i> Дебаг PCF8574</div>\n";
    html += "<div class=\"card-body\">\n";
    
    html += "<div class=\"alert alert-info\">\n";
    html += "<i class=\"bi bi-info-circle\"></i> Эта страница доступна только в DEBUG_MODE. Используйте для тестирования пинов PCF8574.\n";
    html += "</div>\n";
    
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
    
    html += "</div></div>\n";
    
    // JavaScript для обновления состояния и управления
    html += "<script>\n";
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
    html += "        // Пин\n";
    html += "        const cellPin = row.insertCell(0);\n";
    html += "        cellPin.textContent = pin.pin;\n";
    html += "        \n";
    html += "        // Название\n";
    html += "        const cellName = row.insertCell(1);\n";
    html += "        cellName.textContent = pin.name;\n";
    html += "        \n";
    html += "        // Тип\n";
    html += "        const cellType = row.insertCell(2);\n";
    html += "        cellType.innerHTML = pin.isOutput ? '<span class=\"badge bg-primary\">Выход</span>' : '<span class=\"badge bg-secondary\">Вход</span>';\n";
    html += "        \n";
    html += "        // Состояние\n";
    html += "        const cellState = row.insertCell(3);\n";
    html += "        const stateBadge = pin.state ? '<span class=\"badge bg-success\">HIGH</span>' : '<span class=\"badge bg-danger\">LOW</span>';\n";
    html += "        cellState.innerHTML = stateBadge;\n";
    html += "        \n";
    html += "        // Управление\n";
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
    html += "// Обновляем состояние каждые 500мс\n";
    html += "updatePinsStatus();\n";
    html += "setInterval(updatePinsStatus, 500);\n";
    html += "</script>\n";
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}

