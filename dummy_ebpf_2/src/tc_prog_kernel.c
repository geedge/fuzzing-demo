#include <linux/types.h>

#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <linux/pkt_cls.h>
#include <xdp/parsing_helpers.h>

struct {
  __uint(type, BPF_MAP_TYPE_HASH);
  __type(key, int);
  __type(value, int);
  __uint(max_entries, 256);
} dummy_redirect_map SEC(".maps");

SEC("tc")
int tc_redirect_map_func(struct __sk_buff *skb) {
  int ret = 0;
  int action = TC_ACT_OK;
  void *data = (void *)(long)skb->data;
  void *data_end = (void *)(long)skb->data_end;

  bpf_printk("Receive a packet");

#if 0
  char key;
  unsigned int offset = data_end - data - 1;
  ret = bpf_skb_load_bytes(skb, offset, &key, sizeof(key));
  if (ret < 0) {
    bpf_printk("bpf_skb_load_bytes failed. return code:%d", ret);
    action = TC_ACT_OK;
    goto out;
  }
#endif

  // According to the last character, determine which interface to forward the data packet
  if (data_end - data < sizeof(struct ethhdr) + 1) {
    bpf_printk("Packet length is insufficient");
    goto out;
  }

  /**
   * Why do we need to calculate with 0x3fff here?
   * Because there's a hole here: when getting the last character, the usual way will fail to load.
   * ref:https://github.com/gamemann/Xdp-Access-Last-Byte
   */
  __u32 ofs = (data_end - data - 1) & 0x3fff;
  void *mgc = data + ofs;
  if ((mgc + 1) > data_end) {
    bpf_printk("Offset calculation error");
    goto out;
  }

  int key = (int)*(char *)mgc;

  bpf_printk("key: %d", key);

  // Remove the last character
  ret = bpf_skb_change_tail(skb, skb->len - 1, 0);
  if (ret < 0) {
    bpf_printk("bpf_skb_change_tail failed. return code:%d", ret);
    action = TC_ACT_OK;
    goto out;
  }

  const int *ifindex = bpf_map_lookup_elem(&dummy_redirect_map, &key);
  if (ifindex == NULL) {
    bpf_printk("%d has no corresponding port", key);
    action = TC_ACT_OK;
    goto out;
  }

  // Redirect the packet to the endpoint referenced by map at index key.
  action = bpf_redirect(*ifindex, 0);
  if (action != TC_ACT_REDIRECT) {
    bpf_printk("bpf_redirect_map failed. return code:%d", ret);
  }

out:
  return action;
}

char _license[] SEC("license") = "GPL";