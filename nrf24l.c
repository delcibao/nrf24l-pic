/**********************************************************************
    Port of the http://www.tinkerer.eu/AVRLib/nRF24L01 library

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, 
    EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF 
    MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND 
    NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT 
    HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
    WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, 
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER 
    DEALINGS IN THE SOFTWARE.
 ***********************************************************************/
#if defined(__XC)
    #include <xc.h>        /* XC8 General Include File */
#elif defined(HI_TECH_C)
    #include <htc.h>       /* HiTech General Include File */
#endif
#include <stdint.h>

#include "nrf24l.h"
#include "nRF24L01.h"

#include "system.h" 	/* delay routines definitions */

/**
 * 1 GND   GND
 * 2 VCC   VCC
 * 3 CE    RC1
 * 4 CSN   RC2
 * 5 SCK   SCL
 * 6 MOSI  SDO
 * 7 MISO  SDI
 * 8 IRQ   n/c
 */

/**
 *  Defines for setting the nrf24l01p registers for transmitting or receiving mode
 * @param dataout
 * @param datain
 * @param len
 */
void nrf24l_transferSync(uint8_t *dataout, uint8_t *datain, uint8_t len) {
    uint8_t i;
    for (i = 0; i < len; i++) {
        datain[i] = spi_transfer(dataout[i]);
    }
}

/**
 *
 * @param dataout
 * @param len
 */
void nrf24l_transmitSync(uint8_t *dataout, uint8_t len) {
    uint8_t i;
    for (i = 0; i < len; i++) {
        spi_transfer(dataout[i]);
    }
}

/**
 * Initializes pins to communicate with the nrf24l01p module.
 * Should be called in the early initializing phase at startup.
 */
void nrf24l_init() {

    nrf24l_channel = 75; // Channel 0 - 127 or 0 - 84 in the US.
    nrf24l_payload = 4; // Payload width in bytes. max 32.

    TRISCbits.TRISC1 = 0; // CE Pin Output;
    TRISCbits.TRISC2 = 0; // CSN pin Output;
    spi_init();
    nrf24l_ceLow();
    nrf24l_csnHigh();
    delayms(5);
}

/**
 * Sets the important registers in the nrf24l01p module and powers the
 * module in receiving mode
 * NB: channel and payload must be set now.
 */
void nrf24l_config() {


    // Set RF channel
    nrf24l_configRegister(RF_CH, nrf24l_channel);

    // Set length of incoming payload 
    nrf24l_configRegister(RX_PW_P0, nrf24l_payload);
    nrf24l_configRegister(RX_PW_P1, nrf24l_payload);
    //nrf24l_configRegister(RF_SETUP, 0x06); // 1Mbps, Max PA
    nrf24l_configRegister(RF_SETUP, (RF_DR_1MBPS | RF_MAX_PA)); // 1Mbps, Max Power Amp.
    // Start receiver 
    nrf24l_powerUpRx();

    nrf24l_flushRx();
    nrf24l_flushTx();
}

/**
 * Sets the receiving address
 * @param adr
 */
void nrf24l_setRADDR(uint8_t * adr) {
    nrf24l_ceLow();
    nrf24l_writeRegister(RX_ADDR_P1, adr, NRF24L_ADDR_LEN);
    nrf24l_ceHigh();
}

/**
 * Sets the transmitting address
 * @param adr
 */
void nrf24l_setTADDR(uint8_t * adr) {
    /*
     * RX_ADDR_P0 must be set to the sending addr for auto ack to work.
     */
    nrf24l_writeRegister(RX_ADDR_P0, adr, NRF24L_ADDR_LEN);
    nrf24l_writeRegister(TX_ADDR, adr, NRF24L_ADDR_LEN);
}

/**
 *
 * @param pipe
 */
void nrf24l_setDynamicPayload(uint8_t pipe){
    nrf24l_configRegister(EN_AA, (1<<pipe) );  // Enable Auto Acknowledgment.
    nrf24l_configRegister(DYNPD, (1<<pipe) );  // Requires EN_DPL and ENAA_Px
}

uint8_t nrf24l_readConfig(uint8_t reg){
  nrf24l_csnLow();
  spi_transfer( R_REGISTER | ( REGISTER_MASK & reg ) );
  uint8_t result = spi_transfer(NOOP);
  nrf24l_csnHigh();
  return result;
}

