#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "receiver.h"

/* reverse:  reverse string s in place */
void reverse(char s[])
{
    int i, j;
    char c;

    for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
        c = s[i];
        s[i] = s[j];
        s[j] = c;
    }
}


void m_itoa(int n, char s[])
{
    int i, sign;

    if ((sign = n) < 0)  /* record sign */
        n = -n;          /* make n positive */
    i = 0;
    do {       /* generate digits in reverse order */
        s[i++] = n % 10 + '0';   /* get next digit */
    } while ((n /= 10) > 0);     /* delete it */
    if (sign < 0)
        s[i++] = '-';
    s[i] = '\0';
    reverse(s);
}


int sender(int nAddr, int nStream, float bandwidth, int packet_size, int test_time, int test_inc, char *addr, int start_port, int verbose){
    int i, j, pid, k;
	char bw[20];
	sprintf(bw, "%fm",bandwidth);
	char ttime[8];
	m_itoa(test_time, ttime);
	char pktsize[8];
	m_itoa(packet_size, pktsize);
	for (k = 0; k < 8; k++){
		if (pktsize[k] == '\0'){
			pktsize[k] = 'B';
			pktsize[k + 1] = '\0';
			break;
		}
	}
	k = 1;
	if (test_inc == 0) k = nAddr;
	int n = 1;
	signal(SIGCHLD, SIG_IGN);
	while(k <= nAddr ){	
		char *plural_s = "";
		if (nStream > 1) plural_s = "s";
		char *plural_a = "";
		if (nAddr > 1) plural_a = "es";
		if (verbose == 1) printf("Test %d: Sending to %d/%d Address%s (starting at %s:%d) over %d stream%s, at %.2f mbps for %d seconds.\n", n, k, nAddr, plural_a, addr, start_port,nStream,plural_s, bandwidth, test_time);
		for(i = 0; i < k; i++) {
			for (j = 0; j < nStream; j++){
				pid = fork();
			    if(pid < 0) {
			        printf("Error");
			        return 0;
			    } else if (pid == 0) {
					int port = start_port + i *nStream + j;
					char p[6];
					m_itoa(port, p);
					
					char fn[20];
					sprintf(fn, "%d.%d.%d.txt",i,j,k);					
					execl("/usr/bin/iperf", "/usr/bin/iperf", "-c", increment_address(addr, i), "-u", "-T", "32", "-l", pktsize, "-p", p, "-x", "CDMSV", "-t", ttime, "-b", bw,"-o",fn, NULL);
			        exit(0);
			    } else  {
					continue;
			    }
			}
		}
		n++;
		sleep(test_time + 4);		
		if (k == nAddr) break;
		if (k == 1 && test_inc != 1) k = 0;
		k += test_inc;
		if (k > nAddr) k = nAddr;
	}
	return 0;
}