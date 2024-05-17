from scapy.all import *

def vxlan_test():
    packets = rdpcap('./vxlan.pcapng')

    for packet in packets:
        sendp(packet, iface="vwire_1")
        print(packet.summary()) 
        # print(packet.show()) 

if __name__ == "__main__":
    for i in range(10000):
        vxlan_test()