
from flask import Flask, render_template, request, jsonify
import requests

app = Flask(__name__)

ESP8266_IP = "192.168.4.1"


@app.route("/")
def index():
    return render_template("index.html")


@app.route("/api/config", methods=["POST"])
def configure_esp():

    data = request.get_json()

    if not data:
        return jsonify({
            "success": False,
            "message": "Invalid request."
        }), 400

    board_name = data.get("boardName", "").strip()
    ssid = data.get("ssid", "").strip()
    password = data.get("password", "")

    if not board_name or not ssid or not password:
        return jsonify({
            "success": False,
            "message": "All fields are required."
        }), 400

    if len(ssid) > 32 or len(password) < 8 or len(password) > 63:
        return jsonify({
            "success": False,
            "message": "Invalid Wi-Fi credentials."
        }), 400

    if len(board_name) > 32:
        return jsonify({
            "success": False,
            "message": "Board name is too long."
        }), 400

    try:

        response = requests.post(
            f"http://{ESP8266_IP}/connect",
            data={
                "boardName": board_name,
                "ssid": ssid,
                "password": password
            },
            timeout=10
        )

        response.raise_for_status()

        return jsonify({
            "success": True,
            "message": "Credentials sent to ESP8266. Connecting to Wi-Fi..."
        })

    except requests.RequestException as error:

        return jsonify({
            "success": False,
            "message": "Cannot communicate with ESP8266. Check the setup Wi-Fi connection."
        }), 503


if __name__ == "__main__":

    app.run(
        host="0.0.0.0",
        port=5000,
        debug=False
    )
