#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
 
static volatile int klaar = 0;

// Handler voor SIGINT wordt aangeroepen bij indrukken ctrl-c
void ctrl_c_handler(int signum)
{
	klaar = 1;
	printf("\nSignaal %d (SIGINT) ontvangen\n", signum);
}

// Globale file descriptor
static int fd = -1;

// Handler voor SIGIO wordt aangeroepen bij interrupt op switches (opgaande flank)
void sigio_handler(int signum) 
{
	printf("Signaal %d (SIGIO) ontvangen\n", signum);

	if(fd < 0) {
		fprintf(stderr, "ERROR: Device is not open\n");
	}
	else {
		int buf;
		read(fd, &buf, 1);
		printf("switches = 0x%02X\n", buf);
	}
}
 
int main()
{
	struct sigaction actie;
 
	// Handler voor SIGIO registreren
	if (sigemptyset(&actie.sa_mask) < 0) {
		perror("sigemptyset");
		exit(EXIT_FAILURE);
	}
	actie.sa_flags = (SA_RESTART);
	actie.sa_handler = sigio_handler; //pointer naar signal routine
	if (sigaction(SIGIO, &actie, NULL) < 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	// Handler voor SIGINT registreren
	if (sigemptyset(&actie.sa_mask) < 0) {
		perror("sigemptyset");
		return EXIT_FAILURE;
	}
	actie.sa_flags = (SA_RESETHAND);
	actie.sa_handler = ctrl_c_handler;
	if (sigaction(SIGINT, &actie, NULL) < 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	// Device openen
	fd = open("/dev/async_switches", O_RDONLY);
	if(fd < 0) {
		perror("open");
		return EXIT_FAILURE;
	}
 
	printf("Applicatie registreren voor SIGIO bij device\n");
	// Geef het process id van dit proces door aan het device zodat deze signals kan sturen 
	if (fcntl(fd, F_SETOWN, getpid()) < 0) {
		perror("fcntl");
		close(fd);
		return EXIT_FAILURE;
	}
	// Enable asynchronous notification zodat device signal SIGIO stuurt bij interrupt
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		perror("fcntl");
		close(fd);
		return EXIT_FAILURE;
	}
	if (fcntl(fd, F_SETFL, flags | FASYNC) < 0) {
		perror("fcntl");
		close(fd);
		return EXIT_FAILURE;
	}

	printf("Wacht op signaal...\n");
	while (!klaar);

	// Device sluiten
	close(fd);

	return EXIT_SUCCESS;
}
