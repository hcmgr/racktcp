from scapy.all import *

def packet_callback(packet):
    if IP in packet:
        ip_src = packet[IP].src
        ip_dst = packet[IP].dst
        if TCP in packet:
            tcp_segment = packet[TCP]
            print(f"IP {ip_src} -> {ip_dst}")
            print(f"TCP Src Port: {tcp_segment.sport}, Dst Port: {tcp_segment.dport}")
            print(f"Flags: {tcp_segment.flags}")
            print(f"Seq: {tcp_segment.seq}, Ack: {tcp_segment.ack}")
            print("-" * 50)

PORT = 8101

# Capture packets on the desired interface
print(f"Listening for TCP traffic on port {PORT}...")
sniff(filter=f"tcp port 8101 and host 10.49.0.5", iface="eth0", prn=packet_callback)

