from scapy.all import *

def arp_test():
    # 构建一个 ARP 请求数据包
    arp_request = Ether(dst="ff:ff:ff:ff:ff:ff") / ARP(pdst="10.10.10.10")

    # 在数据包负载后面追加一个不属于这个数据包的字符
    extra_data = '0'
    arp_request = arp_request / Raw(load=extra_data)

    # 发送 ARP 请求到 dummy0 接口
    sendp(arp_request, iface="dummy0")


def icmp_test():
    icmp_request = Ether(dst="12:34:56:78:9a:bc") / IP(dst="8.8.8.8")/ICMP()

    extra_data = '1'
    icmp_request = icmp_request / Raw(load=extra_data)

    sendp(icmp_request,iface="dummy0")


if __name__ == "__main__":
    arp_test()
    icmp_test()