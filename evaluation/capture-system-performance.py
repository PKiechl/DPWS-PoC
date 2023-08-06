import psutil
import csv
import time
import signal
import sys


def measure_system_resource_use():
    # RAM % and CPU %
    measurements = []

    def signal_handler(sig, frame):
        # https://stackoverflow.com/a/1112350
        # capture program interrupt and write measurements to a CSV file before terminating
        write_to_csv(measurements)
        sys.exit(0)
    # register signal handler
    signal.signal(signal.SIGINT, signal_handler)

    while True:
        # Get current system time to synch up with execution of ns-3 simulation run
        time_stamp = time.strftime("%Y-%m-%d %H:%M:%S", time.localtime())

        # Get CPU usage
        cpu_percent = psutil.cpu_percent(interval=1)

        # Get RAM usage
        ram = psutil.virtual_memory()
        ram_percent = ram.percent

        # Create a dictionary for the measurements including the timestamp
        measurement = {
            "time_stamp": time_stamp,
            "cpu_percent": cpu_percent,
            "ram_percent": ram_percent,
        }

        # Append the measurement to the list
        measurements.append(measurement)

        # Print the measurements
        print(f"Time: ${time_stamp}")
        print(f"CPU Usage: {cpu_percent}%")
        print(f"RAM Usage: {ram_percent}% ")

        # Wait for 1 second before measuring again
        # Note: this is very likely to have some slight timing drift. Additionally, it actually does not measure
        # every second, but rather performs the measurements, then waits 1 second. this seems to result in more
        # of an every 2 seconds measurements. This is perfectly fine, given that the goal is not to have measurements
        # happening in perfect synch, but rather a steady stream of measurements such that trends over time can be
        # observed.
        time.sleep(1)


def write_to_csv(measurements):
    # define fieldnames CSV
    fieldnames = ["time_stamp", "cpu_percent", "ram_percent"]

    # prompt for filename
    filename = input("Enter the output filename (without file extension): ")
    filename = filename+".csv"

    # write to csv
    with open(filename, mode="w", newline="") as file:
        writer = csv.DictWriter(file, fieldnames=fieldnames)
        # header row written separately
        writer.writeheader()
        writer.writerows(measurements)

    print(f"Performance Data written to: '{filename}'")


measure_system_resource_use()
