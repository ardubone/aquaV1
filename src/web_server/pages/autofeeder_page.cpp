// autofeeder_page.cpp
#include "autofeeder_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/autofeeder.h"

extern WebServer server;

// Вспомогательная функция для генерации секции кормушки
String generateFeederSection(const char* feederName, const char* feederId) {
    String html = "<div class=\"col-md-6 mb-4\">\n";
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header bg-primary text-white\"><i class=\"bi bi-egg\"></i> Кормушка " + String(feederName) + "</div>\n";
    html += "<div class=\"card-body\">\n";
    
    // Статус и кнопка активации
    html += "<div class=\"mb-3\">\n";
    html += "<p>Состояние реле: <span id=\"status" + String(feederId) + "\" class=\"badge bg-secondary\">Загрузка...</span></p>\n";
    html += "<button class=\"btn btn-primary\" onclick=\"activateFeeder('" + String(feederId) + "')\"><i class=\"bi bi-play-circle\"></i> Ручное кормление</button>\n";
    html += "</div>\n";
    
    // Календарь
    html += "<div class=\"mb-3\">\n";
    html += "<h5><i class=\"bi bi-calendar-check\"></i> Расписание</h5>\n";
    html += "<div id=\"schedule" + String(feederId) + "\" class=\"table-responsive\">\n";
    html += "<table class=\"table table-sm\">\n";
    html += "<thead><tr><th>Время</th><th>День</th><th>Повтор</th><th></th></tr></thead>\n";
    html += "<tbody id=\"scheduleBody" + String(feederId) + "\">\n";
    html += "<tr><td colspan=\"4\" class=\"text-center\">Загрузка...</td></tr>\n";
    html += "</tbody></table></div>\n";
    html += "<button class=\"btn btn-sm btn-success\" onclick=\"showAddSchedule('" + String(feederId) + "')\"><i class=\"bi bi-plus\"></i> Добавить</button>\n";
    html += "</div>\n";
    
    // Логи
    html += "<div class=\"mb-3\">\n";
    html += "<h5><i class=\"bi bi-clock-history\"></i> Логи кормления</h5>\n";
    html += "<div class=\"table-responsive\" style=\"max-height: 300px; overflow-y: auto;\">\n";
    html += "<table class=\"table table-sm table-striped\">\n";
    html += "<thead class=\"table-dark sticky-top\"><tr><th>Дата/Время</th><th>Тип</th><th>Вкл</th><th>Выкл</th><th>Концевик</th></tr></thead>\n";
    html += "<tbody id=\"logsBody" + String(feederId) + "\">\n";
    html += "<tr><td colspan=\"5\" class=\"text-center\">Загрузка...</td></tr>\n";
    html += "</tbody></table></div></div>\n";
    
    html += "</div></div></div>\n";
    return html;
}

