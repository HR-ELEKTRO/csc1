#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
 
int main()
{
	sigset_t signal_set;
 
	// Maak een set van signals aan
	if (sigemptyset(&signal_set) < 0) {
		perror("sigemptyset");
		exit(EXIT_FAILURE);
	}
	// SIGINT wordt verstuurd bij indrukken ctrl-c
	if (sigaddset(&signal_set, SIGINT) < 0) {
		perror("sigaddset");
		exit(EXIT_FAILURE);
	}
	// SIGIO wordt aangeroepen bij interrupt op switches (opgaande flank)
	if (sigaddset(&signal_set, SIGIO) < 0) {
		perror("sigaddset");
		exit(EXIT_FAILURE);
	}
	// Block signals in signal_set (SIGINT en SIGIO) zodat we er in main op kunnen wachten
	if (sigprocmask(SIG_BLOCK, &signal_set, NULL) < 0) {
		perror("sigprocmask");
		exit(EXIT_FAILURE);
	}

	// Device openen
	int fd = open("/dev/async_switches", O_RDONLY);
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
	bool klaar = false;
	while (!klaar) {
		int sig_number;
		sigwait(&signal_set, &sig_number);
		if (sig_number == SIGIO) {
				ssize_t buf;
				if (read(fd, &buf, 1) < 0) {
					perror("read");
					close(fd);
					return EXIT_FAILURE;
				}
				printf("Signaal %d (SIGIO) ontvangen switches = 0x%02X\n", sig_number, buf);
		}
		else if (sig_number == SIGINT) {
			printf("Signaal %d (SIGINT) ontvangen\n", sig_number);
			klaar = true;
		}
	}

	// Device sluiten
	close(fd);

	return EXIT_SUCCESS;
}
