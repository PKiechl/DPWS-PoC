import matplotlib.pyplot as plt
from matplotlib.patches import Patch
import numpy as np
from scapy.all import rdpcap
from collections import defaultdict
import warnings

def process_pcap(pcap, dr_tcp_global, pv_tcp_global, dr_udp_global, pv_udp_global, dr_icmp_global, pv_icmp_global,
                 attackers, max_time):
    for pkt in pcap:
        if 'IP' in pkt:
            ip = pkt['IP']

            # don't process packets whose ip.src does not match any of the attackers
            if ip.src not in attackers:
                continue

            time_bin_index = int(pkt.time)
            # only store information for range covered by provided max_time (end_time)
            if time_bin_index <= max_time:
                # update data rate and packet count for src--time_bin pair for given protocol
                if ip.proto == 1:
                    # ICMP
                    dr_icmp_global[ip.src][time_bin_index] += len(pkt)
                    pv_icmp_global[ip.src][time_bin_index] += 1
                elif ip.proto == 6:
                    # TCP
                    dr_tcp_global[ip.src][time_bin_index] += len(pkt)
                    pv_tcp_global[ip.src][time_bin_index] += 1
                elif ip.proto == 17:
                    # UDP
                    dr_udp_global[ip.src][time_bin_index] += len(pkt)
                    pv_udp_global[ip.src][time_bin_index] += 1
                else:
                    warnings.warn("Found packet that does not fit protocols (UDP, TCP, ICMP)", pkt)
        else:
            warnings.warn("Found packet without IP layer:", pkt)

def rearrange_into_array(attackers, dr_dict, dr_arr, pv_dict, pv_arr):
    for atk in attackers:
        # check if atk actually was present in any packet (both dicts share same keys)
        if atk in dr_dict.keys():
            dr_arr.append([dr / 1e6 * 8 for dr in dr_dict[atk]])
            pv_arr.append(pv_dict[atk])

