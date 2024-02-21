#include <avr/io.h>
#include <util/delay.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <avr/eeprom.h>

volatile static uint8_t rx_buffer[USB_EP_SIZE] = {0};
volatile static uint16_t rx_count = 0;
volatile static uint8_t uart_tx_busy = 1;
ISR(USART_RX_vect)
{

    volatile static uint16_t rx_write_pos = 0;

    rx_buffer[rx_write_pos] = UDR0;
    rx_count++;
    rx_write_pos++;
    if (rx_write_pos >= USB_EP_SIZE)
    {
        rx_write_pos = 0;
    }
}

ISR(USART_TX_vect)
{
    uart_tx_busy = 1;
}

void uart_init(uint32_t baud, uint8_t high_speed)
{

    uint8_t speed = 16;

    if (high_speed != 0)
    {
        speed = 8;
        UCSR0A |= 1 << U2X0;
    }

    baud = (F_CPU / (speed * baud)) - 1;

    UBRR0H = (baud & 0x0F00) >> 8;
    UBRR0L = (baud & 0x00FF);

    UCSR0B |= (1 << TXEN0) | (1 << RXEN0) | (1 << TXCIE0) | (1 << RXCIE0);
}

void uart_send_byte(uint8_t c)
{
    while (uart_tx_busy == 0)
        ;
    uart_tx_busy = 0;
    UDR0 = c;
}

void uart_send_array(uint8_t *c, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        uart_send_byte(c[i]);
    }
}

void uart_send_string(uint8_t *c)
{
    uint16_t i = 0;
    do
    {
        uart_send_byte(c[i]);
        i++;

    } while (c[i] != '\0');
    uart_send_byte(c[i]);
}

uint16_t uart_read_count(void)
{
    return rx_count;
}

uint8_t uart_read(void)
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
#define MAX_EEPROM_SIZE 2048
int main(void)
{
    const uint8_t start[] = "Program Start\n\r";
    const uint8_t end[] = "Program End\n\r";
    uint8_t data = 'A';

    uart_init(2400, 0);
    sei();
    uart_send_string(start);
    uint8_t *s = 64;
    uint8_t *index = s;
    uint16_t counter = 0;
    while (1)
    {
        if (counter > MAX_EEPROM_SIZE)
            break;
        if (uart_read_count() > 0)
        {

            data = uart_read();
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
        uart_send_byte(c);
    }
}