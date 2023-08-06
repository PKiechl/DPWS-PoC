# neglecting CPU in this plot because it is basically always maxed anyway

import csv
import matplotlib.pyplot as plt
from datetime import datetime

def create_plot_data_for_file(filename, idle_ram_avg, time_frame_start, time_frame_stop):
    # perform full file read
    timestamps = []
    ram_percentages = []

    with open(filename, mode="r") as file:
        reader = csv.DictReader(file)
        for row in reader:
            timestamps.append(datetime.strptime(row["time_stamp"], "%Y-%m-%d %H:%M:%S"))
            ram_percentages.append(float(row["ram_percent"]))

    # narrow down to specified time-frame & express timestamps as time elapsed in seconds
    start_time = datetime.strptime(time_frame_start, "%Y-%m-%d %H:%M:%S")
    end_time = datetime.strptime(time_frame_stop, "%Y-%m-%d %H:%M:%S")

    to_plot__time_elapsed = []
    start_index = -1
    end_index = -1

    for i, stamp in enumerate(timestamps):
        # just to be safe we operate with the day as well. I sincerely hope none of the tests will for that long
        # though...
        if start_time <= stamp <= end_time:
            diff = int((stamp - start_time).total_seconds())
            # int() to trim the decimal positions. datetime as used in this project does not consider sub-second
            # time, so the diff is always essentially just an integer with a .0
            to_plot__time_elapsed.append(diff)

            if start_index == -1:
                # take note of the first index to be considered in the cpu/ram data lists
                start_index = i
            end_index = i

    # extract relevant sub-lists of the cpu/ram data, based on the relevant time-frame (upper slice index is exclusive,
    # thus +1)
    to_plot__ram_percentages = ram_percentages[start_index:end_index + 1]

    # subtract the idle system averages to get isolated costs of just the simulation
    to_plot__ram_percentages = [el - idle_ram_avg for el in to_plot__ram_percentages]

    print(f"max ram for file {filename}: {max(to_plot__ram_percentages)}")

    return to_plot__time_elapsed, to_plot__ram_percentages

def plot_system_load(filename_list, timestamp_list, idle_ram_avg_list, plot_labels, plot_name, plot_title, force_full_y_axis=False, overwrite_legend_position=""):

    plt.title(plot_title)
    plt.xlabel("Time Elapsed [s]")
    plt.ylabel("RAM Usage [%]")

    for index, file in enumerate(filename_list):
        print(f"processing file: {file}")
        x, y = create_plot_data_for_file(file, idle_ram_avg_list[index], timestamp_list[index][0], timestamp_list[index][1])
        plt.plot(x, y, label=plot_labels[index])

    if overwrite_legend_position != "":
        plt.legend(loc=overwrite_legend_position)
    else:
        plt.legend()
    plt.locator_params(axis='x', nbins=15)  # Setting the number of ticks
    plt.xticks(rotation=45)
    if force_full_y_axis:
        plt.yticks([0, 10, 20, 30, 40, 50, 60, 70, 80, 90, 100])
    plt.tight_layout()

    # Save and show
    plt.savefig(f"{plot_name}.pdf", format="pdf", bbox_inches="tight")
    plt.show()

files = [
    "../../filename.csv",
    "../../filename2.csv",
]

labels = [
    "Label 1",
    "Label 2",
]

timestamps = [
    ("2023-07-23 14:25:30", "2023-08-01 14:48:01"),
    ("2023-07-23 14:50:58", "2023-08-01 15:07:21"),
]

ram_idle_avgs = [
    13.7,
    14.2,
]

plot_system_load(files, timestamps, ram_idle_avgs,  labels, "output_filename", "Impact of UDP Packet Sink on Memory Consumption")
