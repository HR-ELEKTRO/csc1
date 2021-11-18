#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <signal.h>
 
static int klaar = 0;
 
void ctrl_c_handler(int n, siginfo_t *info, void *unused)
{
    if (n == SIGINT) {
        klaar = 1;
    }
}
 
//onze signal service routine
void SSR(int n, siginfo_t *info, void *unused)
{
    if (n == 10) {
        printf ("Signaal ontvangen met waarde =  %u\n", info->si_int);
    }
}
 
int main()
{
    int fd;
    int32_t nummer;
    struct sigaction actie;
 
    /* install custom signal handler */
    sigemptyset(&actie.sa_mask);
    actie.sa_flags = (SA_SIGINFO | SA_RESTART);
    actie.sa_sigaction = SSR; //pointer naar signal routine
    sigaction(10, &actie, NULL); //interrupt signaal 10

    /* ook handler voor ctrl-c maken */
    sigemptyset (&actie.sa_mask);
    actie.sa_flags = (SA_SIGINFO | SA_RESETHAND);
    actie.sa_sigaction = ctrl_c_handler;
    sigaction (SIGINT, &actie, NULL);
 
    fd = open("/dev/leds_n_switches", O_RDWR);
    if(fd < 0) {
            printf("Device bestand niet kunnen openen..\n");
            return 0;
    }
 
    printf("Applicatie registreren\n");
    /* ioctl magie :) */
    if (ioctl(fd, _IOW('a','a',int32_t*) ,(int32_t*) &nummer)) {
        printf("IOCTL magie fout\n");
        close(fd);
        exit(1);
    }
   
    printf("Wacht op signaal...\n");
 
    while (!klaar);
    printf("Device bestand sluiten\n");
    close(fd);
}
