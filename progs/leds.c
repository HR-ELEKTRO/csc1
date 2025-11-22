#include <stdio.h>
#include <unistd.h>

int main() {
	FILE *fp = fopen("/dev/leds", "w");
	if (fp != NULL) {
		int i;
		for (i = 0; i < 17; i++) {
			printf("Send 0x%02X to the leds\n", i);
			fputc(i, fp);
			fflush(fp);
			sleep(1);
		}
		fclose(fp);
	}
	else {
		printf("Error: can not open /dev/leds\n");
	}
	return 0;
}
