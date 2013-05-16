#include <msp430.h> 

//Global variables
int i2cStatus = 0;	//Status of communication
char i2cDataTX = 0;	//Data to be transmited
char i2cDataRX = 0;	//Data received
char dataOk = 0;	//Check data

/*
 * main.c
 */
int main(void)
{
    WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer

    //Set clk = 1MHz
    DCOCTL = CALDCO_1MHZ;
    BCSCTL1 = CALBC1_1MHZ;
    //End set clk = 1MHz

    //Config I2C control registries
    USICTL0 |= USIPE7 + USIPE6 + USISWRST;	//Enable port function and reset regitries
    USICTL1 |= USII2C + USISTTIE + USIIE;	//Enable I2C and enable interrupts
    USICKCTL |= USICKPL;	//Set clk polarity as inactive state high
    //End config I2C control registries

    USICTL0 &= ~USISWCLK;	//start USI
    USICTL1 &= ~USIIFG;		//Clear interrupt flag

    //Set ports
    P1DIR = 0xF7;	//All output less P1.3 (switch)
    P1OUT = 0x01;
    //End set ports

    _EINT();	//Enable interrupt

    while(1)
    {
    	if(i2cDataRX == 0x0F) //Command to start counting
    	{
    		while(~(P1IN & 0x04))	//While s2 not pressed
    		{
    			i2cDataTX ++;	//Increase counting
    			P1OUT ^= 0x01;	//Toggle LED
    			__delay_cycles(1000);	//Delay
    		}//End while

    		i2cDataTX = 0;	//Reset count
    	}
    	else
    	{
    		P1OUT = 0x01;
    		i2cDataTX = 0;
    	}
    }
}

//USI interrupt service routine
#pragma vector=USI_VECTOR
__interrupt void usiI2C(void)
{
	//Adders of the device
	static const char slaveAddres = 0xA0;

	if(USICTL1 & USISTTIFG)
	{
		USICTL1 &= ~USISTTIFG;	//Clear start condition interrupt flag
		i2cStatus = 1;	//Go to receive address
	}

	switch(i2cStatus)
	{
	case 0:	//idle state
		USICNT |= USISCLREL;	//release SCL
		break;
	case 1: //Receive address
		USICTL0 &= ~USIOE;	//Disable output
		USICNT = (USICNT & 0xE0) + 0x08;	//USICNT = 8
		i2cStatus = 2;
		break;
	case 2:	//Check address and send (N)ACK
		if((USISRL & 0xF0) == slaveAddres)
		{
			//Check R/W
			if(USISRL & 0x01)
				i2cStatus = 3;	//Go to read
			else
				i2cStatus = 4;	//Go to write

			USISRL = 0x00;	//MSB = 0 (ACK)
		}
		else
		{
			USISRL = 0xFF;	//MSB = 1 (NACK)
			i2cStatus = 8;	//Prep for start
		}
		//Sending (N)ACK
		USICTL0 |= USIOE;	//Enable output
		USICNT = (USICNT & 0xE0) + 0x01;	//USICNT = 1
		//End sending (N)ACK
		break;
	case 3:	//Start RX
		USICTL0 &= ~USIOE;	//Disable output
		USICNT = (USICNT & 0xE0) + 0x08;	//USICNT = 8
		i2cStatus = 5;
		break;
	case 4:	//Sending data
		USISRL = i2cDataTX;
		USICTL0 |= USIOE;	//Enable output
		USICNT = (USICNT & 0xE0) + 0x08;	//USICNT = 8
		i2cStatus = 6;
		break;
	case 5:	//Reception of data
		i2cDataRX = USISRL;
		//Sending ACK
		USISRL = 0x00;	//MSB = 0 (ACK)
		USICTL0 |= USIOE;	//Enable output
		USICNT = (USICNT & 0xE0) + 0x01;	//USICNT = 1
		//End sending ACK
		i2cStatus = 8;
		break;
	case 6:	//Receiving (N)ACK after TX
		USICTL0 &= ~USIOE;	//Disable output
		USICNT = (USICNT & 0xE0) + 0x01;	//USICNT = 1
		break;
	case 7:	//Checking (N)ACK after TX
			dataOk = (USISRL & 0x01)?1:0;
			i2cStatus = 0;
		break;
	case 8:	//Preparation for Start condition
		USICTL0 &= ~USIOE;	//Disable output
		i2cStatus = 0;
		break;
	}//End switch
}//End uisI2C
