#include <system.h>
#include <altera_avalon_pio_regs.h>

int main(void)
{
	while (1)
	{
		int SW_value = IORD_ALTERA_AVALON_PIO_DATA(SWITCHES_BASE);
		IOWR_ALTERA_AVALON_PIO_DATA(LEDS_BASE, SW_value);
	}
    return 0;
}
