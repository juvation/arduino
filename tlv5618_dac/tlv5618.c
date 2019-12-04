// DAC Data Transfer
#define SLAVESELECT 10 // CS
#define DATAOUT 11 // DIN
#define SPICLOCK 13 // SCLK

First thing we need to do is configure the SPI interface correctly inside the Arduino.This routine should be put inside the setup() function:

// Setup SPI Interface code BEGIN ///////////////////////////////////////////
byte clr;
pinMode(DATAOUT, OUTPUT);
pinMode(SPICLOCK,OUTPUT);
pinMode(SLAVESELECT,OUTPUT);

digitalWrite(SLAVESELECT,HIGH); //disable device

//The SPI control register (SPCR) has 8 bits, each of which control a particular SPI setting.

// SPCR
// | 7 | 6 | 5 | 4 | 3 | 2 | 1 | 0 |0000000000000000000
// | SPIE | SPE | DORD | MSTR | CPOL | CPHA | SPR1 | SPR0 |

// SPIE - Enables the SPI interrupt when 1
// SPE - Enables the SPI when 1
// DORD - Sends data least Significant Bit First when 1, most Significant Bit first when 0
// MSTR - Sets the Arduino in master mode when 1, slave mode when 0
// CPOL - Sets the data clock to be idle when high if set to 1, idle when low if set to 0
// CPHA - Samples data on the falling edge of the data clock when 1, rising edge when 0'
// SPR1 and SPR0 - Sets the SPI speed, 00 is fastest (4MHz) 11 is slowest (250KHz)

SPCR = (1<<SPE)|(1<<MSTR)|(1<<CPHA);
clr=SPSR;
clr=SPDR;
delay(10);

So now we can write a function SetVoltage(int) that will change the DAC output.

/////////////////////////////////////////////////////////////////////
// DAC SPI Interface
char spi_transfer(volatile char data)
{
  SPDR = data; // Start the transmission
  while (!(SPSR & (1<<SPIF))) // Wait the end of the transmission
  {
  };
  return SPDR; // return the received byte
}

/////////////////////////////////////////////////////////////////////
// Set the voltage on the 12bit DAC
byte SetVoltage(short Voltage)
{
  Voltage = Voltage | 32768; // Use DAC A

  digitalWrite(SLAVESELECT,LOW);

  //2 byte opcode -- for some reason we have to do this twice to make it stick with the TLV5618
  spi_transfer(Voltage>>8);
  spi_transfer(Voltage);

  spi_transfer(Voltage>>8);
  spi_transfer(Voltage);

  digitalWrite(SLAVESELECT,HIGH); //release chip, signal end transfer
}
