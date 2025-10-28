#include <stdio.h>
#include <unistd.h>

int main() {
	FILE *fp = fopen("/dev/leds", "r");
	if (fp != NULL) {
		char buffer;
		fread(&buffer, sizeof(char), 1, fp);
		printf("Read character code 0x%02X", (int)buffer);
	}
	else {
		printf("Error: can not open /dev/leds\n");
		perror("Error message");
	}
	return 0;
}