/**
void RF24::enableDynamicPayloads(void)
{
  // Enable dynamic payload throughout the system
  nrf24l_writeRegister(FEATURE,read_register(FEATURE) | (1<<EN_DPL) );

  // If it didn't work, the features are not enabled
  if ( ! nrf24l_readConfig(FEATURE) ){
      // So enable them and try again
     toggle_features();
     write_register(FEATURE, nrf24l_readConfig(FEATURE) | (1<<EN_DPL) );
  }
  // Enable dynamic payload on all pipes
  //
  // Not sure the use case of only having dynamic payload on certain
  // pipes, so the library does not support it.
  write_register(DYNPD,read_register(DYNPD) | _BV(DPL_P5) | _BV(DPL_P4) | _BV(DPL_P3) | _BV(DPL_P2) | _BV(DPL_P1) | _BV(DPL_P0));

  dynamic_payloads_enabled = true;
}
*/

/**
 * Test whether there was a carrier on the line for the
 * previous listening period.
 *
 * Useful to check for interference on the current channel.
 *
 * @return true if was carrier, false if not
 */
uint8_t nrf24l_testCarrier(void){
    uint8_t tempvar;
   nrf24l_readRegister(CD,&tempvar,1);
   return (tempvar & 1);
}

/**
 * Set RF communication channel
 *
 * @param channel Which RF channel to communicate on, 0-127
 */
void nrf24l_setChannel(uint8_t channel){
    nrf24l_channel = channel;
    nrf24l_configRegister(RF_CH, nrf24l_channel);
}

/**
 * 
 * @param value
 */
void nrf24l_setPayload(uint8_t value){
    nrf24l_configRegister(EN_AA, 0x00);  // Enable Auto Acknowledgment.
    nrf24l_configRegister(DYNPD, 0x00);
    nrf24l_payload = value;
}

/**
 * Checks if data is available for reading
 * @return
 */
uint8_t nrf24l_dataReady() {
    // See note in getData() function - just checking RX_DR isn't good enough
    uint8_t status = nrf24l_getStatus();

    // We can short circuit on RX_DR, but if it's not set, we still need
    // to check the FIFO for any pending packets
    //if (status & (1 << RX_DR)) return 1;
    //return !nrf24l_rxFifoEmpty();
    uint8_t result = ( status & (1<<RX_DR) );

  if (result){
    // Clear the status bit
    // ??? Should this REALLY be cleared now?  Or wait until we
    // actually READ the payload?
    nrf24l_configRegister(STATUS,(1<<RX_DR));

    // Handle ack payload receipt
    if ( status & (1<<TX_DS) )
        nrf24l_configRegister(STATUS,(1<<TX_DS));
  }
  return result;   
}

/**
 * 
 * @return 
 */
uint8_t nrf24l_rxFifoEmpty() {
    uint8_t fifoStatus;

    nrf24l_readRegister(FIFO_STATUS, &fifoStatus, sizeof (fifoStatus));
    return (fifoStatus & (1 << RX_EMPTY));
}

/**
 * Reads nrf24l_payload bytes into data array
 * @param data
 */
void nrf24l_getData(uint8_t * data) {
    nrf24l_csnLow(); // Pull down chip select
    spi_transfer(R_RX_PAYLOAD); // Send cmd to read rx nrf24l_payload
    nrf24l_transferSync(data, data, nrf24l_payload); // Read payload
    nrf24l_csnHigh(); // Pull up chip select
    // NVI: per product spec, p 67, note c:
    //  "The RX_DR IRQ is asserted by a new packet arrival event. The procedure
    //  for handling this interrupt should be: 1) read payload through SPI,
    //  2) clear RX_DR IRQ, 3) read FIFO_STATUS to check if there are more 
    //  payloads available in RX FIFO, 4) if there are more data in RX FIFO,
    //  repeat from step 1)."
    // So if we're going to clear RX_DR here, we need to check the RX FIFO
    // in the dataReady() function
    nrf24l_configRegister(STATUS, (1 << RX_DR)); // Reset status register
}

/**
 * Clocks only one byte into the given nrf24l01p register
 *
 * @param reg
 * @param value
 */
void nrf24l_configRegister(uint8_t reg, uint8_t value) {
    nrf24l_csnLow();
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    spi_transfer(value);
    nrf24l_csnHigh();
}

/**
 * Reads an array of bytes from the given start position in the nrf24l01p registers.
 * @param reg
 * @param value
 * @param len
 */
void nrf24l_readRegister(uint8_t reg, uint8_t * value, uint8_t len) {
    nrf24l_csnLow();
    spi_transfer(R_REGISTER | (REGISTER_MASK & reg));
    nrf24l_transferSync(value, value, len);
    nrf24l_csnHigh();
}

/**
 * Writes an array of bytes into inte the nrf24l01p registers.
 * @param reg
 * @param value
 * @param len
 */
