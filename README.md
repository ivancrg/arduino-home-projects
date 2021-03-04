# arduino-home-projects
Some Arduino home projects (RFID garage door opener and WiFi customizable, music responsive ledstrip)



## RFID_garage

Used to open/close/freeze garage door when valid card is close enough to the sensor.



## wifi_arduino_responsive_ledstrip

Several modes of operation. Everything controlled from your browser.

### 1. Music visualizer
You can choose from several representations of detected sound. Frequencies are split into 7 parts by MSGEQ7 (not yet implemented due to missing resistors and capacitators, will be added).
You can select a color for each frequency interval.

### 2. Running light
Running light with customizable color and speed (in milliseconds).

### 3. Loudness visualizer
Uses MSGEQ7 to detect loudness of the received sound signal and displays it accordingly to the selected color. Can be used with an ordinary microphone to display actual loudness.

### 4. Fade in - fade out
Fades from one color to another at a certain speed. Colors and speed fully customizable.

### 5. Turned on
Constantly turned on ledstrip in a selected color.





Note: other arduino home automation and car-arduino projects to be added
