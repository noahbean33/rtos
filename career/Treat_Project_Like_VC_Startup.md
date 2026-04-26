# Treat Your Project Like a VC Startup

## Three Principles for Embedded Portfolio Projects

### 1. Get Your MVP Done ASAP
- **Speed is critical** — motivation fades over time; feature creep kills projects
- Don't build infrastructure you don't need (e.g., don't write a custom RTOS for a plant waterer)
- Use **existing libraries** — FreeRTOS, Arduino Bluetooth drivers, etc.
- Prove the concept first; optimize or rebuild components later
- **Hiring signal:** shows you can meet deadlines and prioritize business value over gold-plated code

### 2. Pick Something Personal
- VCs invest in people with a **specific reason** to solve a specific problem
- Don't build a generic weather station from a tutorial
- Build something tied to **your domain knowledge** (e.g., mountain bike telemetry if you're a cyclist)
- Personal projects are **unique** — recruiters have seen 10 identical mosquito turrets
- You'll remember the hard technical problems because you *needed* to solve them
- **Hiring signal:** passionate, deep technical dives; easier to talk about I2C bus contention when it blocked your personal goal

### 3. Tell a Compelling Story
- Structure around **STAR**: Situation, Task, Action, Result
- GitHub README needs a **problem statement and solution**, not just a file list
- Build a **portfolio website** with photos, videos, hardware close-ups
- **Bad:** "I used an ESP32 to send data to the cloud"
- **Good:** "I had a 10mA power budget but needed Wi-Fi. I implemented deep sleep cycles and optimized the TCP stack to reduce on-time by 40%"
- Practice explaining your projects out loud — rehearse before interviews

## Embedded-Specific Advice
- Use a **microcontroller** (ESP32, STM32, etc.) — interviewers want to see hardware
- Write in **C or C++** (Rust is still niche — proceed with caution)
- Project should **interact with the real world** — LEDs, motors, sensors, actuators
- CS-background applicants especially need to demonstrate hardware comfort (breadboard, soldering)

## Example: "Light Phone" Project
- **Problem:** checking phone breaks flow state; missing important notifications causes anxiety
- **Solution:** ESP32 + 16×16 LED matrix + Bluetooth connection to phone
- Default: calming lava-lamp pattern; blinks red only for prioritized contacts
- Used existing hardware (ESP32, LED matrix) and existing Bluetooth drivers
- Personal, shipped fast, tells a compelling story
