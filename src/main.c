#include <pcap/pcap.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

#define SERVER_UDP_PORT 	8765	
#define MAXLEN				65535	

/*
 * Global Variables
 */
int sd; /* The socket descriptor used to comunicate with the client device. */
struct sockaddr_in client; /* The client socket. */
pcap_t* nic_descr; /* The network capture interface. */

struct completePacket
{
	uint32_t sec;
	uint32_t usec;
	uint32_t inclen;
	uint32_t totallen;
	char data[MAXLEN];
} p;

/* This fuction is run in a thread to receive pings from the client to
 * monitor if the client is still active. If the receiving fails or 
 * times out the pcap loop is broken and the thread returns 
 */
void *socketHeartBeat( void *ptr __attribute__((unused)))
{
	fd_set rfds;
    struct timeval tv;
    int retval;
	char buf[MAXLEN];
	int n;
	
	/* Loop until an error occurs */
	while(1)
	{
		FD_ZERO(&rfds);
		FD_SET(sd, &rfds);
	
		tv.tv_sec = 6;
		tv.tv_usec = 0;
	
	   retval = select(sd + 1, &rfds, NULL, NULL, &tv);
	
	   if (retval == -1)
			perror("select()");
		else if (retval)
		{
			if (FD_ISSET(sd, &rfds))
			{
				socklen_t client_len = sizeof(client);
				if ((n = recvfrom (sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0)
				{
					 /* The recv call failed so the socket is likely closed. */
					pcap_breakloop(nic_descr);
					errno = 0;
					return 0;
				}
			}
		}
		else /* The select() call timed out, the client has likely disconnected. */
		{
			pcap_breakloop(nic_descr);
			errno = 0;
			return 0;	
		}		
	}
}

/* The callback for when a packet is captured on the network interface. The packet is then sent to the client
 * on a UDP socket.
 */
void pkt_callback(u_char *ptr_null __attribute__((unused)), const struct pcap_pkthdr* pkthdr, const u_char* packet)
{
	unsigned int size = sizeof(struct pcap_pkthdr) + pkthdr->caplen;
	p.sec = pkthdr->ts.tv_sec;
	p.usec = pkthdr->ts.tv_usec;
	p.inclen = pkthdr->caplen;
	p.totallen = pkthdr->len;
	memcpy(&(p.data), packet, pkthdr->caplen);
	if (sendto (sd, &p, size, 0,(struct sockaddr *)&client, sizeof(client)) != size)
	{
		if (errno != EAGAIN)
			pcap_breakloop(0);
		errno = 0;
	}
}

/* The usage message */
void usage(char* name)
{
	fprintf(stderr, "Usage: %s [-f pcap filter] [-i interface]\n", name); 
	fprintf(stderr, "  pcap filter  A filter using the libpcap format,\n"
					"               filters with spaces should be in quotes.\n");
	fprintf(stderr, "  interface    The name of the network interface to capture on.\n"); 
	exit(1);
}

/* Setup the packet capture device and compile the filter. */
void setupPcap(char* nic_dev, char* filter)
{
	char completeFilter[MAXLEN];
	struct bpf_program fp;      /* holds compiled program */              
	bpf_u_int32 netp;           /* ip        */
	bpf_u_int32 maskp;          /* subnet mask  */
	char errbuf[PCAP_ERRBUF_SIZE];
	
	/* Lookup the NIC if it was not specified on the command line. */
	if(nic_dev == NULL) 
		nic_dev = pcap_lookupdev (errbuf);
	
	if (nic_dev == NULL)
	{ 
		fprintf(stderr, "%s\n",errbuf); 
		exit(1); 
	}
	
	
	/* Use pcap to get the IP address and subnet mask of the device */
	pcap_lookupnet (nic_dev, &netp, &maskp, errbuf);

			
	/* open device for reading */
	nic_descr = pcap_open_live (nic_dev, BUFSIZ, 0, -1, errbuf);
	if (nic_descr == NULL)
	{ 
		printf("pcap_open_live(): %s\n",errbuf); 
		exit(1); 
	}

	/* Set the filter to not capture the programs own traffic */
	if (filter == NULL)
		strcpy(completeFilter, "not udp port 8765");
	else
		sprintf(completeFilter, "not udp port 8765 and (%s)", filter); /* Add user specificed filter */
		
	/* Compile the filter expression */
	if (pcap_compile (nic_descr, &fp, completeFilter, 0, netp) == -1)
	{ 
		fprintf(stderr,"Invalid Filter\n"); 
		exit(1);
	}

	/* Load the filter into the capture device */
	if (pcap_setfilter (nic_descr, &fp) == -1)
	{ 
		fprintf(stderr,"Error setting filter\n"); 
		exit(1); 
	}
	
}

/* Create the socket and bind it. */
void setupSocket()
{
	struct	sockaddr_in server;
	/* Create a datagram socket */
	if ((sd = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == -1)
	{
		perror ("Can't create a socket"); 
		exit(1);
	}

	/* Bind an address to the socket */
	bzero((char *)&server, sizeof(server)); 
	server.sin_family = AF_INET; 
	server.sin_port = htons(SERVER_UDP_PORT); 
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind (sd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		perror ("Can't bind name to socket");
		exit(1);
	}
	
}

int main(int argc, char** argv)
{
	char *nic_dev = NULL; 
	char *filter = NULL;
	socklen_t client_len;
	pthread_t thread;
	int c;
	int n;
	char buf[MAXLEN];

	while ((c = getopt (argc, argv, "f:i:")) != -1)
	switch (c)
	{
		case 'f':
			filter = optarg;
			break;
		case 'i':
			nic_dev = optarg;
			break;			
		default:
			usage(argv[0]);
			break;
	}

	setupSocket();
	setupPcap(nic_dev, filter);
	
	/* Loop until the process is killed */
	while(1)
	{	
		printf("\r                                     ");
		printf("\rNo Client Connected");
		fflush(stdout);
		
		/* Wait for a 'syn' message from the client */
		client_len = sizeof(client);
		if ((n = recvfrom (sd, buf, MAXLEN, 0, (struct sockaddr *)&client, &client_len)) < 0)
		{
			perror ("recvfrom error");
			exit(1);
		}
		
		if(n != 3) /* The expected packet data is 'syn' anything else can be ignored. */
			continue;
		
		/* Return the same packet to let the client know the server is ready. */
		if ( sendto (sd, buf, n, 0,(struct sockaddr *)&client, client_len) != n)
		{
			perror("sendto error");
			exit(1);
		}
			
		printf("\r                                      ");
		printf("\rClient Connected from %s", inet_ntoa(client.sin_addr));
		fflush (stdout);
		
		/* Create a thread to recieve the client heartbeat packets to detect when the client has disconnected */
		pthread_create( &thread, NULL, socketHeartBeat, NULL);
	
		/* Call pcap_loop and  with the callback */
		pcap_loop(nic_descr, 0, pkt_callback, NULL);
		
		/* Wait for the heartbeat thread to complete. */
		pthread_join(thread, NULL);	
	}
	return 0;
}
