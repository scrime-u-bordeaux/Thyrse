# Thyrse
Projet de Donatien Garnier, controlleur sans fil dans un cep de vigne, NRF24-01+ et teensy3.2   
  
  **MAI 2024 : MPU6050 + HX710 + teensy3.2  
  lib : https://github.com/tockn/MPU6050_tockn/tree/master  
  !!!ATTENTION axes conforme à la carte (Y = vertical), changement patch à prévoir!!!  
  calibration de la dérive des gyros au démarage => ne pas bouger le metaphorminx avant led verte allumée (7 secondes)  
  reste 7 pitchbend (14bit) disponibles  
  implementation midi 2024-mai :  
channel1-pitchbend = capteur pression souffle    
channel2-pitchbend = bouton rotatif    
channel3-pitchbend = accelerationX  
channel4-pitchbend = accelerationY  
channel5-pitchbend = accelerationZ  
channel6-pitchbend = gyroX  
channel7-pitchbend = gyroY  
channel8-pitchbend = gyroZ  
channel7-noteOn/Off 1-5 = boutons**  
.................................  
**implementation midi Thyrse2022-novembre :  
channel1-pitchbend = capteur pression souffle    
channel2-pitchbend = bouton rotatif    
channel3-pitchbend = accelerationX  
channel4-pitchbend = accelerationY  
channel5-pitchbend = accelerationZ  
channel7-noteOn/Off 1-5 = boutons**  
  
Bibliothèque capteur pression hx710 (teensy OK) : https://github.com/kurimawxx00/hx710B_pressure_sensor/blob/main/HX710B.cpp  
  
doc:  
https://lastminuteengineers.com/wp-content/uploads/arduino/Pinout-nRF24L01-PA-LNA-External-Antenna-Wireless-Transceiver-Module.png  
https://github.com/nRF24/RF24  
https://forum.pjrc.com/threads/46060-Teensy-(LC)-and-nrf24-do-not-work  
https://playground.arduino.cc/InterfacingWithHardware/Nrf24L01/  
  
Debug, code simple : https://forum.arduino.cc/t/simple-nrf24l01-2-4ghz-transceiver-demo/405123  
problèmes d'antenne :   
https://forum.arduino.cc/t/nrf24l01-pa-lna-having-to-touch-antenna/368173/22  
https://hackaday.com/2016/05/31/fixing-the-terrible-range-of-your-cheap-nrf24l01-palna-module/  

  
Modif antenne : https://www.instructables.com/Enhanced-NRF24L01/  
    
autoAck off : https://devzone.nordicsemi.com/f/nordic-q-a/3649/setting-auto-ack-off-on-pipe0-and-on-on-pipe1-makes-nrf24l01-stop-sending-receiving
  
alternatif touch sensor avec teensy : https://www.pjrc.com/teensy/td_libs_CapacitiveSensor.html  
**TeensyLC OK pour capteurs capacitifs directement sur pins "touch" x11!**  
Bibliothèque alternative pour n'importe quelle pin avec pullup : https://github.com/adrianfreed/FastTouch  
**Teensy LC !: 
  SerialUSB.begin(9600);
  while (!Serial) {
    ; // _wait for serial port to connect. Needed for native USB port only_
  }**  
    
Liste d'achat :  
https://www.robotshop.com/eu/fr/chargeur-batteries-lipo-micro-usb-adafruit.html  
https://www.robotshop.com/eu/fr/cellule-batterie-lithium-de-polymere-37-v-1200-mah.html  
https://www.robotshop.com/eu/fr/module-emetteur-recepteur-24g-nrf24l01-velleman-2pk.html  
https://www.robotshop.com/eu/fr/platine-developpement-microcontroleur-usb-teensy-lc.html  
https://www.robotshop.com/eu/fr/module-accelerometre-3-axes-mma8452-velleman.html  
_(radio alternative, plus fiable? :  
https://www.robotshop.com/eu/fr/module-emetteur-recepteur-seriel-sans-fil-433mhz-1000m.html ; fix the clones : https://www.youtube.com/watch?v=ZfBuEAH-Q8Y&ab_channel=DIYTECHBROS ; TUTO :https://wolles-elektronikkiste.de/en/hc-12-radio-module)_  

(?)https://www.robotshop.com/eu/fr/seeedstudio-grove-capteur-tactile-capacitif-i2c-v3-12-touches-mpr121.html  
(option: carte avec radio/processeur/chargeur batterie...)https://learn.adafruit.com/bluefruit-nrf52-feather-learning-guide  
https://learn.adafruit.com/introducing-the-adafruit-nrf52840-feather  
