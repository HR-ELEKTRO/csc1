#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HW_REGS_BASE ( 0xff200000 )
#define HW_REGS_SPAN ( 0x00200000 )
#define HW_REGS_MASK ( HW_REGS_SPAN - 1 )
#define LED_PIO_BASE ( 0x10040 )
#define SW_BASE 0x100C0

#define HWREG(x) (*((volatile uint32_t *)(x)))

int main(void)
{
	volatile unsigned int *h2p_lw_led_addr=NULL;
	void *virtual_base;
	int fd;

	// Open /dev/mem
	if( ( fd = open( "/dev/mem", ( O_RDWR | O_SYNC ) ) ) == -1 ) {
			printf( "ERROR: could not open \"/dev/mem\"...\n" );
			return( 1 );
	}

	// Hiermee koppelen we het fysieke geheugen aan het virtuele geheugen
	// en krijgen we de pointer terug
	virtual_base = mmap( NULL, HW_REGS_SPAN, ( PROT_READ | PROT_WRITE ),
						MAP_SHARED, fd, HW_REGS_BASE );
	if( virtual_base == MAP_FAILED ) {
		printf( "ERROR: mmap() failed...\n" );
		close( fd );
		return(1);
	}


	HWREG(virtual_base + LED_PIO_BASE) = 0;

	uint8_t teller =0;
	while(teller++<255)
	{
		//zolang KEY0 niet is ingedrukt
		while(HWREG(virtual_base+SW_BASE) & 1);

		//knop ingedrukt dus verder
		printf("Ophogen... %d\n", teller);

		// Add 1 to the PIO register
		HWREG(virtual_base + LED_PIO_BASE) +=1;
		usleep(300000);
	}

	if( munmap( virtual_base, HW_REGS_SPAN ) != 0 ) {
		printf( "ERROR: munmap() failed...\n" );
		close( fd );
		return( 1 );
	}

	close( fd );
	return 0;
}

