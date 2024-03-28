var Socket;

// Function to initialize WebSocket
function init() {
  Socket = new WebSocket("ws://" + window.location.hostname + ":81/");

  // Add error event handler
  Socket.addEventListener("error", function(error) {
    console.error("WebSocket error:", error);
  });

  // Add close event handler
  Socket.addEventListener("close", function(event) {
    if (event.wasClean) {
      console.log(
        "WebSocket closed cleanly, code=" +
          event.code +
          ", reason=" +
          event.reason
      );
    } else {
      console.error("WebSocket connection died");
    }
  });

  // Add init_request event handler
  Socket.addEventListener("open", function(event) {
    console.log("WebSocket connection opened");
    Socket.send(JSON.stringify({ type: "init_request" }));
  });

  // Add event listener to handle initial state message
  Socket.onmessage = function(event) {
    const data = JSON.parse(event.data);
    if (data.type === "initial_state") {
      // Update UI with initial state from ESP32
      updateTime(data.time);
      updateDuration(data.duration);
      updateSystemState(data.enabled);
    }
  };
}

// Function to update time input field
function updateTime(time) {
  document.getElementById("time-input").value = time;
}

// Function to update duration input field
function updateDuration(duration) {
  document.getElementById("duration-input").value = duration;
}

// Function to update system state
function updateSystemState(enabled) {
  document.getElementById("toggle-switch").checked = enabled;
}

function setTime() {
  const timeInput = document.getElementById("time-input").value;
  Socket.send(JSON.stringify({ type: "set_time", time: timeInput }));
  console.log("Time set: ", timeInput);
}

function toggleSystem() {
  const toggleSwitch = document.getElementById("toggle-switch");
  const isSystemEnabled = toggleSwitch.checked;
  console.log(isSystemEnabled);
  Socket.send(
    JSON.stringify({ type: "toggle_system", enabled: isSystemEnabled })
  );
}

function setDuration() {
  const durationInput = document.getElementById("duration-input").value;
  console.log("Duration set: ", durationInput, "second/s");
  Socket.send(JSON.stringify({ type: "set_duration", time: durationInput }));
}

// Call the init function when the window loads
window.onload = function(event) {
  init();
};
