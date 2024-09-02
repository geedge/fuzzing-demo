from scapy.all import *

def arp_test():
    # 构建一个 ARP 请求数据包
    arp_request = Ether(src="76:56:e5:3f:ac:8e", dst="ff:ff:ff:ff:ff:ff") / ARP(pdst="192.168.41.71")

    # 在数据包负载后面追加一个不属于这个数据包的字符
    extra_data = '0'
    arp_request = arp_request / Raw(load=extra_data)

    # 发送 ARP 请求到 dummy0 接口
    sendp(arp_request, iface="dummy0")


def icmp_test():
    icmp_request = Ether(src="76:56:e5:3f:ac:8e", dst="76:56:e5:3f:ac:8e") / IP(dst="192.168.41.71")/ICMP()

    extra_data = '1'
    icmp_request = icmp_request / Raw(load=extra_data)

    sendp(icmp_request,iface="dummy0")


def udp_test():
    ip = IP(dst="192.168.1.1")  
    udp = UDP(sport=12345, dport=80) 
    data = Raw(load="Hello, UDP!")

    packet = ip / udp / data

    extra_data = '0'
    packet = packet / Raw(load=extra_data)

    sendp(packet, iface="dummy0")

if __name__ == "__main__":
    arp_test()
    icmp_test()
    udp_test()