from scapy.all import *

def arp_test():
    packets = rdpcap('./arp.pcapng')

    for packet in packets:
        sendp(packet, iface="vwire_1")
        print(packet.summary()) 
        # print(packet.show()) 

if __name__ == "__main__":
    arp_test()