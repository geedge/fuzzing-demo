#include <arpa/inet.h>
#include <netinet/if_ether.h>
#include <pcap.h>
#include <stdio.h>

void process_packet(const struct pcap_pkthdr *header, const u_char *packet) {
  printf("Packet capeln length: %d bytes\n", header->caplen);

  struct ethhdr *eth = (struct ethhdr *)packet;

  // 打印源MAC地址
  printf("Source MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_source[0],
         eth->h_source[1], eth->h_source[2], eth->h_source[3], eth->h_source[4],
         eth->h_source[5]);

  // 打印目标MAC地址
  printf("Destination MAC: %02x:%02x:%02x:%02x:%02x:%02x\n", eth->h_dest[0],
         eth->h_dest[1], eth->h_dest[2], eth->h_dest[3], eth->h_dest[4],
         eth->h_dest[5]);
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <pcapng file>\n", argv[0]);
    return 1;
  }

  char errbuf[PCAP_ERRBUF_SIZE];
  pcap_t *handle = pcap_open_offline(argv[1], errbuf);

  if (handle == NULL) {
    fprintf(stderr, "Error opening file: %s\n", errbuf);
    return 1;
  }

  struct pcap_pkthdr header;
  const u_char *packet;

  while ((packet = pcap_next(handle, &header)) != NULL) {
    process_packet(&header, packet);
  }

  pcap_close(handle);
  return 0;
}