void handleAutoFeederPage() {
    String html = htmlHeader("Автокормушки");
    html += generateNavMenu("autofeeder");
    
    html += "<div class=\"row\">\n";
    html += generateFeederSection("Tank10", "tank10");
    html += generateFeederSection("Tank20", "tank20");
    html += "</div>\n"; // row
    
    // Модальное окно для добавления расписания
    html += "<div class=\"modal fade\" id=\"addScheduleModal\" tabindex=\"-1\">\n";
    html += "<div class=\"modal-dialog\">\n";
    html += "<div class=\"modal-content\">\n";
    html += "<div class=\"modal-header\">\n";
    html += "<h5 class=\"modal-title\">Добавить расписание</h5>\n";
    html += "<button type=\"button\" class=\"btn-close\" data-bs-dismiss=\"modal\"></button>\n";
    html += "</div>\n";
    html += "<div class=\"modal-body\">\n";
    html += "<form id=\"scheduleForm\">\n";
    html += "<input type=\"hidden\" id=\"feederId\" value=\"\">\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Час (0-23)</label>\n";
    html += "<input type=\"number\" class=\"form-control\" id=\"scheduleHour\" min=\"0\" max=\"23\" value=\"9\" required>\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Минута (0-59)</label>\n";
    html += "<input type=\"number\" class=\"form-control\" id=\"scheduleMinute\" min=\"0\" max=\"59\" value=\"0\" required>\n";
    html += "</div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">День недели</label>\n";
    html += "<select class=\"form-select\" id=\"scheduleDay\" required>\n";
    html += "<option value=\"0\">Воскресенье</option>\n";
    html += "<option value=\"1\">Понедельник</option>\n";
    html += "<option value=\"2\">Вторник</option>\n";
    html += "<option value=\"3\">Среда</option>\n";
    html += "<option value=\"4\">Четверг</option>\n";
    html += "<option value=\"5\">Пятница</option>\n";
    html += "<option value=\"6\">Суббота</option>\n";
    html += "</select></div>\n";
    html += "<div class=\"mb-3\">\n";
    html += "<label class=\"form-label\">Количество повторений</label>\n";
    html += "<input type=\"number\" class=\"form-control\" id=\"scheduleRepeat\" min=\"1\" max=\"10\" value=\"1\" required>\n";
    html += "</div></form></div>\n";
    html += "<div class=\"modal-footer\">\n";
    html += "<button type=\"button\" class=\"btn btn-secondary\" data-bs-dismiss=\"modal\">Отмена</button>\n";
    html += "<button type=\"button\" class=\"btn btn-primary\" onclick=\"addSchedule()\">Добавить</button>\n";
    html += "</div></div></div></div>\n";
    
    // JavaScript
    html += "<script>\n";
    html += "let currentFeederId = '';\n";
    html += "\n";
    html += "function showAddSchedule(feederId) {\n";
    html += "  currentFeederId = feederId;\n";
    html += "  document.getElementById('feederId').value = feederId;\n";
    html += "  const modal = new bootstrap.Modal(document.getElementById('addScheduleModal'));\n";
    html += "  modal.show();\n";
    html += "}\n";
    html += "\n";
    html += "function addSchedule() {\n";
    html += "  const feederId = document.getElementById('feederId').value;\n";
    html += "  const hour = document.getElementById('scheduleHour').value;\n";
    html += "  const minute = document.getElementById('scheduleMinute').value;\n";
    html += "  const dayOfWeek = document.getElementById('scheduleDay').value;\n";
    html += "  const repeatCount = document.getElementById('scheduleRepeat').value;\n";
    html += "  \n";
    html += "  const formData = new URLSearchParams();\n";
    html += "  formData.append('hour', hour);\n";
    html += "  formData.append('minute', minute);\n";
    html += "  formData.append('dayOfWeek', dayOfWeek);\n";
    html += "  formData.append('repeatCount', repeatCount);\n";
    html += "  \n";
    html += "  fetch('/autofeeder/' + feederId + '/schedule/add', {\n";
    html += "    method: 'POST',\n";
    html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n";
    html += "    body: formData\n";
    html += "  })\n";
    html += "  .then(response => response.json())\n";
    html += "  .then(data => {\n";
    html += "    if (data.success) {\n";
    html += "      bootstrap.Modal.getInstance(document.getElementById('addScheduleModal')).hide();\n";
    html += "      loadSchedule(feederId);\n";
    html += "    }\n";
    html += "  });\n";
    html += "}\n";
    html += "\n";
    html += "function removeSchedule(feederId, index) {\n";
    html += "  const formData = new URLSearchParams();\n";
    html += "  formData.append('index', index);\n";
    html += "  \n";
    html += "  fetch('/autofeeder/' + feederId + '/schedule/remove', {\n";
    html += "    method: 'POST',\n";
    html += "    headers: { 'Content-Type': 'application/x-www-form-urlencoded' },\n";
    html += "    body: formData\n";
    html += "  })\n";
    html += "  .then(response => response.json())\n";
    html += "  .then(data => {\n";
    html += "    if (data.success) {\n";
    html += "      loadSchedule(feederId);\n";
    html += "    }\n";
    html += "  });\n";
    html += "}\n";
    html += "\n";
    html += "function loadSchedule(feederId) {\n";
    html += "  fetch('/autofeeder/' + feederId + '/schedule/get')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      const tbody = document.getElementById('scheduleBody' + feederId);\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      \n";
    html += "      if (data.length === 0) {\n";
    html += "        tbody.innerHTML = '<tr><td colspan=\"4\" class=\"text-center text-muted\">Нет расписаний</td></tr>';\n";
    html += "        return;\n";
    html += "      }\n";
    html += "      \n";
    html += "      const days = ['Вс', 'Пн', 'Вт', 'Ср', 'Чт', 'Пт', 'Сб'];\n";
    html += "      data.forEach((sched, index) => {\n";
    html += "        const row = tbody.insertRow();\n";
    html += "        row.innerHTML = '<td>' + String(sched.hour).padStart(2, '0') + ':' + String(sched.minute).padStart(2, '0') + '</td>' +\n";
    html += "                        '<td>' + days[sched.dayOfWeek] + '</td>' +\n";
    html += "                        '<td>' + sched.repeatCount + '</td>' +\n";
    html += "                        '<td><button class=\"btn btn-sm btn-danger\" onclick=\"removeSchedule(\\'' + feederId + '\\', ' + index + ')\"><i class=\"bi bi-trash\"></i></button></td>';\n";
    html += "      });\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "function loadLogs(feederId) {\n";
    html += "  fetch('/autofeeder/' + feederId + '/logs')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      const tbody = document.getElementById('logsBody' + feederId);\n";
    html += "      tbody.innerHTML = '';\n";
    html += "      \n";
    html += "      if (data.length === 0) {\n";
    html += "        tbody.innerHTML = '<tr><td colspan=\"5\" class=\"text-center text-muted\">Нет логов</td></tr>';\n";
    html += "        return;\n";
    html += "      }\n";
    html += "      \n";
    html += "      data.forEach(log => {\n";
    html += "        const row = tbody.insertRow();\n";
    html += "        const onTime = log.relayOnTime ? log.relayOnTime.split(' ')[1] : '-';\n";
    html += "        const offTime = log.relayOffTime ? log.relayOffTime.split(' ')[1] : '-';\n";
    html += "        const limitTime = log.limitTriggered && log.limitTriggerTime ? log.limitTriggerTime.split(' ')[1] : '-';\n";
    html += "        row.innerHTML = '<td>' + log.timestamp.split(' ')[1] + '</td>' +\n";
    html += "                        '<td>' + log.type + '</td>' +\n";
    html += "                        '<td>' + onTime + '</td>' +\n";
    html += "                        '<td>' + offTime + '</td>' +\n";
    html += "                        '<td>' + (log.limitTriggered ? 'Да (' + limitTime + ')' : 'Нет') + '</td>';\n";
    html += "      });\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "function updateStatus(feederId) {\n";
    html += "  fetch('/autofeeder/' + feederId + '/status')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      const statusEl = document.getElementById('status' + feederId);\n";
    html += "      statusEl.className = 'badge ' + (data.relayOn ? 'bg-success' : 'bg-danger');\n";
    html += "      statusEl.textContent = data.relayOn ? 'ВКЛ' : 'ВЫКЛ';\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "function activateFeeder(feederId) {\n";
    html += "  fetch('/autofeeder/' + feederId + '/activate')\n";
    html += "    .then(response => response.json())\n";
    html += "    .then(data => {\n";
    html += "      if (data.success) {\n";
    html += "        updateStatus(feederId);\n";
    html += "      }\n";
    html += "    });\n";
    html += "}\n";
    html += "\n";
    html += "// Загрузка данных при открытии страницы\n";
    html += "['tank10', 'tank20'].forEach(feederId => {\n";
    html += "  loadSchedule(feederId);\n";
    html += "  loadLogs(feederId);\n";
    html += "  updateStatus(feederId);\n";
    html += "  setInterval(() => {\n";
    html += "    loadLogs(feederId);\n";
    html += "    updateStatus(feederId);\n";
    html += "  }, 5000);\n";
    html += "});\n";
    html += "</script>\n";
    
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleAutoFeederDebug() {
    // Удалена функция дебага, так как она больше не нужна
    handleAutoFeederPage();
}
