#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

// Define a buffer to store received bytes
volatile static uint8_t rx_buffer[USB_EP_SIZE] = {0};

// Track the number of received bytes
volatile static uint16_t rx_count = 0;

// Flag indicating UART transmitter status
volatile static uint8_t uart_tx_busy = 1;

// ISR for USART Receive Complete
ISR(USART_RX_vect)
{
    // Track position to write into the receive buffer
    volatile static uint16_t rx_write_pos = 0;

    // Read received byte and store in buffer
    rx_buffer[rx_write_pos] = UDR0;

    // Increment received byte count
    rx_count++;

    // Move to next buffer position, wrap if necessary
    if (++rx_write_pos >= USB_EP_SIZE)
        rx_write_pos = 0;
}

// ISR for USART Transmit Complete
ISR(USART_TX_vect)
{
    // Set flag to indicate UART busy transmitting
    uart_tx_busy = 1;
}
class UART
{
private:
public:
    // Initialize USART with given baud rate
    void init(uint32_t baud = 2400)
    {
        // Default speed divisor
        uint8_t speed = 16;

        // Calculate baud rate register value
        baud = (F_CPU / (speed * baud)) - 1;

        // Set baud rate registers
        UBRR0H = (baud & 0x0F00) >> 8;
        UBRR0L = (baud & 0x00FF);

        // Enable transmitter, receiver, and corresponding interrupts
        UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << TXCIE0) | (1 << RXCIE0);
    }

    void send(uint8_t c)
    {
        // wait till not busy
        while (uart_tx_busy == 0)
            ;
        uart_tx_busy = 0;
        UDR0 = c;
    }
    void send(uint8_t *c, uint16_t len)
    {
        for (uint16_t i = 0; i < len; i++)
            send(c[i]);
    }

    void send(uint8_t *c)
    {
        uint16_t i = 0;
        do
        {
            send(c[i]);
            i++;

        } while (c[i] != '\0');
        send(c[i]);
    }
    // gives number of bytes to be read
    uint16_t read_count()
    {
        return rx_count;
    }
    // reads a byte
    uint8_t read()
    {
        static uint16_t rx_read_pos = 0;
        uint8_t data = 0;

        data = rx_buffer[rx_read_pos];
        rx_read_pos++;
        rx_count--;
        if (rx_read_pos >= USB_EP_SIZE)
        {
            rx_read_pos = 0;
        }
        return data;
    }
};
#define MAX_EEPROM_SIZE 2048

int main(void)
{
    // start and end messages
    const uint8_t start[] = "Program Start\n\r";
    const uint8_t end[] = "Program End\n\r";
    uint8_t data = 'A';

    // create uart object and init
    UART uart;
    uart.init();

    // enable interrupts
    sei();

    uart.send((uint8_t *)start);
    // initialize start address for EEPROM
    uint8_t *s = 64;
    uint8_t *index = s;
    uint16_t counter = 0;
    while (1)
    {
        // if overflow exit
        if (counter > MAX_EEPROM_SIZE)
            break;
        if (uart.read_count() > 0)
        {

            data = uart.read();
            if (data == '\r')
                break;
            eeprom_write_byte(index, data);
            index++;
            counter++;
        }
    }
    index++;
    eeprom_write_byte(index, '\r');
    index = s;
    auto c = eeprom_read_byte(index);
    index++;
    while (c != '\r')
    {
        c = eeprom_read_byte(index);
        index++;
        uart.send(c);
    }
}
