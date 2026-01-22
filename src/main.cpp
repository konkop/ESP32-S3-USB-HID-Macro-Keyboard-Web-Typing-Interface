
#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

#include "USB.h"
#include "USBHIDKeyboard.h"
#include <ESPmDNS.h>

const int NUM_BUTTONS = 9;
const int debounceDelay = 500; // milliseconds

int buttonPins[NUM_BUTTONS] = {4,5,12,13,14,15,16,17,18};
bool lastButtonState[NUM_BUTTONS];
unsigned long lastDebounceTime[NUM_BUTTONS];
bool typingInProgress = false;

int typingSpeedMs = 80;     // base delay between keystrokes
int errorPercent = 5;      // percent chance of typo

bool keepAliveEnabled = false;
unsigned long lastKeepAliveTime = 0;
const unsigned long KEEP_ALIVE_INTERVAL = 5UL * 60UL * 1000UL; // 5 minutes

// ================= USB HID ================= 

USBHIDKeyboard Keyboard;

// ================= WIFI ================= 

const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

WebServer server(80);

String textBuffer;
bool startTyping = false;

// ================= HTML ================= 

const char* htmlPage = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <title>ESP32 USB HID Keyboard</title>
  <style>
    body { font-family: Arial; }
    textarea { width: 100%; font-size: 16px; }
    .slider-container { margin-top: 10px; }
    label { display: block; margin-top: 10px; }
  </style>

  <script>
    document.addEventListener("DOMContentLoaded", function() {
      const textarea = document.getElementById("inputText");
      const speedSlider = document.getElementById("speed");
      const errorSlider = document.getElementById("error");
      const speedValue = document.getElementById("speedValue");
      const errorValue = document.getElementById("errorValue");

      speedValue.textContent = speedSlider.value;
      errorValue.textContent = errorSlider.value;

      speedSlider.oninput = () => speedValue.textContent = speedSlider.value;
      errorSlider.oninput = () => errorValue.textContent = errorSlider.value;

      textarea.addEventListener("keydown", function(e) {
        if (e.key === "Enter" && !e.shiftKey) {
          e.preventDefault();
          sendText();
        }
      });

      function sendText() {
        const text = textarea.value;
        if (!text || text.trim().length === 0) return;

        const params =
          "text=" + encodeURIComponent(text) +
          "&speed=" + speedSlider.value +
          "&error=" + errorSlider.value;

        var xhr = new XMLHttpRequest();
        xhr.open("POST", "/send", true);
        xhr.setRequestHeader("Content-type", "application/x-www-form-urlencoded");
        xhr.send(params);

        textarea.value = "";
        textarea.focus();
      }
    });
  </script>
</head>

<body>
  <h2>ESP32 USB HID Keyboard</h2>

  <textarea id="inputText" rows="10"
    placeholder="Enter = send | Shift+Enter = newline"></textarea>

  <div class="slider-container">
    <label>
      Typing Speed (ms): <span id="speedValue"></span>
      <input type="range" id="speed" min="20" max="200" value="80">
    </label>

    <label>
      Error %: <span id="errorValue"></span>%
      <input type="range" id="error" min="0" max="20" value="5">
    </label>
  </div>
</body>
</html>
)rawliteral";


// ================= HUMAN TYPING ================= 

void runKeepAlive() {
  Serial.println("KEEP ALIVE: Launching Edge");

  // Win + R
  Keyboard.press(KEY_LEFT_GUI);
  Keyboard.press('r');
  Keyboard.releaseAll();
  delay(300);

  // Type Edge command
  Keyboard.print("msedge https://www.google.com");
  delay(100);

  // Enter
  Keyboard.press(KEY_RETURN);
  Keyboard.release(KEY_RETURN);
}

void humanDelay() {
  int jitter = random(-typingSpeedMs / 3, typingSpeedMs / 3);
  delay(max(10, typingSpeedMs + jitter));
}

void typeChar(char c) {
  Keyboard.print(c);
}

void backspace() {
  Keyboard.press(KEY_BACKSPACE);
  delay(10);
  Keyboard.release(KEY_BACKSPACE);
}

void humanType(const String& text) {
  for (int i = 0; i < text.length(); i++) {
    humanDelay();

    char c = text[i];

    // 5% typo chance only for letters
    if (random(0, 100) < errorPercent && isalpha(c)) {
      Keyboard.print('x');
      delay(random(40, 120));
      Keyboard.press(KEY_BACKSPACE);
      Keyboard.release(KEY_BACKSPACE);
    }

    if (c == '\n') {
      Keyboard.press(KEY_RETURN);
      delay(10);
      Keyboard.release(KEY_RETURN);
    } else {
      typeChar(c);
    }

    // Natural pauses
    if (c == '.' || c == ',')
      delay(random(300, 700));
    if (c == '\n')
      delay(random(500, 1000));
  }
}

