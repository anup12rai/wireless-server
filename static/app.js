
const configForm = document.getElementById("configForm");

const saveButton = document.getElementById("saveButton");

const result = document.getElementById("result");


configForm.addEventListener("submit", async function (event) {

    event.preventDefault();

    const boardName = document.getElementById("boardName").value.trim();

    const ssid = document.getElementById("ssid").value.trim();

    const password = document.getElementById("password").value;

    if (!boardName || !ssid || !password) {

        result.textContent = "Please complete all fields.";
        return;

    }

    saveButton.disabled = true;

    saveButton.textContent = "Connecting...";

    result.textContent = "Sending Wi-Fi credentials to JARVIC...";


    try {

        const response = await fetch("/api/config", {

            method: "POST",

            headers: {
                "Content-Type": "application/json"
            },

            body: JSON.stringify({

                boardName: boardName,

                ssid: ssid,

                password: password

            })

        });


        const data = await response.json();

        if (!response.ok) {

            throw new Error(data.message || "Connection request failed.");

        }

        result.textContent = data.message;

        result.style.color = "green";

    }

    catch (error) {

        result.textContent = error.message;

        result.style.color = "red";

    }

    finally {

        saveButton.disabled = false;

        saveButton.textContent = "Save & Connect";

    }

});
