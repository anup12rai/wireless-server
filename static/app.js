
const form = document.getElementById("configForm");

const result = document.getElementById("result");

const button = document.getElementById("saveButton");


form.addEventListener("submit", async function(event) {

    event.preventDefault();

    const settings = {

        boardName: document.getElementById("boardName").value.trim(),

        ssid: document.getElementById("ssid").value.trim(),

        password: document.getElementById("password").value

    };


    if (!settings.boardName || !settings.ssid) {

        result.textContent = "Please fill in all fields.";

        return;

    }

    if (settings.password.length < 8 ||
        settings.password.length > 63) {

        result.textContent =
            "Wi-Fi password must contain 8 to 63 characters.";

        return;

    }


    button.disabled = true;

    button.textContent = "Connecting...";

    result.textContent = "Sending settings to ESP8266...";


    try {

        const response = await fetch("/api/config", {

            method: "POST",

            headers: {

                "Content-Type": "application/json"

            },

            body: JSON.stringify(settings)

        });


        const data = await response.json();


        if (!response.ok) {

            throw new Error(data.message || "Configuration failed");

        }


        result.textContent = data.message;

    }

    catch(error) {

        result.textContent =
            "Error: " + error.message;

    }

    finally {

        button.disabled = false;

        button.textContent = "Save & Connect";

    }

});
