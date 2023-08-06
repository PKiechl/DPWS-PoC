from scapy.all import *

def calculate_raw_stats(packets, start, end, target, sources):
    data_per_ip = {}  # data per ip address
    data_sizes = {} # counts per packet-size

    for pkt in packets:
        if "IP" in pkt:
            # filter packets to only specified pulse and duration
            if pkt['IP'].src not in sources:
                continue
            if pkt['IP'].dst != target:
                continue
            if pkt.time > end or pkt.time < start:
                continue

            src_ip = pkt["IP"].src
            size = len(pkt)

            # accumulate per IP
            if src_ip in data_per_ip:
                data_per_ip[src_ip]['total_size'] += size
                data_per_ip[src_ip]['total_packets'] += 1
            else:
                data_per_ip[src_ip] = {'total_size': size, 'total_packets': 1}
            # count per size
            if size in data_sizes:
                data_sizes[size] += 1
            else:
                data_sizes[size] = 1

    return data_per_ip, data_sizes

def calculate_values(filename, start, end, target, sources):
    packets = rdpcap(filename)
    pv_dr_per_ip, pkt_sizes = calculate_raw_stats(packets, start, end, target, sources)

    for ip, data in pv_dr_per_ip.items():
        data_bytes = data['total_size']
        num_pkts = data['total_packets']
        avg_mps = ((data_bytes / (end-start)) / 1000000) * 8  # convert to Mbps
        avg_pps = num_pkts / (end-start)

        print(f"\nIP: {ip}")
        print(f"\t\tAverage DR: {avg_mps:.4f} Mbps")
        print(f"\t\tAverage PV: {avg_pps:.4f} Pps")

    pkt_count = sum(pkt_sizes.values())
    print("\nPacket Size Percentages:")
    for size, data in pkt_sizes.items():
        print(f"{size}: {data/pkt_count*100:.2f} %")

    # sum packet counts and accumulated bytes
    total_pv = 0
    total_bytes = 0
    for ip, data in pv_dr_per_ip.items():
        total_pv += data['total_packets']
        total_bytes += data['total_size']

    print("\nAverages:")
    print(f"\t\tOverall Average DR: {((total_bytes / (end-start)) / 1000000) * 8:.4f} Mbps")
    print(f"\t\tOverall Average PV: {total_pv / (end-start):.4f}")

sources=["192.168.1.2", "192.168.2.2", "192.168.3.2", "192.168.4.2", "192.168.5.2"]
calculate_values("filename.pcap", 180, 210, "192.173.1.2", sources)
