#include <AT91SAM7SE512.H>              /* AT91SAM7SE512 definitions          */
#include "..\AT91SAM7SE-EK.h"           /* AT91SAM7SE-EK board definitions    */

#include "spi.h"




#define BOARD_MCK               48000000

void wait_ms(unsigned int n) {
#define MY_OSC_2 4987 // 1ms
	unsigned int i = MY_OSC_2 * n;
	while(i)i--;
}

#define PIN_USART0_RXD  (1 << 5)
#define PIN_USART0_TXD  (1 << 6)

#define USART_RX_LED AT91B_LED2
#define USART_TX_LED AT91B_LED1


void SPI_Enable(AT91S_SPI *spi)
{
    spi->SPI_CR = AT91C_SPI_SPIEN;
}
void SPI_Disable(AT91S_SPI *spi)
{
    spi->SPI_CR = AT91C_SPI_SPIDIS;
}
void SPI_Configure(AT91S_SPI *spi,
                          unsigned int id,
                          unsigned int configuration)
{
    AT91C_BASE_PMC->PMC_PCER = 1 << id;
    spi->SPI_CR = AT91C_SPI_SPIDIS;
    // Execute a software reset of the SPI twice
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_CR = AT91C_SPI_SWRST;
    spi->SPI_MR = configuration;
}
void SPI_ConfigureNPCS(AT91S_SPI *spi,
                              unsigned int npcs,
                              unsigned int configuration)
{
    spi->SPI_CSR[npcs] = configuration;
}
void SPI_Write(AT91S_SPI *spi, unsigned int npcs, unsigned short data)
{
    // Discard contents of RDR register
    //volatile unsigned int discard = spi->SPI_RDR;

    // Send data
	//spi->SPI_TDR = 0x20 | SPI_PCS(npcs);
	
    while ((spi->SPI_SR & AT91C_SPI_TXEMPTY) != 0);
    spi->SPI_TDR = data | SPI_PCS(npcs);
	
    while((spi->SPI_SR & AT91C_SPI_TDRE) == 0);
}
unsigned char SPI_IsFinished(AT91S_SPI *pSpi)
{
    return ((pSpi->SPI_SR & AT91C_SPI_TXEMPTY) != 0);
}
unsigned short SPI_Read(AT91S_SPI *spi)
{
    while ((spi->SPI_SR & AT91C_SPI_RDRF) == 0);
    return spi->SPI_RDR & 0xFFFF;
}

void enable_led(unsigned int LED) {
	if (LED == AT91B_POWERLED)
		AT91C_BASE_PIOA->PIO_SODR = LED;
	else
		AT91C_BASE_PIOA->PIO_CODR = LED;
}

void disable_led(unsigned int LED) {
	if (LED == AT91B_POWERLED)
		AT91C_BASE_PIOA->PIO_CODR = LED;
	else
		AT91C_BASE_PIOA->PIO_SODR = LED;
}

void blinkenlights(int n) {
	while(n--) {
		enable_led(AT91B_POWERLED);
		wait_ms(200);
		disable_led(AT91B_POWERLED);
		wait_ms(200);
	}
}

unsigned int uart_getc(void) {
	enable_led(USART_RX_LED);
	while(1) {
		if (AT91C_BASE_US0->US_CSR & AT91C_US_RXRDY) {
			break;
		} // if rxrdy, break
	} // while !rxrdy, wait.
	
	disable_led(USART_RX_LED);
	return AT91C_BASE_US0->US_RHR;
}

void uart_putc(unsigned int c) {
	enable_led(USART_TX_LED);
	while(1) {
		if (AT91C_BASE_US0->US_CSR & AT91C_US_TXRDY) {
			break;
		} // if txempty, break
	} // while !txempty, wait.

	AT91C_BASE_US0->US_THR = c;
	while(1) {
		if (AT91C_BASE_US0->US_CSR & AT91C_US_TXRDY) {
			disable_led(USART_TX_LED);
			break;
		} // if txempty, break
	} // while !txempty, wait.
}

int main(void) {
	unsigned int data = 65;

	// enable peripheral clock of us0 in PMC
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_PIOA;
	AT91C_BASE_PMC->PMC_PCER = 1 << AT91C_ID_US0;
	
	// disable rx, tx
	AT91C_BASE_PIOA->PIO_PDR = PIN_USART0_RXD | PIN_USART0_TXD;
	// enable rx, tx
	AT91C_BASE_PIOA->PIO_PER = PIN_USART0_RXD | PIN_USART0_TXD;
	
	// enable output on leds
	AT91C_BASE_PIOA->PIO_PER  = AT91B_LED_MASK;
	AT91C_BASE_PIOA->PIO_OER  = AT91B_LED_MASK;
	
	// switch off leds by default
	AT91C_BASE_PIOA->PIO_SODR = AT91B_LED2 | AT91B_LED1;
	AT91C_BASE_PIOA->PIO_CODR = AT91B_POWERLED;

	// reset && disable tx & rx
	AT91C_BASE_US0->US_CR = AT91C_US_RSTRX | AT91C_US_RSTTX | AT91C_US_RXDIS | AT91C_US_TXDIS; // control register
			
	// set mode
	AT91C_BASE_US0->US_MR = AT91C_US_CHRL | AT91C_US_PAR_NONE; // clock selection, no parity
	
	// set baud rate
	AT91C_BASE_US0->US_BRGR = (BOARD_MCK / 9600) / 16;
	
	AT91C_BASE_US0->US_RTOR = 0; // recieve timeout : inf
	AT91C_BASE_US0->US_TTGR = 0; //Transmitter Time-guard Register

	//enable tx, rx
	AT91C_BASE_US0->US_CR = AT91C_US_RXEN | AT91C_US_TXEN;
	
	

    SPI_Enable(AT91C_BASE_SPI);
		
		SPI_Configure(AT91C_BASE_SPI,5,0x1);
		
		
		AT91C_BASE_SPI->SPI_CSR[0] = AT91C_SPI_SCBR;



	while(1) {
		//AT91C_BASE_SPI->SPI_TDR = 0x34;
	
		
		SPI_Write(AT91C_BASE_SPI,1,0x30);
		
		
		
		
		
	} // while

	return 0;
}