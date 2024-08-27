// from: https://blog.csdn.net/xxb249/article/details/86690067

#include "tap.h"

#include <arpa/inet.h>
#include <fcntl.h>
#include <linux/if.h>
#include <linux/if_tun.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

int tap_alloc(const char *ifname, int flags) {
  struct ifreq ifr;
  int fd, err;
  char *clonedev = "/dev/net/tun";

  if ((fd = open(clonedev, O_RDWR)) < 0) {
    return fd;
  }

  memset(&ifr, 0, sizeof(ifr));
  ifr.ifr_flags = flags;
  strcpy(ifr.ifr_name, ifname);
  if ((err = ioctl(fd, TUNSETIFF, (void *)&ifr)) < 0) {
    close(fd);
    return err;
  }
  /* 进程退出 tap0不消失 如果想删除则设置为0 */
  if (ioctl(fd, TUNSETPERSIST, 1) < 0) {
    perror("enabling TUNSETPERSIST");
    exit(1);
  }
  return fd;
}

int tap_set_mac(const unsigned char *interface_name, const unsigned char *str_macaddr) {
  int ret;
  int sock_fd;
  struct ifreq ifr;
  unsigned int mac2bit[6];

  if (interface_name == NULL || str_macaddr == NULL) {
    return -1;
  }

  // 提取mac格式
  sscanf((char *)str_macaddr, "%02X:%02X:%02X:%02X:%02X:%02X", (unsigned int *)&mac2bit[0],
         (unsigned int *)&mac2bit[1], (unsigned int *)&mac2bit[2], (unsigned int *)&mac2bit[3],
         (unsigned int *)&mac2bit[4], (unsigned int *)&mac2bit[5]);

  sock_fd = socket(PF_INET, SOCK_DGRAM, 0);
  if (sock_fd < 0) {
    return -2;
  }

  sprintf(ifr.ifr_ifrn.ifrn_name, "%s", interface_name);
  ifr.ifr_ifru.ifru_hwaddr.sa_family = 1;
  ifr.ifr_ifru.ifru_hwaddr.sa_data[0] = mac2bit[0];
  ifr.ifr_ifru.ifru_hwaddr.sa_data[1] = mac2bit[1];
  ifr.ifr_ifru.ifru_hwaddr.sa_data[2] = mac2bit[2];
  ifr.ifr_ifru.ifru_hwaddr.sa_data[3] = mac2bit[3];
  ifr.ifr_ifru.ifru_hwaddr.sa_data[4] = mac2bit[4];
  ifr.ifr_ifru.ifru_hwaddr.sa_data[5] = mac2bit[5];

  ret = ioctl(sock_fd, SIOCSIFHWADDR, &ifr);
  if (ret != 0) {
    return -4;
  }
  close(sock_fd);
  return 0;
}

int tap_set_ip(const unsigned char *interface_name, const unsigned char *ipaddr) {
  int err;
  int ret;
  int socket_fd;
  struct ifreq ifr;
  struct sockaddr_in sin;

  if (interface_name == NULL || ipaddr == NULL) {
    return -1;
  }

  socket_fd = socket(AF_INET, SOCK_DGRAM, 0);
  if (socket_fd < 0) {
    printf("Create Socket Failed.\n");
    return -2;
  }
  // 指定网卡名称且up
  sprintf(ifr.ifr_name, "%s", interface_name);
  /* 获得接口的标志 */
  if ((err = ioctl(socket_fd, SIOCGIFFLAGS, (void *)&ifr)) < 0) {
    perror("ioctl SIOCGIFADDR");
    close(socket_fd);
    return -3;
  }
  ifr.ifr_flags |= IFF_UP;
  ret = ioctl(socket_fd, SIOCSIFFLAGS, &ifr);
  if (ret != 0) {
    printf("Up Device %s Failed.\n", interface_name);
    close(socket_fd);
    return -3;
  }
  // 设置ip
  memset(&sin, 0, sizeof(struct sockaddr_in));
  sin.sin_family = AF_INET;
  inet_pton(AF_INET, ipaddr, &sin.sin_addr.s_addr);
  memcpy(&ifr.ifr_addr, &sin, sizeof(struct sockaddr));
  ret = ioctl(socket_fd, SIOCSIFADDR, &ifr);
  if (ret != 0) {
    printf("Set Ipaddr For Device %s Failed.\n", interface_name);
    close(socket_fd);
    return -4;
  }

  // 设置mask
  sin.sin_family = AF_INET;
  inet_pton(AF_INET, "255.255.255.0", &sin.sin_addr.s_addr);
  memcpy(&ifr.ifr_netmask, &sin, sizeof(struct sockaddr));
  ret = ioctl(socket_fd, SIOCSIFNETMASK, &ifr);

  if (ret != 0) {
    printf("Set NetMask For Device %s Failed.\n", interface_name);
    close(socket_fd);
    return -5;
  }
  close(socket_fd);
  return 0;
}

#if 0 
int main() {
  int tun_fd, nread;
  char buffer[1500];

  /* Flags: IFF_TUN   - TUN device (no Ethernet headers)
   *        IFF_TAP   - TAP device
   *        IFF_NO_PI - Do not provide packet information
   */
  tun_fd = tun_alloc("tap0", IFF_TAP | IFF_NO_PI);

  if (tun_fd < 0) {
    perror("Allocating interface");
    exit(1);
  }
  tap_set_mac("tap0", "08:00:11:22:33:44");
  tap_set_ip("tap0", "192.168.62.10");
  while (1) {
    nread = read(tun_fd, buffer, sizeof(buffer));
    if (nread < 0) {
      perror("Reading from interface");
      close(tun_fd);
      exit(1);
    }
    printf("Read %d bytes [%s] from tun/tap device\n", nread, buffer);
  }
  return 0;
}
#endif