void nrf24l_writeRegister(uint8_t reg, uint8_t * value, uint8_t len) {
    nrf24l_csnLow();
    spi_transfer(W_REGISTER | (REGISTER_MASK & reg));
    nrf24l_transmitSync(value, len);
    nrf24l_csnHigh();
}

/**
 * Sends a data package to the default address. Be sure to send the correct
 * amount of bytes as configured as payload on the receiver.
 * @param value
 */
void nrf24l_send(uint8_t * value) {
    uint8_t status = 0;
    uint16_t i = 0;
    status = nrf24l_getStatus();

    while (nrf24l_PTX) {
        //status = nrf24l_getStatus();
        nrf24l_readRegister(OBSERVE_TX, &status, 1);    // just like the nRF24+ library.
        if ((status & ((1 << TX_DS) | (1 << MAX_RT)))) {
            nrf24l_PTX = 0;
            break;
        }
        if(i >= 500)
            break;      // timeout.
        delayms(1);
        i++;
    } // Wait until last paket is send or time out.

    nrf24l_ceLow();

    nrf24l_powerUpTx(); // Set to transmitter mode , Power up

    nrf24l_csnLow(); // Pull down chip select
    spi_transfer(FLUSH_TX); // Write cmd to flush tx fifo
    nrf24l_csnHigh(); // Pull up chip select

    nrf24l_csnLow(); // Pull down chip select
    spi_transfer(W_TX_PAYLOAD); // Write cmd to write nrf24l_payload
    nrf24l_transmitSync(value, nrf24l_payload); // Write payload
    nrf24l_csnHigh(); // Pull up chip select

    nrf24l_ceHigh(); // Start transmission
    delayms(15);
    nrf24l_ceLow();
}

/**
 * Test if chip is still sending.
 * When sending has finished return chip to listening.
 * @return
 */
uint8_t nrf24l_isSending() {
    uint8_t status;
    if (nrf24l_PTX) {
        status = nrf24l_getStatus();

        /*
         *  if sending successful (TX_DS) or max retries exceded (MAX_RT).
         */

        if ((status & ((1 << TX_DS) | (1 << MAX_RT)))) {
            nrf24l_powerUpRx();
            return nrf24l_false;
        }

        return nrf24l_true;
    }
    return nrf24l_false;
}

/**
 *
 * @return
 */
uint8_t nrf24l_getStatus() {
    uint8_t rv;
    nrf24l_readRegister(STATUS, &rv, 1);
    return rv;
}

/**
 *
 */
void nrf24l_powerUpRx() {
    nrf24l_PTX = 0;
    nrf24l_ceLow();
    nrf24l_configRegister(CONFIG, NRF24L_CONFIG | ((1 << PWR_UP) | (1 << PRIM_RX)));
    nrf24l_ceHigh();
    nrf24l_configRegister(STATUS, (1 << TX_DS) | (1 << MAX_RT));
    delayms(5);             // Tpd2stby
}

/**
 *
 */
void nrf24l_flushRx() {
    nrf24l_csnLow();
    spi_transfer(FLUSH_RX);
    nrf24l_csnHigh();
}

/**
 *
 */
void nrf24l_flushTx() {
    nrf24l_csnLow();
    spi_transfer(FLUSH_TX);
    nrf24l_csnHigh();
}

void nrf24l_startListening(){
    uint8_t tempConfig = 0;
    nrf24l_readRegister(CONFIG, &tempConfig, 1);
    nrf24l_configRegister(CONFIG, tempConfig | (1<<PWR_UP) | (1<<PRIM_RX));
    nrf24l_configRegister(STATUS, (1<<RX_DR) | (1<<TX_DS) | (1<<MAX_RT) );

    // Flush buffers
    nrf24l_flushTx();
    nrf24l_flushRx();
    nrf24l_ceHigh();
    // wait for the radio to come up (130us actually only needed)
    delayms(130);
}

void nrf24l_stopListening(){
    nrf24l_ceLow();
    nrf24l_flushRx();
    nrf24l_flushTx();
}

/**
 *
 */
void nrf24l_powerUpTx() {
    nrf24l_PTX = 1;
    nrf24l_configRegister(CONFIG, NRF24L_CONFIG | ((1 << PWR_UP) | (0 << PRIM_RX)));
    delayms(5);             // Tpd2stby
}

/**
 * 
 */
void nrf24l_powerDown() {
    nrf24l_ceLow();
    //nrf24l_configRegister(CONFIG, NRF24L_CONFIG);
    nrf24l_ceLow();
    uint8_t tempConfig = 0;
    nrf24l_readRegister(CONFIG, &tempConfig, 1);
    nrf24l_configRegister(CONFIG, (tempConfig & (0<<PWR_UP)));
}
