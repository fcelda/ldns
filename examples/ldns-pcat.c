#include "config.h"

#include <ldns/dns.h>
#include <pcap.h>

#ifndef ETHERTYPE_IPV6
#define ETHERTYPE_IPV6 	0x86dd
#endif
#define DNS_UDP_OFFSET 	42

#define DIFF_VERSION 	"1.0"

/* output in the following manner. All numbers are decimal, data is in hex.
 * sequence number of the packet, starting at 0. Newline
 * query size. Newline
 * query data in hex. Newline
 * answer size. Newline
 * answer data in hex. Newline
 * Thus:
 * seq\n
 * qsize\n
 * qdata\n
 * asize\n
 * adata\n
 */

void
usage(FILE *fp, char *progname)
{
	fprintf(fp, "%s: -l LOG [-a IP] [-p PORT] PCAP_FILE\n\n", progname);
	fprintf(fp, "   -a IP\tuse IP as nameserver\n");
	fprintf(fp, "   -p PORT\tuse PORT as port, defaults to 53\n");
	fprintf(fp, "   -l STR\tuse STR as header, defaults to 127.0.0.1\n");
	fprintf(fp, "  PCAP_FILE\tuse this file as source\n");
	fprintf(fp, "  If no file is given standard input is read\n");
	fprintf(fp, "\nOUTPUT FORMAT v"DIFF_VERSION "\n");
	fprintf(fp, "   The output is line based and each line is ended with a newline:\n");
	fprintf(fp, "    ; header information\n");
	fprintf(fp, "    (decimal) pkt sequence number\n");
	fprintf(fp, "    (decimal) number of hex characters of query\n");
	fprintf(fp, "    hex dump of query\n");
	fprintf(fp, "    (decimal) number of hex characters of reply\n");
	fprintf(fp, "    hex dump of reply\n");
}

void
data2hex(FILE *fp, u_char *p, size_t l)
{
	size_t i;
	for(i = 0; i < l; i++) {
		/* do it in network order */
		fprintf(fp, "%02x", p[i]);
		fputs(" ", fp);
	}
	fputs("\n", fp);
}

u_char *
pcap2ldns_pkt_ip(const u_char *packet, struct pcap_pkthdr *h)
{
	h->caplen -= DNS_UDP_OFFSET;
	if (h->caplen < 0) {
		return NULL;
	} else {
		return (u_char*)(packet + DNS_UDP_OFFSET);
	}
}

u_char *
pcap2ldns_pkt(const u_char *packet, struct pcap_pkthdr *h)
{
	struct ether_header *eptr;

	eptr = (struct ether_header *) h;
	switch(eptr->ether_type) {
		case ETHERTYPE_IP:
			return pcap2ldns_pkt_ip(packet, h);
			break;
		case ETHERTYPE_IPV6:
			break;
		case ETHERTYPE_ARP:
			fprintf(stderr, "ARP pkt, dropping\n");
			break;
		default:
			fprintf(stderr, "Not IP pkt, dropping\n");
			break;
	}
	return 0;
}

int
main(int argc, char **argv) 
{
	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t *p;
	struct pcap_pkthdr h;
	const u_char *x;
	char *progname;
	size_t i = 0;
	ldns_rdf *ip;
	char *ip_str;
	int c;

	uint8_t *result;
	uint16_t port;
	char *log;
	ldns_buffer *qpkt;
	u_char *q;
	size_t size;
	socklen_t tolen;

	struct timeval timeout;
	struct sockaddr_storage *data;
	struct sockaddr_in  *data_in;

	port = 0;
	ip = NULL;
	ip_str = NULL;
	progname = strdup(argv[0]);
	log = NULL;

	while ((c = getopt(argc, argv, "a:p:l:")) != -1) {
		switch(c) {
		case 'a':
			ip_str = optarg;
			ip = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, optarg);
			if (!ip) {
				fprintf(stderr, "-a requires an IP address\n");
				exit(EXIT_FAILURE);
			}
			break;
		case 'l':
			log = optarg;
			break;
		case 'p':
			port = atoi(optarg);
			if (port == 0) {
				fprintf(stderr, "-p requires a port number\n");
				exit(EXIT_FAILURE);
			}
			break;
		default:
			usage(stdout, progname);
			exit(EXIT_FAILURE);
		}
	}
	argc -= optind;
	argv += optind;

	if (port == 0)
		port = 53;

	if (!log) {
		fprintf(stderr, "No log msg given. This is mandatory, use the -l switch\n"); 
		usage(stdout, progname);
		exit(EXIT_FAILURE);
	}

	if (!ip) {
		ip_str = "127.0.0.1";
		ip = ldns_rdf_new_frm_str(LDNS_RDF_TYPE_A, "127.0.0.1");
	}

	if (argc < 1) {
		/* no file given - use standard input */
		p = pcap_fopen_offline(stdin, errbuf);
	} else {
		p = pcap_open_offline(argv[0], errbuf);
	}
	if (!p) {
		fprintf(stderr, "Cannot open pcap lib %s\n", errbuf);
		exit(EXIT_FAILURE);
	}

	qpkt = ldns_buffer_new(LDNS_MAX_PACKETLEN);
	data = LDNS_MALLOC(struct sockaddr_storage);
	timeout.tv_sec = 2;
	timeout.tv_usec = 0;

	/* setup the socket */
	data->ss_family = AF_INET;
        data_in = (struct sockaddr_in*) data;
        data_in->sin_port = (in_port_t)htons(port);
        memcpy(&(data_in->sin_addr), ldns_rdf_data(ip), ldns_rdf_size(ip));
        tolen = sizeof(struct sockaddr_in);

	fprintf(stdout, ";v%s `%s\'  %s:%d\n", DIFF_VERSION, log, ip_str, port);
	i = 1;  /* start counting at 1 */
	while ((x = pcap_next(p, &h))) {
		q = pcap2ldns_pkt_ip(x, &h);
		ldns_buffer_write(qpkt, q, h.caplen);

		if (ldns_udp_send(&result, qpkt, data, tolen, timeout, &size) ==
				LDNS_STATUS_OK) {
			/* double check if we are dealing with correct replies 
			 * by converting to a pkt... todo */
			fprintf(stdout, "%zd\n%zd\n", i, h.caplen);
			/* query */
			data2hex(stdout, q, h.caplen); 
			/* answer */
			fprintf(stdout, "%zd\n", size);
			data2hex(stdout, result, size);
		} else {
			fprintf(stderr, "Failure to send packet\n");
		}
		ldns_buffer_clear(qpkt);
		i++;
	}
	pcap_close(p);
	return 0;
}
