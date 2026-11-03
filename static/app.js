
const ESP_URL = window.location.origin;

const form = document.getElementById("wifiForm");
const saveButton = document.getElementById("saveButton");
const statusElement = document.getElementById("status");
const networkElement = document.getElementById("network");
const ipElement = document.getElementById("ip");

function showStatus(message) {
    if (statusElement) {
        statusElement.textContent = message;
    }
}

// ============== SAVE WIFI ==============

async function saveWiFi(event) {

    if (event) {
        event.preventDefault();
    }

    const ssidElement = document.getElementById("ssid");
    const passwordElement = document.getElementById("password");

    if (!ssidElement || !passwordElement) {
        showStatus("Wi-Fi input fields not found.");
        return;
    }

    const ssid = ssidElement.value.trim();
    const password = passwordElement.value;

    if (!ssid) {
        showStatus("Please enter your Wi-Fi name.");
        return;
    }

    if (password.length > 0 && password.length < 8) {
        showStatus("Wi-Fi password must be at least 8 characters.");
        return;
    }

    if (saveButton) {
        saveButton.disabled = true;
        saveButton.textContent = "Connecting...";
    }

    showStatus("Sending Wi-Fi details to ESP8266...");

    try {

        const response = await fetch(
            ESP_URL + "/api/config",
            {
                method: "POST",
                headers: {
                    "Content-Type": "application/json"
                },
                body: JSON.stringify({
                    ssid: ssid,
                    password: password
                })
            }
        );

        const result = await response.json();

        if (!response.ok) {
            throw new Error(result.message || "Request failed");
        }

        showStatus(result.message);

        // Start checking the connection status
        updateStatus();

    } catch (error) {

        console.error("Wi-Fi configuration error:", error);

        showStatus(
            "Failed to fetch: " +
            (error.message || "Cannot connect to ESP8266")
        );

    } finally {

        if (saveButton) {
            saveButton.disabled = false;
            saveButton.textContent = "Save & Connect";
        }

    }

}

// ============== CHECK STATUS ==============

async function updateStatus() {

    try {

        const response = await fetch(
            ESP_URL + "/api/status",
            {
                method: "GET",
                cache: "no-store"
            }
        );

        if (!response.ok) {
            throw new Error("Status request failed");
        }

        const data = await response.json();

        showStatus(data.message || "ESP8266 is online");

        if (networkElement) {

            networkElement.textContent =
                "Wi-Fi: " + (data.ssid || "Not connected");

        }

        if (ipElement) {

            ipElement.textContent =
                "IP Address: " + (data.ip || "Not connected");

        }

    } catch (error) {

        console.error("Status error:", error);

        showStatus("ESP8266 is not responding.");

    }

}

// ============== EVENT LISTENERS ==============

if (form) {

    form.addEventListener("submit", saveWiFi);

} else if (saveButton) {

    saveButton.addEventListener("click", saveWiFi);

}

// Update status every 3 seconds

updateStatus();

setInterval(updateStatus, 3000);
