from scapy.all import Ether, IP, ICMP, sendp

def send_icmp_request(target_mac):
    # 创建 Ethernet 数据包，设置目标 MAC 地址
    ethernet_packet = Ether(dst=target_mac)

    # 创建 ICMP Echo 请求数据包
    icmp_packet = IP(dst="192.168.1.1") / ICMP()

    # 合并 Ethernet 数据包和 ICMP 数据包
    final_packet = ethernet_packet / icmp_packet

    # 发送数据包
    sendp(final_packet, iface="eth0")  # 请替换为你的网络接口

# 替换为目标设备的实际 MAC 地址
target_mac_address = "00:11:22:33:44:55"

# 发送 ICMP Echo 请求
send_icmp_request(target_mac_address)
