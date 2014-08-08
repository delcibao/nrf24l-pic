/*
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
DEALINGS IN THE SOFTWARE.
*/

#ifndef NRF24L_H_
#define NRF24L_H_

#include <stdint.h>
#include "nRF24L01.h"

/* NRF24L defaults settings */
#define NRF24L_ADDR_LEN     5
/* CRC Enabled, CRC encoding 8 bits. */
#define NRF24L_CONFIG       ((1<<EN_CRC) | (0<<CRCO) )
/* RF output odBm */
#define RF_MAX_PA           ((1<<RF_PWR_LOW) | (1<<RF_PWR_HIGH) )
/* [RF_DR_LOW, RF_DR_HIGH]:
?00? ? 1Mbps
?01? ? 2Mbps
?10? ? 250kbps     */
#define RF_DR_1MBPS         ((0<<RF_DR_LOW)|(0<<RF_DR_HIGH))
#define RF_DR_2MBPS         ((0<<RF_DR_LOW)|(1<<RF_DR_HIGH))
#define RF_DR_250KBPS       ((1<<RF_DR_LOW)|(0<<RF_DR_HIGH))
#define nrf24l_true         1
#define nrf24l_false        0

/* CE Pin High =  RX / TX. */
#define nrf24l_ceHigh()     PORTCbits.RC1 = 1  // CE Pin
/* CE Pin Low = StandBy Mode  */
#define nrf24l_ceLow()      PORTCbits.RC1 = 0  // CE Pin
/* CSN Pin Chip Select Not. */
#define nrf24l_csnHigh()    PORTCbits.RC2 = 1  // CSN pin
 /* CSN Pin Chip Select Not. */
#define nrf24l_csnLow()     PORTCbits.RC2 = 0  // CSN pin

/**
 *  Defines for setting the nrf24l01p registers for transmitting or receiving mode
 * @param dataout
 * @param datain
 * @param len
 */
void nrf24l_transferSync(uint8_t *dataout, uint8_t *datain, uint8_t len);

/**
 *
 * @param dataout
 * @param len
 */
void nrf24l_transmitSync(uint8_t *dataout, uint8_t len);

/**
 * Initializes pins to communicate with the nrf24l01p module.
 * Should be called in the early initializing phase at startup.
 */
void nrf24l_init();

/**
 * Sets the important registers in the nrf24l01p module and powers the
 * module in receiving mode
 * NB: channel and payload must be set now.
 */
void nrf24l_config();

/**
 * Sets the receiving address
 * @param adr
 */
void nrf24l_setRADDR(uint8_t * adr);

/**
 * Sets the transmitting address
 * @param adr
 */
void nrf24l_setTADDR(uint8_t * adr);


uint8_t nrf24l_testCarrier(void);


/**
 * Set RF communication channel
 *
 * @param channel Which RF channel to communicate on, 0-127
 */
void nrf24l_setChannel(uint8_t channel);

/**
 *
 * @param pipe
 */
void nrf24l_setDynamicPayload(uint8_t pipe);

/**
 * Checks if data is available for reading
 * @return
 */
uint8_t nrf24l_dataReady();

/**
 *
 * @return
 */
uint8_t nrf24l_rxFifoEmpty();

/**
 * Reads nrf24l_payload bytes into data array
 * @param data
 */
void nrf24l_getData(uint8_t * data);

/**
 * Clocks only one byte into the given nrf24l01p register
 *
 * @param reg
 * @param value
 */
void nrf24l_configRegister(uint8_t reg, uint8_t value);

/**
 * Reads an array of bytes from the given start position in the nrf24l01p registers.
 * @param reg
 * @param value
 * @param len
 */
void nrf24l_readRegister(uint8_t reg, uint8_t * value, uint8_t len);

/**
 * Writes an array of bytes into inte the nrf24l01p registers.
 * @param reg
 * @param value
 * @param len
 */
void nrf24l_writeRegister(uint8_t reg, uint8_t * value, uint8_t len);

/**
 * Sends a data package to the default address. Be sure to send the correct
 * amount of bytes as configured as payload on the receiver.
 * @param value
 */
void nrf24l_send(uint8_t * value);

/**
 * Test if chip is still sending.
 * When sending has finished return chip to listening.
 * @return
 */
uint8_t nrf24l_isSending();

/**
 *
 * @return
 */
uint8_t nrf24l_getStatus();

/**
 *
 */
void nrf24l_powerUpRx();

/**
 *
 */
void nrf24l_flushRx();

/**
 * 
 */
void nrf24l_flushTx();

/**
 * 
 */
void nrf24l_startListening();

/**
 * 
 */
void nrf24l_stopListening();

/**
 *
 */
void nrf24l_powerUpTx();

/**
 *
 */
void nrf24l_powerDown();

/**
 * In sending mode.
 */
uint8_t nrf24l_PTX;

/**
 * Channel 0 - 127 or 0 - 84 in the US.
 */
uint8_t nrf24l_channel;

/**
 *  Payload width in bytes default 16 max 32.
 */
uint8_t nrf24l_payload;

#endif 
/* NRF24L_H_ */
