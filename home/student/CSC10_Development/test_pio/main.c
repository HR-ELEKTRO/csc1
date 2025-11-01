#include <stdio.h>
#include <unistd.h>
#include <stdint.h>
#include <fcntl.h>
#include <sys/mman.h>

#define HW_REGS_BASE (0xff200000)
#define HW_REGS_SPAN (0x00200000)
#define HW_REGS_MASK (HW_REGS_SPAN - 1)
#define LED_PIO_BASE (0x10040)
#define SW_PIO_BASE (0x100C0)

#define HWREG(x) (*(volatile uint32_t *)(x))

int main(void)
{
	// open /dev/mem
	int fd = open("/dev/mem", (O_RDWR | O_SYNC));
	if (fd == -1) {
		fprintf(stderr, "ERROR: could not open \"/dev/mem\"...\n");
		return -1;
	}
	// couple fysical memory to virtual memory
	void *virtual_base = mmap( NULL, HW_REGS_SPAN, (PROT_READ | PROT_WRITE), MAP_SHARED, fd, HW_REGS_BASE);
	if (virtual_base == MAP_FAILED) {
		fprintf(stderr, "ERROR: mmap() failed...\n");
		close(fd);
		return -1;
	}
	// Set initial value of PIO register to 0
	HWREG(virtual_base + LED_PIO_BASE) = 0;

	uint8_t teller = 0;
	while (teller++ < 255){
		// wait until KEY0 is released
		while (HWREG(virtual_base + SW_PIO_BASE) & 1);
		printf("Ophogen... %d\n", teller);

		// Add 1 to the PIO register
		HWREG(virtual_base + LED_PIO_BASE) += 1;
		usleep(300000);
	}

	if (munmap(virtual_base, HW_REGS_SPAN) != 0) {
		fprintf(stderr, "ERROR: munmap() failed...\n");
		close(fd);
		return 1;
	}

	close(fd);
	return 0;
}
