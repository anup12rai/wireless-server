
from flask import Flask, render_template, request, jsonify
import requests

app = Flask(__name__)

ESP_IP = "192.168.4.1"
ESP_URL = f"http://{ESP_IP}"


@app.route("/")
def home():
    return render_template("index.html")


# Send configuration to ESP8266
@app.route("/api/config", methods=["POST"])
def configure_esp():

    try:
        data = request.get_json()

        response = requests.post(
            f"{ESP_URL}/config",
            json=data,
            timeout=8
        )

        return jsonify(response.json()), response.status_code

    except requests.exceptions.RequestException as error:

        return jsonify({
            "message": "Cannot connect to ESP8266.",
            "error": str(error)
        }), 503


# Get ESP8266 status
@app.route("/api/status")
def esp_status():

    try:
        response = requests.get(
            f"{ESP_URL}/status",
            timeout=5
        )

        return jsonify(response.json())

    except requests.exceptions.RequestException:

        return jsonify({
            "connected": False,
            "message": "ESP8266 is unavailable."
        }), 503


if __name__ == "__main__":

    print("================================")
    print("JARVIC ESP8266 WIFI DASHBOARD")
    print("================================")
    print("Dashboard: http://127.0.0.1:5000")

    app.run(
        host="127.0.0.1",
        port=5000,
        debug=False
    )
