#include "stream_page.h"
#include "../utils/html_templates.h"
#include "../../../include/web_server.h"
#include "../../../include/camera.h"
#include "../../../include/config.h"
#include "esp_camera.h"

extern WebServer server;

void handleStreamPage() {
    String html = htmlHeader("Трансляция с камеры");
    html += generateNavMenu("stream");
    
    html += "<div class=\"card\">\n";
    html += "<div class=\"card-header\"><i class=\"bi bi-camera-video\"></i> Онлайн трансляция</div>\n";
    html += "<div class=\"card-body\">\n";
    
    if (!isCameraReady()) {
        html += "<div class=\"alert alert-warning\">\n";
        html += "<i class=\"bi bi-exclamation-triangle\"></i> Камера не инициализирована или недоступна.\n";
        html += "</div>\n";
    } else {
        html += "<div class=\"text-center\">\n";
        html += "<img id=\"stream\" src=\"/stream/frame\" alt=\"Трансляция\" style=\"max-width: 100%; height: auto; border-radius: 10px;\">\n";
        html += "</div>\n";
        html += "<script>\n";
        html += "let img = document.getElementById('stream');\n";
        html += "let updateInterval = setInterval(function() {\n";
        html += "    let timestamp = new Date().getTime();\n";
        html += "    img.src = '/stream/frame?t=' + timestamp;\n";
        html += "}, " + String(1000 / CAMERA_FPS) + ");\n";
        html += "img.onerror = function() {\n";
        html += "    clearInterval(updateInterval);\n";
        html += "    this.src = '';\n";
        html += "    this.alt = 'Ошибка загрузки изображения';\n";
        html += "};\n";
        html += "</script>\n";
    }
    
    html += "</div></div>\n";
    html += htmlFooter();
    server.send(200, "text/html", html);
}

void handleStreamFrame() {
    if (!isCameraReady()) {
        server.send(503, "text/plain", "Camera not available");
        return;
    }
    
    size_t len = 0;
    void* fb = getCameraFrame(&len);
    
    if (!fb) {
        server.send(503, "text/plain", "Failed to capture frame");
        return;
    }
    
    // Отправляем JPEG изображение
    camera_fb_t* frame = (camera_fb_t*)fb;
    server.send_P(200, "image/jpeg", (const char*)frame->buf, len);
    
    // Освобождаем буфер после отправки
    releaseCameraFrame(fb);
}

