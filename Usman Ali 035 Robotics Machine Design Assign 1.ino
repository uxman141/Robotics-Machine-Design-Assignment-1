
#include <WiFi.h>
#include <WebServer.h>
#include <Adafruit_NeoPixel.h>

#define LED_PIN 48
#define NUM_LEDS 1

Adafruit_NeoPixel led(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

const char* ssid = "RGB-TEST";
const char* password = "12345678";

WebServer server(80);
bool robotRunning = false;

void setRobotState(bool running) {
  robotRunning = running;

  if (running) {
    led.setPixelColor(0, led.Color(0, 255, 0));
    Serial.println("ROBOT OPERATIONAL - GREEN LED");
  } else {
    led.setPixelColor(0, led.Color(255, 0, 0));
    Serial.println("SAFETY HALT - RED LED");
  }

  led.show();
}

const char webpage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1">
<title>Usman | Robot Control</title>

<style>
* {
  box-sizing: border-box;
}

body {
  margin: 0;
  min-height: 100vh;
  background: #f1f3f6;
  color: #202938;
  font-family: Arial, sans-serif;
  display: flex;
  align-items: center;
  justify-content: center;
  padding: 20px;
}

.card {
  width: 100%;
  max-width: 470px;
  padding: 35px 26px;
  background: #ffffff;
  border-radius: 22px;
  box-shadow: 0 15px 50px #1f29371c;
  border: 1px solid #e5e7eb;
  text-align: center;
}

.top-label {
  display: inline-block;
  padding: 9px 17px;
  border-radius: 30px;
  background: #fff0df;
  color: #e87500;
  font-size: 12px;
  font-weight: bold;
  letter-spacing: 1.5px;
}

h1 {
  margin: 22px 0 8px;
  font-size: 29px;
  color: #1f2937;
}

.subtitle {
  color: #64748b;
  font-size: 14px;
}

.student {
  margin-top: 25px;
  padding: 20px;
  background: #fff7ed;
  border: 1px solid #fed7aa;
  border-radius: 14px;
  color: #c2410c;
  font-size: 23px;
  font-weight: bold;
  letter-spacing: 1px;
}

.student-label {
  display: block;
  font-size: 11px;
  color: #9a5b35;
  font-weight: normal;
  margin-bottom: 10px;
  letter-spacing: 2px;
}

.status-box {
  margin: 25px 0;
  padding: 25px 12px;
  background: #f8fafc;
  border: 1px solid #e2e8f0;
  border-radius: 14px;
}

.status-label {
  font-size: 12px;
  color: #64748b;
  letter-spacing: 1.5px;
  margin-bottom: 15px;
}

.status {
  font-size: 19px;
  font-weight: bold;
}

.green {
  color: #15803d;
}

.red {
  color: #dc2626;
}

button {
  display: block;
  width: 100%;
  border: none;
  padding: 18px;
  margin: 13px 0;
  border-radius: 11px;
  font-size: 16px;
  font-weight: bold;
  cursor: pointer;
  transition: 0.2s;
}

.start {
  background: #f97316;
  color: white;
  box-shadow: 0 5px 15px #f9731633;
}

.stop {
  background: #293241;
  color: white;
}

button:active {
  transform: scale(0.97);
}

#message {
  color: #ea580c;
  font-size: 13px;
  min-height: 20px;
}

.footer {
  margin-top: 25px;
  padding-top: 20px;
  border-top: 1px solid #e5e7eb;
  font-size: 12px;
  color: #94a3b8;
}
</style>
</head>

<body>

<div class="card">

<div class="top-label">
ESP32-S3 CONTROL PANEL
</div>

<h1>ROBOT CONTROL</h1>

<p class="subtitle">
Smart Wireless Robot Monitoring
</p>

<div class="student">
<span class="student-label">DEVELOPED BY</span>
USMAN
</div>

<div class="status-box">
<div class="status-label">CURRENT ROBOT STATUS</div>
<div id="status" class="status">Loading...</div>
</div>

<button class="start" onclick="control(1)">
START ROBOT
</button>

<button class="stop" onclick="control(0)">
SAFETY HALT
</button>

<p id="message"></p>

<div class="footer">
ESP32-S3 | Local Web Server | Port 80
</div>

</div>

<script>
async function refreshStatus() {
  try {
    const response = await fetch('/robot/status');

    if (!response.ok) {
      throw new Error('Server Error');
    }

    const data = await response.json();
    const status = document.getElementById('status');

    if (data.running) {
      status.textContent = '● OPERATIONAL - GREEN LED';
      status.className = 'status green';
    } else {
      status.textContent = '● SAFETY HALT - RED LED';
      status.className = 'status red';
    }

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

async function control(state) {
  try {
    const response = await fetch(
      '/robot/status?state=' + state
    );

    if (!response.ok) {
      throw new Error('Command Failed');
    }

    document.getElementById('message').textContent =
      'Command Successful';

    await refreshStatus();

  } catch (error) {
    document.getElementById('message').textContent =
      'Network Error';
  }
}

refreshStatus();
setInterval(refreshStatus, 2000);
</script>

</body>
</html>
)rawliteral";

void handleRobotStatus() {
  if (server.hasArg("state")) {
    String state = server.arg("state");

    if (state == "1") {
      setRobotState(true);
    } else if (state == "0") {
      setRobotState(false);
    } else {
      server.send(
        400,
        "application/json",
        "{\"error\":\"Invalid state\"}"
      );
      return;
    }
  }

  String response = "{\"running\":";
  response += robotRunning ? "true" : "false";
  response += "}";

  server.send(200, "application/json", response);
}

void setup() {
  Serial.begin(115200);

  led.begin();
  led.setBrightness(50);
  setRobotState(false);

  WiFi.mode(WIFI_AP);

  bool started = WiFi.softAP(
    ssid,
    password,
    1,
    0,
    4
  );

  if (!started) {
    Serial.println("WiFi Access Point Failed");
    return;
  }

  Serial.println();
  Serial.println("================================");
  Serial.println("ESP32 ROBOT CONTROL");
  Serial.println("Name: Usman");
  Serial.println("================================");

  Serial.println("WiFi Access Point Started");

  Serial.print("Network Name: ");
  Serial.println(ssid);

  Serial.print("IP Address: ");
  Serial.println(WiFi.softAPIP());

  server.on("/", HTTP_GET, []() {
    server.send_P(200, "text/html", webpage);
  });

  server.on(
    "/robot/status",
    HTTP_GET,
    handleRobotStatus
  );

  server.onNotFound([]() {
    server.send(
      404,
      "text/plain",
      "Page Not Found"
    );
  });

  server.begin();

  Serial.println("HTTP SERVER STARTED ON PORT 80");
  Serial.println("Open: http://192.168.4.1");
}

void loop() {
  server.handleClient();
}
