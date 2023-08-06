# The purpose of this file is to plot the traffic at a specific IXP node (or just some "location" in general where
#     ns3 produces pcap output).
# You may provide multiple pcap files that will all be bundled into one traffic stream. This is to account for the fact
#     that in the simulator, one IXP node captures traffic at potentially many different interfaces.
#     To get the complete view of the traffic at that node, the traces from all its interfaces need to be considered.


from scapy.all import *
import matplotlib.pyplot as plt
import numpy as np
import warnings


def process_pcap(pcap, packet_data_rate_global, ip_src_filter):
    for pkt in pcap:
        if 'IP' in pkt:
            ip = pkt['IP']
            # only process packets that are in filter list
            if ip.src not in ip_src_filter:
                continue

            time_bin_index = int(pkt.time)
            packet_data_rate_global[time_bin_index] += len(pkt)  # len(packet) gives packet size in bytes
        else:
            warnings.warn("Found packet without IP layer:", pkt)

def plot_traffic_data_rate(pcap_file_list, end_time, plot_file_name, filter_ip_src_list=None, force_y_lim=[]):
    # mutable default argument should not be a problem here, but PyCharm is not a fan, so here we go :)
    if filter_ip_src_list is None:
        filter_ip_src_list = []

    time_array = np.arange(end_time + 1)  # 1 second time "bins". range is exclusive at the end, thus + 1
    packet_data_rate = np.zeros(len(time_array))  # bytes, stored per seconds, thus effectively yielding data rate

    for file in pcap_file_list:
        print("Processing file:", file)
        pcap = rdpcap(file)
        process_pcap(pcap, packet_data_rate, filter_ip_src_list)

    # convert bytes go Mbit/s -> div by 1e6 then times 8
    packet_data_rate = [dr / 1e6 * 8 for dr in packet_data_rate]

    fig, ax = plt.subplots()
    ax.plot(time_array, packet_data_rate, label="Attack Traffic", linewidth=0.75, alpha=0.9)
    ax.legend(loc='lower right')
    ax.set_xlabel('Time [s]')
    ax.set_ylabel('Data Rate [Mbit/s]')
    if len(force_y_lim) == 2:
        ax.set_ylim(force_y_lim)
    ax.set_title('Attack Traffic Data Rate over Time')
    plt.savefig(plot_file_name, format="pdf", bbox_inches="tight")
    plt.show()

pcap_files = ["filename.pcap"]
filter_src_ip = ["192.168.1.2", "192.168.2.2", "192.168.3.2", "192.168.4.2", "192.168.5.2"]
plot_traffic_data_rate(pcap_files, 600, "output_filename.pdf", filter_ip_src_list=filter_src_ip)
