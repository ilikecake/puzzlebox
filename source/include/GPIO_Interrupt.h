//Register addresses for LPC1758 GPIO interrupt setup

//GPIOINT addresses are added to LPC_GPIOINT_BASE	0x40028000
#define LPC_GPIOINT_BASE				0x40028000

#define LPC_GPIOINT_Status      		0x80	//GPIO overall Interrupt Status register. Section 9.5.6.1

#define LPC_GPIOINT_IO0IntEnR  			0x90	//GPIO Interrupt Enable for port 0 Rising Edge. Section 9.5.6.2
#define LPC_GPIOINT_IO2IntEnR           0xB0	//GPIO Interrupt Enable for port 2 Rising Edge. Section 9.5.6.3
#define LPC_GPIOINT_IO0IntEnF           0x94	//GPIO Interrupt Enable for port 0 Falling Edge. Section 9.5.6.4
#define LPC_GPIOINT_IO2IntEnF           0xB4	//GPIO Interrupt Enable for port 2 Falling Edge. Section 9.5.6.5

#define LPC_GPIOINT_IO0IntStatR         0x84	//GPIO Interrupt Status for port 0 Rising Edge Interrupt. Section 9.5.6.6
#define LPC_GPIOINT_IO2IntStatR         0xA4	//GPIO Interrupt Status for port 2 Rising Edge Interrupt. Section 9.5.6.7
#define LPC_GPIOINT_IO0IntStatF         0x88	//GPIO Interrupt Status for port 0 Falling Edge Interrupt. Section 9.5.6.8
#define LPC_GPIOINT_IO2IntStatF         0xA8	//GPIO Interrupt Status for port 2 Falling Edge Interrupt. Section 9.5.6.9

#define LPC_GPIOINT_IO0IntClr			0x8C	//GPIO Interrupt Clear register for port 0. Section 9.5.6.10
#define LPC_GPIOINT_IO2IntClr         	0xAC	//GPIO Interrupt Clear register for port 2. Section 9.5.6.11


void GPIOInt_Init();
void Chip_GPIO_WriteIntBit(uint32_t Register, uint8_t Port, uint32_t bit, uint8_t setting);
