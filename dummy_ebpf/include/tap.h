#pragma once

#include <linux/if_tun.h>

int tap_alloc(const char *ifname, int flags);
int tap_set_mac(const unsigned char *interface_name, const unsigned char *str_macaddr);
int tap_set_ip(const unsigned char *interface_name, const unsigned char *ipaddr);