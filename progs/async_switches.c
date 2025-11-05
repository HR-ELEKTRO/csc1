#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
 
static volatile int klaar = 0;

// Handler voor SIGINT wordt aangeroepen bij indrukken ctrl-c
void ctrl_c_handler(int)
{
	klaar = 1;
	printf("\nSignaal SIGINT ontvangen\n");
}

// globale file descriptor
static int fd = -1;

// Handler voor SIGIO wordt aangeroepen bij interrupt op switches (opgaande flank)
void sigio_handler(int n) 
{
	printf("Signaal SIGIO ontvangen\n");
	// Vul hier de code in om de switch status te lezen en af te drukken
}
 
int main()
{
	struct sigaction actie;
 
	// Handler voor SIGIO registreren
	sigemptyset(&actie.sa_mask);
	actie.sa_flags = (SA_RESTART);
	actie.sa_handler = sigio_handler; //pointer naar signal routine
	sigaction(SIGIO, &actie, NULL);

	// Handler voor SIGINT registreren
	sigemptyset(&actie.sa_mask);
	actie.sa_flags = (SA_RESETHAND);
	actie.sa_handler = ctrl_c_handler;
	sigaction(SIGINT, &actie, NULL);
 
	// Device openen
	fd = open("/dev/async_switches", O_RDONLY);
	if(fd < 0) {
		perror("open");
		return 0;
	}
 
	printf("Applicatie registreren voor SIGIO bij device\n");
	// Geef het process id van dit proces door aan het device zodat deze signals kan sturen 
	if (fcntl(fd, F_SETOWN, getpid()) < 0) {
		perror("fcntl");
		close(fd);
		exit(EXIT_FAILURE);
	}
	// Enable asynchronous notification zodat device signal SIGIO stuurt bij interrupt
	int flags = fcntl(fd, F_GETFL);
	if (flags < 0) {
		perror("fcntl");
		exit(EXIT_FAILURE);
	}
	if (fcntl(fd, F_SETFL, flags | FASYNC) < 0) {
		perror("fcntl");
		exit(EXIT_FAILURE);
	}

	printf("Wacht op signaal...\n");
	while (!klaar);

	// device sluiten
	close(fd);

	exit(EXIT_SUCCESS);
}
