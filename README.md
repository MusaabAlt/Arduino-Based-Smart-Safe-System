# Arduino-Based-Smart-Safe-System


# SmartSafe-Arduino

An Arduino-based smart safe system that provides secure access control using a keypad, LCD display, servo motor lock, LEDs, and buzzer feedback.  
The project is designed as a complete embedded system example combining hardware control, user interaction, and state-based logic.

---

## Features

- Password-protected access (4-digit PIN)
- LCD user interface (16x2)
- Servo-controlled locking mechanism
- Visual feedback using LEDs (success / error)
- Audible feedback using buzzer
- On-device menu after successful login
- Password reset functionality
- Lockout protection after multiple failed attempts
- Modular and readable code structure

---

## Hardware Components

- Arduino UNO
- 16x2 LCD (HD44780 – 16 pins)
- 4x3 Keypad 
- Servo Motor SG90
- Green and Red LED "Diode"
- Buzzer (active or passive)
- Potentiometer (10k) for LCD contrast
- Resistors (220Ω for LEDs / LCD backlight)
- Breadboard and jumper wires
- 5V Power Source

---

## How It Works

1. The user enters a 4-digit password using the keypad.
2. The input is masked on the LCD for security.
3. If the password is correct:
   - Access is granted
   - Servo unlocks the safe
   - Menu appears on the LCD
4. The user can:
   - Reset the password
   - Lock and exit
5. Incorrect attempts trigger visual and audio alerts.
6. After multiple failed attempts, the system enters a temporary lockout state.

---

## Software Requirements

- Arduino IDE
- Libraries:
  - `Keypad`
  - `LiquidCrystal`
  - `Servo`
