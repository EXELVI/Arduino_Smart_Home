# Arduino Smart Home 🏠

[![Arduino](https://img.shields.io/badge/Arduino-IDE-00979D?style=for-the-badge&logo=arduino&logoColor=white)](https://www.arduino.cc/)
[![Repository](https://img.shields.io/github/languages/code-size/EXELVI/Arduino_Smart_Home?style=for-the-badge)](https://github.com/EXELVI/Arduino_Smart_Home)
[![Last Commit](https://img.shields.io/github/last-commit/EXELVI/Arduino_Smart_Home?style=for-the-badge)](https://github.com/EXELVI/Arduino_Smart_Home)
[![Stars](https://img.shields.io/github/stars/EXELVI/Arduino_Smart_Home?style=for-the-badge)](https://github.com/EXELVI/Arduino_Smart_Home)
[![Forks](https://img.shields.io/github/forks/EXELVI/Arduino_Smart_Home?style=for-the-badge)](https://github.com/EXELVI/Arduino_Smart_Home)
[![Issues](https://img.shields.io/github/issues/EXELVI/Arduino_Smart_Home?style=for-the-badge)](https://github.com/EXELVI/Arduino_Smart_Home)

Welcome to the Arduino Smart Home project! This project is a miniaturized smart home system and security system. It integrates various sensors, an LCD display, and a keypad to provide real-time information and control 

## 🚀 Features 

- **Security System**: Monitors for motion and door status to detect unauthorized access.
- **Fire Detection**: Alerts you if a fire is detected, requiring a PIN to deactivate the alarm.
- **Environmental Monitoring**: Displays temperature and humidity on an LCD.
- **Admin Controls**: Change PIN, configure zones, and select different display modes.
- **Garage Control**: Open and close the garage door using a servo motor.

## 🛠️ Components 

- Arduino Board (e.g., Arduino Mega)
- LiquidCrystal I2C Display 20x4
- Keypad
- Servo Motor
- DHT11 Temperature and Humidity Sensor
- DS3231 RTC (Real Time Clock)
- Various LEDs, Buzzers, and PIR Motion Sensors
- Magnetic Switches for door detection

## 💻 Installation 💻

1. **Clone the Repository**:

    ```bash
    git clone https://github.com/EXELVI/Arduino_Smart_Home.git
    ```

2. **Install Libraries**:

    Ensure you have the following Arduino libraries installed:
    - `LiquidCrystal_I2C`
    - `Keypad`
    - `Password`
    - `Servo`
    - `dht_nonblocking`
    - `DS3231`

    You can install these libraries via the Arduino Library Manager.

3. **Upload the Code**:

    Open the `Arduino_Smart_Home.ino` file in the Arduino IDE and upload it to your Arduino board.

4. **Connect the Hardware**:

    Follow the pin configuration specified in the code to connect the components to your Arduino board.

## 📜 Usage 

- **Activate the Alarm**: Press 'A' on the keypad.
- **Admin Menu**: Press 'B' to access the admin menu for PIN changes and display settings.
- **Control Lights**: Use keys '1', '2', and '3' to toggle different LEDs.
- **Garage Door**: Press 'D' to open or close the garage door.
- **Display Options**: Use the admin menu to switch between temperature/humidity and date/time displays.

## ⚙️ Configuration 

- **PIN Code**: Default is `0000`. Change it via the admin menu.
- **Zone Settings**: Configure which zones are active or disabled through the admin menu.

## 🧩 Code Overview 

The code implements:

- **Security Monitoring**: Detects motion and door statuses to trigger alarms.
- **Fire Detection**: Uses a fire sensor to detect smoke and raise alerts.
- **Environmental Data**: Reads and displays temperature and humidity.
- **Admin Functions**: Allows PIN changes and configuration of system settings.

## 🤝 Contributing 

Feel free to fork the repository, make improvements, and submit pull requests. If you encounter any issues or have suggestions, please open an issue on GitHub.

