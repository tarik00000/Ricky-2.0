🤖 Ricky 2.0 – Remote Controlled Technical System

Ricky 2.0 is an advanced, independently designed, and custom-built tracked robot engineered for operation in hazardous and high-energy environments. Built on a tracked chassis for maximum stability, it combines 3D-printed hardware, long-range RF communication, and a multi-axis robotic arm.
🚀 Key Features

    Chassis Design: Tracked "mini-tank" design for superior stability on diverse terrains.

    Long-Range Control: Operates via a custom RF link with a range of up to 1 km.

    Robotic Manipulation: Equipped with a 6-axis metal robotic arm powered by high-torque servos.

    Real-Time Vision: Live video streaming from an onboard ESP32-CAM to a mobile device.

    Custom Controller: A modified drone transmitter repurposed with an Arduino Nano for precision control.

🛠️ System Architecture

Robot Unit (Main Controller: Arduino Mega 2560) 
Category	Components
Processing	
<p align="center">
  <img src="/Pictures/Arduino_MEGA_2560.webp" width="600" >
</p>
Arduino Mega 2560
Drivetrain	

4x RM60 12V DC Motors + Dual H-Bridge controllers
Actuators	
<p align="center">
  <img src="/Pictures/12V 60rpm.jpg" width="600" ><p align="center">
  <img src="/Pictures/h-bridge.jpg" width="600" >
</p>
</p>

Metal Robotic Arm with 6x 24kg Servo Motors
Communication	
<p align="center">
  <img src="/Pictures/arm.jpg" width="600" >
</p><p align="center">
  <img src="/Pictures/servomotor.jpg" width="600" >
</p>

RF Antenna + nRF24L01+ PA LNA module
Vision/Lights	
<p align="center">
  <img src="/Pictures/nrf antenna.jpg" width="600" >
</p>
Seeed Studio XIAO ESP32-CAM + Dual 3.3V LED lighting
Power + buzzer
<p align="center">
  <img src="/Pictures/camera.jpg" width="600" >
</p>
<p align="center">
  <img src="/Pictures/led-didoe.avif" width="600" >
</p>
<p align="center">
  <img src="/Pictures/buzz ricky.webp" width="600" >
</p>

0.96 inch OLED display
<p align="center">
  <img src="/Pictures/display.jpg" width="600" >
</p>

3S Lipo Battery (12.6V, 2200mAh) + Buck-down converters
<p align="center">
  <img src="/Pictures/buck-down.jpg" width="600" >
</p>

Remote Control Unit (Controller: Arduino Nano) 
<p align="center">
  <img src="/Pictures/nano.jpg" width="600" >
</p>

Category	Components
Inputs	

2x Joysticks, 2x Potentiometers, 2x 3-way switches, 7x Buttons
Feedback	
<p align="center">
  <img src="/Pictures/joystick.jpg" width="600" >
</p><p align="center">
  <img src="/Pictures/potenciometer.jpg" width="600" >
</p>
<p align="center">
  <img src="/Pictures/3-way.jpg" width="600" >
</p>
<p align="center">
  <img src="/Pictures/btn.jpg" width="600" >
</p>

Status LED and Indication Buzzer
Power	
<p align="center">
  <img src="/Pictures/led.jpg" width="600" >
</p>
<p align="center">
  <img src="/Pictures/buzz controler.jpg" width="600" >
</p>

2S Lithium-ion battery
📊 Technical Analysis

Power Consumption (Measured) 
Component	Voltage	Current (Measured)
Arduino Mega	12V	159mA
DC Motors (x4)	12V	280mA
Servo Motors (x6)	6V	600mA – 2A
Camera System	5V	30mA

Field Test Results 

    Terrain Performance: Excellent on concrete, indoor surfaces, and rocky terrain.

    Signal Range:

        Urban/Obstacles: 100m – 300m.

        Open Space: 500m – 1000m.

        Ideal Conditions: Up to 1.5km.

📂 Software & Design

    Code: Written in C++/Arduino for Mega and Nano boards.

    3D Design: Custom parts designed in SolidWorks and Tinkercad, printed on a Creality Ender 3 V2.
