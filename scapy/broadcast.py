from scapy.all import *

def broadcast_test():
    packets = rdpcap('./broadcast.pcapng')

    for packet in packets:
        sendp(packet, iface="vwire_1")
        print(packet.summary()) 
        # print(packet.show()) 

if __name__ == "__main__":
    for i in range(500):
        broadcast_test()