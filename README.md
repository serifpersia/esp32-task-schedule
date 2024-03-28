<div align="center">

![image](https://github.com/serifpersia/esp32-task-schedule/assets/62844718/b37c0835-81fc-43b3-864c-4adb51dedd6f)
</div>

# ESP32 Task Scheduler

This is a basic template project for scheduling tasks with ESP32. It allows you to set up tasks to run at specific times using a web interface. 

## Required Libraries

To build this project using the Arduino IDE 1.8.x, you will need the following libraries:
- [ESPAsyncWebServer](https://github.com/me-no-dev/ESPAsyncWebServer)
- [AsyncTCP](https://github.com/me-no-dev/AsyncTCP)
- [WebSockets](https://github.com/Links2004/arduinoWebSockets)
- [ArduinoJson](https://github.com/bblanchon/ArduinoJson)

For uploading SPIFFS, you can use my [ESP32PartitionTool](https://github.com/serifpersia/esp32partitiontool).

## Setup

1. Configure your network credentials and LED PIN if your board has it.
2. Upload the sketch to your ESP32 board.
3. Upload SPIFFS using ESP32PartitionTool.

## Usage

1. After uploading, find the IP address of your ESP32 in serial monitor and connect to it using a web browser.
2. Set the time and duration for your task using the web interface.
3. Use the toggle to enable the task.
4. Every time the page is loaded, the ESP32 sends its values to the web browser client, so the UI reflects the state of the ESP32.
5. Every minute, the internet time gets updated to prevent time drift.
6. At the scheduled time, the `runActivity` function will execute. You will see a printout and the LED light up for the duration set from the website UI. The `stopActivity` print indicates the task finished running.
7. You can modify the `runActivity` function to control a servo or perform any other desired action.

## License

This project is licensed under the [MIT License](LICENSE).

## Issues and Contributions

Feel free to open an issue if you encounter any problems or have suggestions for improvements. Contributions via pull requests are also welcome.

