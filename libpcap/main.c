#include <pcap.h>
#include <stdbool.h>

// 判断规则是否匹配数据包
bool isBPFRuleMatching(const char *bpfRule, const unsigned char *packet,
                       int packetLength) {
  pcap_t *pcap_handle;
  struct bpf_program fp;

  // 打开一个虚拟的 pcap 设备，可以根据实际需求修改参数
  pcap_handle = pcap_open_dead(DLT_EN10MB, 65535);

  // 编译 BPF 规则
  if (pcap_compile(pcap_handle, &fp, bpfRule, 0, PCAP_NETMASK_UNKNOWN) < 0) {
    pcap_close(pcap_handle);
    printf("Rule is illegal");
    return false; // 编译失败
  }

  // 执行匹配
  struct pcap_pkthdr header;
  header.caplen = packetLength;
  header.len = packetLength;

  if (pcap_offline_filter(&fp, &header, packet) != 0) {
    pcap_freecode(&fp);
    pcap_close(pcap_handle);
    return true; // 规则匹配
  }

  pcap_freecode(&fp);
  pcap_close(pcap_handle);
  return false; // 规则不匹配
}

int main() {
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

  // 示例 BPF 规则
  const char *bpfRule = "src host 192.168.11.169";

  const char *failBpfRule = "src host 192.168.11.111";

  // 判断规则是否匹配数据包
  if (isBPFRuleMatching(bpfRule, pkt_031, sizeof(pkt_031))) {
    printf("YES\n");
  } else {
    printf("NO\n");
  }

  if (isBPFRuleMatching(failBpfRule, pkt_031, sizeof(pkt_031))) {
    printf("NO\n");
  }

  return 0;
}
