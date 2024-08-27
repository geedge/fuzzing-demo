// clang-format off
#include <linux/types.h>
// clang-format on
#include <bpf/bpf_endian.h>
#include <bpf/bpf_helpers.h>
#include <linux/bpf.h>
#include <linux/in.h>

// modify from:
// https://github.com/xdp-project/xdp-tutorial/blob/master/packet03-redirecting/xdp_prog_kern.c

struct {
  __uint(type, BPF_MAP_TYPE_DEVMAP);
  __type(key, __u8);
  __type(value, int);
  __uint(max_entries, 256);
} tx_port SEC(".maps");

SEC("xdp_redirect_map")
int xdp_redirect_map_func(struct xdp_md *ctx) {
  int ret = 0;
  int action = XDP_PASS;
  void *data = (void *)(long)ctx->data;
  void *data_end = (void *)(long)ctx->data_end;

  // According to the last character, determine which interface to forward the data packet
  unsigned char *last_byte = (unsigned char *)(data_end - 1);
  __u8 key = *last_byte;
  bpf_printk("key: %d", key);

  // Remove the last character
  ret = bpf_xdp_adjust_tail(ctx, -1);
  if (ret < 0) {
    bpf_printk("bpf_xdp_adjust_tail failed. return code:%d", ret);
    action = XDP_DROP;
  }

  // Redirect the packet to the endpoint referenced by map at index key.
  ret = bpf_redirect_map(&tx_port, key, 0);
  if (ret != XDP_REDIRECT) {
    bpf_printk("bpf_redirect_map failed. return code:%d", ret);
    action = XDP_DROP;
  }

out:
  return action;
}

char _license[] SEC("license") = "GPL";