# DPWS - A Distributed Pulse-Wave Simulator
This simulator, based on [NS-3](https://www.nsnam.org/) is designed for the generation of distributed pulse-wave DDoS datasets. It uses this [HTTP-traffic model](https://github.com/saulodamata/ns-3-http-traffic-generator)
to implement benign node traffic and offers thoroughly parametrized implementations of TCP SYN flooding, UPD flooding and ICMP flooding as possible attack vectors. It implements a distributed perspective, i.e., 
traffic traces are captured at different points in the topology.

## Installation Guidelines
The simulator requires that you have NS-3 installed. You may do so in the way that best suits you if you prefer not to install via git,
refer to the [NS-3 documentation](https://www.nsnam.org/wiki/Installation) for possible options.
This project was developed with version 3.38 of NS-3, your mileage may very when using other versions.


> To install with git:
> 
> 1. `git clone https://gitlab.com/nsnam/ns-3-dev.git`
> 2. `cd ns-3-dev`
> 3. `git checkout -b ns-3.38-branch ns-3.38`
> 
> Ensure that you have MPI and CMake installed. If you are on Mac OS, the following commands will suffice:
> 
> - `brew install cmake`
> - `brew install open-mpi`
> 
> Configure NS-3:
> 1. `./ns3 configure --enable-mpi --build-profile=optimized`
>    2. you may use a different build profile, such as `debug` if you wish for additional logging output
> 2. `./ns3 build`
> 3. (Optional) `./test.py`
> 

At this point you have a functioning NS-3 installation and it is time to install the project files.
> - `git clone https://github.com/PKiechl/DPWS-PoC.git`
> - `cd simulator`
> 
> The following files have to be transferred to your NS-3 installation:
> - Transfer the contents of `contrib` into the `contrib` folder of your NS-3 installation.
> - Copy the contents of the `scratch` directory to the `scratch` directory of your NS-3 installation
> - Copy the contents of the `socket memory patch/src/internet/model` directory to the `src/internet/model` directory of your NS-3 installation and overwrite the files present there. Refer to [Patches.md](simulator/PATCHES.md) for more information
> - Copy the contents of the `overwrites/src/internet/model` directory to the `src/internet/model` directory of your NS-3 installation and overwrite the file present there. Refer to [Patches.md](simulator/PATCHES.md) for more information
> 
> As a last step, the build step must be executed again such that the newly transferred
    files take effect: `./ns3 build`
> 

## Running the Simulator

> If you are on Mac OS, consider running `export TMPDIR=/tmp` to avoid [truncation](https://github.com/open-mpi/ompi/issues/7393#issuecomment-882018321) issues that can lead to MPI errors.
> 
> Use the following command to run the simulator with all options:
> 
> `./ns3 run dpws --command-template="mpiexec -np 4 %s"
-- --configFile="sample_config.yaml"
--printConfiguration=true
--progressLogInterval=5
--printTopology=true`
> 
> - the `--command-template` allows for the use of MPI parallelization and specify the number of cores the simulator is allowed to use. In the example above, 4 cores are made available
> - `--configFile` is used to specify the configuration file and is required. Use the included `sample_config.yaml` or [create your own](simulator/CONFIGURATION.md)
> - `--printConfiguration` prints the parsed configuration to the console, including optional parameters you may have not explicitly specified
> - `--progressLogInterval` is used to control in which interval (in simulation time) progress logs are written to console. By default that happens every 15 seconds within the simulation timeline.
> - `--printTopology` is used enabled by default. It prints the generated topology as a list of node-pairs that share an edge.