def plot_traffic(pcap_file_list, attacker_ip_list, end_time, plot_file_name):
    # establish x-axis dimension (time elapsed)
    time_array = np.arange(end_time + 1)  # 1 second time "bins". range is exclusive at the end, thus + 1

    # collections for data rates and packet volume per protocol
    dr_tcp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))
    pv_tcp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))
    dr_udp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))
    pv_udp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))
    dr_icmp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))
    pv_icmp_per_attacker_per_time_bin = defaultdict(lambda: [0] * len(time_array))

    # perform accumulation across all files
    for file in pcap_file_list:
        print("Processing file:", file)
        pcap = rdpcap(file)
        process_pcap(pcap,
                     dr_tcp_per_attacker_per_time_bin,
                     pv_tcp_per_attacker_per_time_bin,
                     dr_udp_per_attacker_per_time_bin,
                     pv_udp_per_attacker_per_time_bin,
                     dr_icmp_per_attacker_per_time_bin,
                     pv_icmp_per_attacker_per_time_bin,
                     attacker_ip_list,
                     end_time)

    # convert dictionaries to nested arrays for plotting, convert data rate form Bytes/s to MBit/s -> div by 1e6 * 8
    # also perform reordering such that order in nested array follows order of attackers in initial attacker_ip_list
    dr_tcp_nested_ordered = []
    pv_tcp_nested_ordered = []
    dr_udp_nested_ordered = []
    pv_udp_nested_ordered = []
    dr_icmp_nested_ordered = []
    pv_icmp_nested_ordered = []

    rearrange_into_array(attacker_ip_list, dr_tcp_per_attacker_per_time_bin, dr_tcp_nested_ordered,
                         pv_tcp_per_attacker_per_time_bin, pv_tcp_nested_ordered)
    rearrange_into_array(attacker_ip_list, dr_udp_per_attacker_per_time_bin, dr_udp_nested_ordered,
                         pv_udp_per_attacker_per_time_bin, pv_udp_nested_ordered)
    rearrange_into_array(attacker_ip_list, dr_icmp_per_attacker_per_time_bin, dr_icmp_nested_ordered,
                         pv_icmp_per_attacker_per_time_bin, pv_icmp_nested_ordered)

    # perform per-time-bin sum across all attacker nodes for the supplementary line-plots
    dr_summed = np.zeros(len(time_array))
    pv_summed = np.zeros(len(time_array))
    for time in time_array:
        for lst in dr_tcp_nested_ordered:
            dr_summed[time] += lst[time]
        for lst in dr_udp_nested_ordered:
            dr_summed[time] += lst[time]
        for lst in dr_icmp_nested_ordered:
            dr_summed[time] += lst[time]

        for lst in pv_tcp_nested_ordered:
            pv_summed[time] += lst[time]
        for lst in pv_udp_nested_ordered:
            pv_summed[time] += lst[time]
        for lst in pv_icmp_nested_ordered:
            pv_summed[time] += lst[time]

    colors_stack_TCP = ['#162258', '#40437e', '#6768a7', '#908fd2', '#d7d4ff']
    colors_stack_UDP = ['#003804', '#21652f', '#53955b', '#84c88b', '#b8febd']
    colors_stack_ICMP = ['#3e2707', '#6b4f2f', '#9a7a59', '#cba986', '#ffdab5']

    # https://stackoverflow.com/a/63741687
    p_TCP_1 = Patch(facecolor=colors_stack_TCP[0], edgecolor='black')
    p_TCP_2 = Patch(facecolor=colors_stack_TCP[1], edgecolor='black')
    p_TCP_3 = Patch(facecolor=colors_stack_TCP[2], edgecolor='black')
    p_TCP_4 = Patch(facecolor=colors_stack_TCP[3], edgecolor='black')
    p_TCP_5 = Patch(facecolor=colors_stack_TCP[4], edgecolor='black')

    p_UDP_1 = Patch(facecolor=colors_stack_UDP[0], edgecolor='black')
    p_UDP_2 = Patch(facecolor=colors_stack_UDP[1], edgecolor='black')
    p_UDP_3 = Patch(facecolor=colors_stack_UDP[2], edgecolor='black')
    p_UDP_4 = Patch(facecolor=colors_stack_UDP[3], edgecolor='black')
    p_UDP_5 = Patch(facecolor=colors_stack_UDP[4], edgecolor='black')

    p_ICMP_1 = Patch(facecolor=colors_stack_ICMP[0], edgecolor='black')
    p_ICMP_2 = Patch(facecolor=colors_stack_ICMP[1], edgecolor='black')
    p_ICMP_3 = Patch(facecolor=colors_stack_ICMP[2], edgecolor='black')
    p_ICMP_4 = Patch(facecolor=colors_stack_ICMP[3], edgecolor='black')
    p_ICMP_5 = Patch(facecolor=colors_stack_ICMP[4], edgecolor='black')


    # plot 1: stackplot of dr per attacker node with pv_summed line plot
    fig, ax = plt.subplots()
    # stackplots for data rate
    ax.stackplot(time_array, dr_tcp_nested_ordered, colors=colors_stack_TCP)
    ax.stackplot(time_array, dr_udp_nested_ordered, colors=colors_stack_UDP)
    ax.stackplot(time_array, dr_icmp_nested_ordered, colors=colors_stack_ICMP)
    ax.legend(bbox_to_anchor=(1.15, 0.85), loc="upper left",
              handles=[p_TCP_5, p_TCP_4, p_TCP_3, p_TCP_2, p_TCP_1,
                       p_UDP_5, p_UDP_4, p_UDP_3, p_UDP_2, p_UDP_1,
                       p_ICMP_5, p_ICMP_4, p_ICMP_3, p_ICMP_2, p_ICMP_1],
              labels=["", "", "", "", "",
                      "", "", "", "", "",
                      "Attacker 5 (TCP / UDP / ICMP)",
                      "Attacker 4 (TCP / UDP / ICMP)",
                      "Attacker 3 (TCP / UDP / ICMP)",
                      "Attacker 2 (TCP / UDP / ICMP)",
                      "Attacker 1 (TCP / UDP / ICMP)"],
              ncol=3, handletextpad=0.5, handlelength=2.5, columnspacing=-0.5)

    ax.set_xlabel('Time [s]')
    ax.set_ylabel('Data Rate [MBit/s]')
    ax.set_title('Data Rate per Attacker Node over Time')
    # line plot for packet volume
    ax_alt = ax.twinx()
    ax_alt.plot(time_array, pv_summed, label="Combined Packet Volume", linewidth=1.25, color="red")
    ax_alt.legend(bbox_to_anchor=(1.15, 1), loc="upper left")
    ax_alt.set_ylim(ymin=0)  # prevent entire subplot from "floating" slightly above the bottom of the graph
    ax_alt.set_ylabel("Packet Volume [Pkt/s]")
    # save and show
    plt.savefig(f"{plot_file_name}__data_rate.pdf", format="pdf", bbox_inches="tight")
    plt.show()


    # # plot 2: stackplot of pv per attacker node with dr_summed line plot
    fig2, ax2 = plt.subplots()
    # stackplots for packet volume
    ax2.stackplot(time_array, pv_tcp_nested_ordered, colors=colors_stack_TCP)
    ax2.stackplot(time_array, pv_udp_nested_ordered, colors=colors_stack_UDP)
    ax2.stackplot(time_array, pv_icmp_nested_ordered, colors=colors_stack_ICMP)
    ax2.legend(bbox_to_anchor=(1.15, 0.85), loc="upper left",
               handles=[p_TCP_5, p_TCP_4, p_TCP_3, p_TCP_2, p_TCP_1,
                        p_UDP_5, p_UDP_4, p_UDP_3, p_UDP_2, p_UDP_1,
                        p_ICMP_5, p_ICMP_4, p_ICMP_3, p_ICMP_2, p_ICMP_1],
               labels=["", "", "", "", "",
                       "", "", "", "", "",
                       "Attacker 5 (TCP / UDP / ICMP)",
                       "Attacker 4 (TCP / UDP / ICMP)",
                       "Attacker 3 (TCP / UDP / ICMP)",
                       "Attacker 2 (TCP / UDP / ICMP)",
                       "Attacker 1 (TCP / UDP / ICMP)"],
               ncol=3, handletextpad=0.5, handlelength=2.5, columnspacing=-0.5)
    ax2.set_xlabel('Time [s]')
    ax2.set_ylabel('Packet Volume [Pkt/s]')
    ax2.set_title('Packet Volume per Attacker Node over Time')
    # line plot for data rate
    ax2_alt = ax2.twinx()
    ax2_alt.plot(time_array, dr_summed, label="Combined Data Rate", linewidth=1.25, color="red")
    ax2_alt.legend(bbox_to_anchor=(1.15, 1), loc="upper left")
    ax2_alt.set_ylim(ymin=0)  # prevent entire subplot from "floating" slightly above the bottom of the graph
    ax2_alt.set_ylabel("Data Rate [MBit/s]")
    # save and show
    plt.savefig(f"{plot_file_name}__packet_volume.pdf", format="pdf", bbox_inches="tight")
    plt.show()


# List pcap file paths
pcap_files = ["filename.pcap"]
# List ip addresses of the attacker nodes
attacker_node_ips = ["192.168.1.2", "192.168.2.2", "192.168.3.2", "192.168.4.2", "192.168.5.2"]
plot_traffic(pcap_files, attacker_node_ips, 220, "output_filename")
