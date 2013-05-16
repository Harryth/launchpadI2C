launchpadI2C
============

I2C transmission with msp430 Launchpad

               MSP430G2452
            -----------------
            |               |
            |               |
      LED <-|P1.0           |
            |               |
       S2 ->|P1.3           |
            |               |
            |           P1.6|<-> SDA
            |           P1.7|<-  SCL



**Pull-up resistor for pins SDA and SCL are needed**
**MSP430 voltage 3.3V**

LED is on until command of start counting is received, this command is the integer 0x0F
Afer that, counts and toggle LED, until switch (S2) is pressed, then data is ready to be transmited. Master must request the data.



