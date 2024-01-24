#include <bpf_def.h>
#include <pcap.h>
#include <rte_bpf.h>
#include <rte_common.h>
#include <rte_eal.h>
#include <rte_errno.h>
#include <rte_malloc.h>
#include <rte_mbuf.h>
#include <rte_memory.h>
#include <stddef.h>
#include <stdio.h>

int main(int argc, char *argv[]) {

  /*31	Ether/IPv4/TCP
    192.168.11.169	202.157.180.190	TCP	78
    64161→1443[SYN] Seq = 2421401215 Win = 65535 Len = 0 MSS = 1460 WS = 16
    TSval = 1409022517 TSecr = 0 SACK_PERM = 1
*/
  static const unsigned char pkt_031[78] = {
      0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xdc, 0x9b, /* ........ */
      0x9c, 0xb2, 0x90, 0x51, 0x08, 0x00, 0x45, 0x00, /* ...Q..E. */
      0x00, 0x40, 0x74, 0x13, 0x40, 0x00, 0x40, 0x06, /* .@t.@.@. */
      0x7a, 0xf7, 0xc0, 0xa8, 0x0b, 0xa9, 0xca, 0x9d, /* z....... */
      0xb4, 0xbe, 0xfa, 0xa1, 0x05, 0xa3, 0x90, 0x53, /* .......S */
      0xa6, 0x7f, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x02, /* ........ */
      0xff, 0xff, 0x66, 0x07, 0x00, 0x00, 0x02, 0x04, /* ..f..... */
      0x05, 0xb4, 0x01, 0x03, 0x03, 0x04, 0x01, 0x01, /* ........ */
      0x08, 0x0a, 0x53, 0xfb, 0xfa, 0x35, 0x00, 0x00, /* ..S..5.. */
      0x00, 0x00, 0x04, 0x02, 0x00, 0x00              /* ...... */
  };

  int ret = rte_eal_init(argc, argv);
  if (ret < 0) {
    printf("Error message: %s\n", rte_strerror(rte_errno));
    return -1;
  }

  struct rte_mbuf *pkt_buf = rte_zmalloc(NULL, sizeof(struct rte_mbuf), 0);

  if (pkt_buf == NULL) {
    printf("Error message: %s\n", rte_strerror(rte_errno));
    return -1;
  }

  pkt_buf->buf_len = sizeof(pkt_031);
  pkt_buf->buf_addr = (void *)(const char *)pkt_031;
  pkt_buf->data_off = 0;
  pkt_buf->data_len = sizeof(pkt_031);
  pkt_buf->pkt_len = sizeof(pkt_031);
  pkt_buf->nb_segs = 1;
  pkt_buf->next = NULL;

  pcap_t *pcap_handle;
  struct bpf_program bp;
  const char *bpfRule = "src host 192.168.11.169";
  pcap_handle = pcap_open_dead(DLT_EN10MB, 65535);
  if (pcap_compile(pcap_handle, &bp, bpfRule, 0, PCAP_NETMASK_UNKNOWN) < 0) {
    pcap_close(pcap_handle);
    printf("Rule is illegal");
    return false; // 编译失败
  }

  struct rte_bpf_prm *rbp = rte_bpf_convert(&bp);
  struct rte_bpf *rb = rte_bpf_load(rbp);
  // The return code from bpf capture filter is non-zero if matched
  uint64_t rc = rte_bpf_exec(rb, pkt_buf);
  printf("rc: %" PRIu64 "\n", rc);
}
