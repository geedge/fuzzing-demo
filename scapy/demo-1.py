from scapy.all import Ether, IP, TCP, srp, Raw, ICMP

def send_icmp_request():
    ethernet_packet = Ether(src="00:15:5d:b8:10:a6", dst="de:a2:35:57:4f:65")

    http_get_request = b"GET / HTTP/1.1\r\nHost: www.baidu.com\r\n\r\n"

    # final_packet = ethernet_packet / IP(src="172.19.199.171", dst="220.181.38.149") / TCP(dport=80) / Raw(load=http_get_request)

    final_packet = ethernet_packet / IP(src="172.19.199.171", dst="220.181.38.149") / ICMP()

    response, _ = srp(final_packet, iface="vwire_1") 

    for sent_packet, received_packet in response:
        print("Sent Packet:")
        print(sent_packet.show())
        print("\nReceived Packet:")
        print(received_packet.show())

send_icmp_request()