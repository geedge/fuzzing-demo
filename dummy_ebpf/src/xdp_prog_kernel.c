#include <linux/types.h>

#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>
#include <linux/in.h>
#include <xdp/parsing_helpers.h>

// modify from:
// https://github.com/xdp-project/xdp-tutorial/blob/master/packet03-redirecting/xdp_prog_kern.c

struct {
  __uint(type, BPF_MAP_TYPE_DEVMAP);
  __type(key, int);
  __type(value, int);
  __uint(max_entries, 256);
} dummy_redirect_map SEC(".maps");

#if 0
static __always_inline char packet_last_byte(struct hdr_cursor *nh, void *data_end) {
  long size = data_end - nh->pos;
  if (nh->pos + size - 1 > data_end) {
    return 0;
  }

  char *last_byte = (char *)(nh->pos + size - 1);
  return *last_byte;
}
#endif

SEC("xdp")
int xdp_redirect_map_func(struct xdp_md *ctx) {
  int ret = 0;
  int action = XDP_PASS;
  void *data = (void *)(long)ctx->data;
  void *data_end = (void *)(long)ctx->data_end;

  bpf_printk("Receive a packet");

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

  char key = *(char *)mgc;

  bpf_printk("key: %d", key);

  // Remove the last character
  ret = bpf_xdp_adjust_tail(ctx, -1);
  if (ret < 0) {
    bpf_printk("bpf_xdp_adjust_tail failed. return code:%d", ret);
    action = XDP_DROP;
  }
  struct sk_buff *skb;
  // Redirect the packet to the endpoint referenced by map at index key.
  ret = bpf_redirect_map(&dummy_redirect_map, key, 0);
  if (ret != XDP_REDIRECT) {
    bpf_printk("bpf_redirect_map failed. return code:%d", ret);
    action = XDP_DROP;
  }

out:
  return action;
}

char _license[] SEC("license") = "GPL";