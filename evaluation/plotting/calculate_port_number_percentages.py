from scapy.all import *
import warnings


def sum_per_port_percentage(packets, target_ip, source_ip_list, end_time, start_time):
    # use target_ip and source_ip_list to narrow down to just attack traffic in one pulse

    source_ports = {}
    dest_ports = {}
    total_packets = 0

    for pkt in packets:
        if 'IP' in pkt:
            # filter packets to only specified pulse and duration
            if pkt['IP'].src not in source_ip_list:
                continue
            if pkt['IP'].dst != target_ip:
                continue
            if pkt.time > end_time or pkt.time < start_time:
                continue

            # grab transport layer packet port counts
            src_port = -1
            dst_port = -1
            if 'TCP' in pkt:
                src_port = pkt['TCP'].sport
                dst_port = pkt['TCP'].dport
            elif 'UDP' in pkt:
                src_port = pkt['UDP'].sport
                dst_port = pkt['UDP'].dport

            if src_port == -1 or dst_port == -1:
                warnings.warn("found packet without port, double check your time stamps")
            else:
                total_packets += 1
                source_ports[src_port] = source_ports.get(src_port, 0) + 1
                dest_ports[dst_port] = dest_ports.get(dst_port, 0) + 1
    return source_ports, dest_ports, total_packets


def calculate_port_percentages(filename, target_ip, source_ip_list, start_time, end_time, random_trim_threshold):
    packets = rdpcap(filename)
    source_ports, dest_ports, total_packets = sum_per_port_percentage(packets, target_ip, source_ip_list, end_time,
                                                                      start_time)

    source_port_percentages = {port: (count / total_packets) * 100 for port, count in source_ports.items()}
    dest_port_percentages = {port: (count / total_packets) * 100 for port, count in dest_ports.items()}

    # sorting
    sorted_src_count = dict(sorted(source_ports.items(), key=lambda item: item[1], reverse=True))
    sorted_src_percent = dict(sorted(source_port_percentages.items(), key=lambda item: item[1], reverse=True))
    sorted_dest_count = dict(sorted(dest_ports.items(), key=lambda item: item[1], reverse=True))
    sorted_dest_percent = dict(sorted(dest_port_percentages.items(), key=lambda item: item[1], reverse=True))

    # remove those with percentages below threshold
    filtered_sorted_src_count = {key: value for key, value in sorted_src_count.items() if value >= random_trim_threshold*total_packets}
    filtered_sorted_dest_count = {key: value for key, value in sorted_dest_count.items() if value >= random_trim_threshold*total_packets}
    filtered_sorted_src_percent = {key: value for key, value in sorted_src_percent.items() if value >= random_trim_threshold}
    filtered_sorted_dest_percent = {key: value for key, value in sorted_dest_percent.items() if value >= random_trim_threshold}

    # count ports with percentages below threshold, use as indication for randomization
    src_ports_below_threshold = len(sorted_src_percent) - len(filtered_sorted_src_percent)
    dest_ports_below_threshold = len(sorted_dest_percent) - len(filtered_sorted_dest_percent)

    # treat low-count/low-percent as randomized -> set trim threshold to change the cut off
    # to determine randomiziation percent, the percentages ABOVE the threshold are subtracted from 1, yielding the remaining
    src_port_random_percent = round(100 - sum(filtered_sorted_src_percent.values()), 2)
    dest_port_random_percent = round(100 - sum(filtered_sorted_dest_percent.values()), 2)

    print(f"total packets considered: {total_packets}\n")

    print(f"source ports counts: {filtered_sorted_src_count}")
    print(f"source ports percentage breakdown: {filtered_sorted_src_percent}")
    print(f"number of individual source port numbers below occurrence threshold: {src_ports_below_threshold}")
    print(f"percent of source ports that are randomized: {src_port_random_percent}\n")

    print(f"destination ports counts: {filtered_sorted_dest_count}")
    print(f"destination ports percentage breakdown: {filtered_sorted_dest_percent}")
    print(f"number of individual destination port numbers below occurrence threshold: {dest_ports_below_threshold}")
    print(f"percent of destination ports that are randomized: {dest_port_random_percent}\n")



src_list = ["192.168.1.2", "192.168.2.2", "192.168.3.2", "192.168.4.2", "192.168.5.2"]
# use the start_time and end_time to restrict the analysis to a sepcific part of the packet. In combination with the
# ip address arguments you can narrow down the analysis to a specific pulse
# use trim threshold to determine which percentages are considered low enough to be random
calculate_port_percentages("filename.pcap", "192.173.1.2", src_list, 60, 90, 0.01)
