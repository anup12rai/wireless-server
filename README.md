# ESP8266 Wi-Fi Manager (Flask UI + ESP8266)

## How it works
- ESP8266 creates a secured setup hotspot `ESP8266_Config`.
- Your computer connects to that hotspot.
- Flask serves the dashboard at `http://127.0.0.1:5000`.
- Browser JavaScript sends the SSID/password directly to the ESP8266 at `http://192.168.4.1/config`.
- ESP saves credentials in LittleFS and restarts. The setup AP remains available for recovery.

## 1) Flash ESP8266
1. Arduino IDE: select your NodeMCU/ESP8266 board and install ESP8266 board support.
2. Install **ArduinoJson** library.
3. Open `esp8266_wifi_manager.ino`, change `AP_PASSWORD` to a strong password (8+ characters), then upload.
4. Open Serial Monitor at 115200 baud.
5. Connect your PC to Wi-Fi hotspot `ESP8266_Config`, password `configure123` (unless you changed it).
   If prompted that this network has no internet, choose stay connected.

## 2) Run Python UI
Install Python 3.10+ and in this project folder run:

```bash
python -m venv .venv
# Windows:
.venv\\Scripts\\activate
# macOS/Linux:
source .venv/bin/activate

pip install -r requirements.txt
python app.py
```

Open `http://127.0.0.1:5000`.

## Important browser note
Modern browsers may block a localhost page from calling an HTTP device due to mixed-content/CORS rules. If the dashboard reports a browser network error, use the ESP8266's own setup page/API directly at `http://192.168.4.1`, or host the UI from the ESP8266. The Python status route is for status checking; the credential POST is intentionally direct from the browser to the ESP8266.

## Security
- Change the default setup AP password before deploying.
- Use only on your own device/network.
- The Wi-Fi password is sent over local HTTP, not encrypted. Configure only on a trusted local network/setup hotspot.
- Do not expose the ESP8266 setup endpoint to the public internet.
- This firmware stores the password in LittleFS. Physical access to the device may expose it.
