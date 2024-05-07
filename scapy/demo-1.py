from scapy.all import Ether, IP, TCP, srp, Raw, ICMP, sendp, UDP, Dot1Q, VXLAN

def send_icmp_request():

    inner_tcp = TCP(dport=80, sport=12345)
    inner_ipv4 = IP(dst="192.168.1.2", src="10.0.0.1") / inner_tcp
    inner_eth = Ether(src="00:15:5d:b8:10:a6", dst="de:a2:35:57:4f:65") / inner_ipv4

    vxlan = VXLAN(vni=1001) /inner_eth

    outer_packet = Ether(src="00:15:5d:b8:10:a6", dst="de:a2:35:57:4f:65")/ IP(src="172.19.199.171", dst="220.181.38.149") / UDP(dport=4789)
    final_packet = outer_packet
    sendp(final_packet, iface="vwire_1") 

    # for sent_packet, received_packet in response:
    #     print("Sent Packet:")
    #     print(sent_packet.show())
    #     print("\nReceived Packet:")
    #     print(received_packet.show())

if __name__ == "__main__":
    for i in range(10000):
        send_icmp_request()