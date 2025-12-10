# LaskaKit STCC4 + SHT40 Sensor of CO2, temperature and humidity of air

This LaskaKit board is based on the Sensirion STCC4, a compact and highly efficent **CO2 sensor** which also comes with an integrated SHT-40 sensor which allows for accuarte measuring of **humidity** and **temperature**.

Attention! You must let the STCC4 "wear in" by letting it continuously measure ambient CO2 for at least 12 hours, after which the sensor will be calibrated and you may then use it without any limitations, even after disconnecting it from power.

We've created several programs, including a re-calibration script, which you can find in the SW folder. Like all of our other boards, it's entirely open source. The diagram can be found in the HW folder.

LaskaKit STCC4 je kompaktní senzor určený pro měření koncentrace CO₂, teploty a vlhkosti vzduchu. Díky integrovanému čidlu od firmy Sensirion nabízí přesná a spolehlivá data s nízkou spotřebou energie. Modul komunikuje přes sběrnici I²C, což umožňuje snadnou integraci s mikrokontroléry jako Arduino, ESP32 nebo Raspberry Pi. Hodí se pro aplikace v oblasti měření kvality vzduchu, chytrých domácností, HVAC systémů nebo výzkumných projektů.

Pozor! Čidlo je nutné před využitím nechat 12 hodin kontinuálně měřit, aby se čidlo "zkalibrovalo". Poté je možné čidlo používat bez omezení (i po odpojení od energie).

Specifications:
Sensor: Sensirion STCC4
Measured values: CO₂, temperature, relative humidity
CO₂ measurement range: 400 – 5000 ppm
CO₂ measurement accuracy: ±(30 ppm + 3 % of the measured value)
Temperature measurement accuracy: ±0,8 °C
Relative humidity measurement accuracy: ±3 % RH
Bus: I²C (up to 400 kHz)
Input voltage : 2.7V - 5.5V, 3.3V recommended
Current: Very low, suitable for battery-powered devices
Dimensions:  23 × 22 mm