// Map buttons to HID actions (example: Ctrl+C, Ctrl+V...)
void triggerAction(int btn) {

  // ANY button press cancels keep-alive
  if (keepAliveEnabled && btn != 3) {
    keepAliveEnabled = false;
    Serial.println("KEEP ALIVE DISABLED (button activity)");
    return;
  }

  switch (btn) {
    case 0: //button 1
      Serial.println("Button 1 pressed: Ctrl+C");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('c');
      delay(50);
      Keyboard.releaseAll();
      break;
    case 1: // Button 2
      Serial.println("Button 2 pressed: Ctrl+V");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('v');
      delay(50);
      Keyboard.releaseAll();
      break;
    case 2:// Button 3
      Serial.println("Button 3 pressed: Ctrl+A (Select All)");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('a');
      delay(50);
      Keyboard.releaseAll();
      break;
    case 3: // BUTTON 4 → KEEP ALIVE Toggle
      keepAliveEnabled = !keepAliveEnabled;
      lastKeepAliveTime = millis();
      Serial.println("KEEP ALIVE changed");
      break;

    case 4: // BUTTON 5 → Alt + tab
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press(KEY_TAB);
      delay(100);
      Keyboard.releaseAll();
      break;
    case 5:
      Serial.println("Button 6 pressed: Ctrl+X (Cut)");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('x');
      delay(50);
      Keyboard.releaseAll();
      break;
    case 6:
      Serial.println("Button 7 pressed: Ctrl+Alt+Delete, wait 2s, Enter");

      // Ctrl + Alt + Delete
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press(KEY_LEFT_ALT);
      Keyboard.press(KEY_DELETE);
      delay(100);
      Keyboard.releaseAll();

      // Wait 2 seconds
      delay(2000);

      // Press Enter
      Keyboard.press(KEY_RETURN);
      delay(50);
      Keyboard.release(KEY_RETURN);

      break;
    case 7:
      Serial.println("Button 8 pressed: Ctrl+S (Save)");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('s');
      delay(50);
      Keyboard.releaseAll();
      break;
    case 8:
      Serial.println("Button 9 pressed: Ctrl+P (Print)");
      Keyboard.press(KEY_LEFT_CTRL);
      Keyboard.press('p');
      delay(50);
      Keyboard.releaseAll();
      break;
  }
}


// ================= WEB HANDLERS ================= 

void handleRoot() {
  server.send(200, "text/html", htmlPage);
}

void handleSend() {
  textBuffer = server.arg("text");

  if (server.hasArg("speed")) {
    typingSpeedMs = server.arg("speed").toInt();
  }

  if (server.hasArg("error")) {
    errorPercent = server.arg("error").toInt();
  }

  Serial.printf(
    "Received text (%d chars), speed=%dms, error=%d%%\n",
    textBuffer.length(),
    typingSpeedMs,
    errorPercent
  );

  startTyping = true;
  server.send(200, "text/plain", "OK");
}

// ================= SETUP ================= 

void setup() {
  delay(1000);

  Serial.begin(115200);       // Start the Serial Monitor
  while (!Serial) { }         // Wait for Serial to be ready (optional)
  
  Serial.println("Booting ESP32-S3...");
  
  // USB HID
  Keyboard.begin();
  USB.begin();

  WiFi.setHostname("human-typing-keyboard");

  //start wifi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi ");

  //keep trying wifi
  int retries = 0;
  while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      retries++;
      if (retries > 20) {  // timeout after 10 seconds
          Serial.println("\nFailed to connect to Wi-Fi!");
          break;
      }
  }

  if (WiFi.status() == WL_CONNECTED) {
      Serial.println("\nWi-Fi Connected!");
      Serial.print("IP Address: ");
      Serial.println(WiFi.localIP());
  }

  //turn on the web server
  server.on("/", handleRoot);
  server.on("/send", HTTP_POST, handleSend);
  server.begin();

  // setup http://human-typing-keyboard.local
  MDNS.begin("human-typing-keyboard");

  for (int i = 0; i < NUM_BUTTONS; i++) {
    pinMode(buttonPins[i], INPUT_PULLUP);
    lastButtonState[i] = HIGH;  // buttons idle HIGH
    lastDebounceTime[i] = 0;
  }

}

// ================= LOOP ================= 

void loop() {
  server.handleClient();

  if (startTyping && !typingInProgress) {
    typingInProgress = true;
    String textToType = textBuffer;  // copy
    startTyping = false;              // reset immediately
    delay(500);                       // optional focus delay
    humanType(textToType);
    typingInProgress = false;
  }

  for (int i = 0; i < NUM_BUTTONS; i++) {
    int reading = digitalRead(buttonPins[i]);

    // If the reading changed, reset the debounce timer
    if (reading == LOW) {
      if ((millis() - lastDebounceTime[i]) > debounceDelay) {
        lastDebounceTime[i] = millis();
        triggerAction(i); // prints to Serial & sends HID keys
      }

    }

    lastButtonState[i] = reading;
  }
  if (keepAliveEnabled) {
    unsigned long now = millis();

    if (now - lastKeepAliveTime >= KEEP_ALIVE_INTERVAL) {
      Serial.println("\nrunning browser");
      lastKeepAliveTime = now;
      runKeepAlive();
    }
  }
}
