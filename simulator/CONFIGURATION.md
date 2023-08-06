The Simulator is highly configurable in terms of the use cases it can model. It requires that a configuration YAML
file be provided. The file expects the following structure to be present.

## Global Settings
> 
> Contains setting that have global impact.
> ```
> global_settings:
>   capture:
>       pcap_prefix: EXAMPLE
>   attack:
>       burst_duration_s: 60.0
>       target_switch_duration_s: 0.0
>       attack_vectors:
>           - type: tcp_syn_flooding
>   scheduling:
>       simulation_duration_s: 600.0
>   autonomous_systems_connections:
>       network_address: 33.33.1.0
>       network_mask: 255.255.255.0
> ```
> - `capture`
>   - `pcap_prefix`: the filename prefix of the output.
> - `attack`
>   - `burst_duration_s`: duration of each pulse.
>   - `target_switch_duration_s`: time it takes for the attack to (1) be redirected to the next target and (2) switch attack vector
>   - `attack_vectors`: the list of attack vectors, discussed in more detail in the [Attack Settings](#attack-settings) chapter
> - `scheduling`
>   - `simulation_duration_s`: duration of the simulation run
> - `autonomous_systems_connections`
>   - `network_address`: subnet addresses used to connect IXP nodes with AS Gateways. More on those in the [Topology Settings](#topology-settings) chapter. These connections are "transparent" and do not show up in your PCAP output.
>   - `network_mask`: corresponding network mask
> 
> Of these settings, the `autonomous_systems_connections` key is optional and you may omit the `target_switch_duration_s`.


## Topology Settings
> The topology conceptually consists of a central network (CN) of IXP nodes to which you can attach autonomous systems (AS) which represent their
> own subnet. As such you control much of how the topology ultimately looks like.
> 
> ### Central Network
> ```
> central_network:
>   network_address: 10.1.1.0
>   network_mask: 255.255.255.0
>   bandwidth: 500Gbps
>   delay: 1ms
>   degree_of_redundancy: 0.75
>   topology_seed: 35
>   nodes:
>       - id: IXP1
>       - id: IXP2
> ```
> - `network_address` and `network_mask` control the corresponding properties. The central network address space is shared also "transparent", i.e., does not show up in the PCAP output. As such the properties can both be omitted.
> - `bandwidth` and `delay` define the corresponding properties of each central network connection. Default values are present, thus they can be omitted.
> - `degree_of_redundancy` controls how many additional connections between IXP nodes are randomly constructed beyond those strictly necessary for a minimal topology. Can be omitted
> - `topology_seed` seeds the topology randomization. The property is optional.
> - `nodes` contains a list of IXP nodes. This allows you to control the number of IXP nodes. Their IDs are used to define to which IXP an AS connects
>
> ### Autonomous Systems
> ```
> autonomous_systems:
>   - id: AS1
>       network_address: 192.168.1.0
>       network_mask: 255.255.255.0
>       bandwidth: 100Gbps
>       delay: 10ms
>       attachment:
>           central_network_attachment_node: IXP1
>           bandwidth: 100Gbps
>           delay: 3ms
>   - id: AS2
>       network_address: 192.173.1.0
>       attachment:
>           central_network_attachment_node: IXP2
> ```
> The `autonomous_systems` key allows you to list any number of ASs. For each of them a suite of settings is available. In the example above, `AS2`
> represents a minimal configuration with only the required properties, whilst `AS2` contains all optional properties as well.
> - `network_address` and `network_mask` control the corresponding settings for the AS
> - `bandwidth` and `delay` define the values of those properties within the AS
> - `attachment`
>   - `central_network_attachment_node` must be a valid IXP node as defined in the CN settings. The AS will then connect to that IXP.
>   - `bandwidth` and `delay` define the values of those properties for the connection from the AS gateway to the IXP node


## Node Settings
> ### Benign Nodes
> ```
> benign_client_nodes:
>   - id: BN1
>     peer: TN1
>     owner_as: AS1
>     max_reading_time: 100
> ```
> Define the list of benign nodes and attribute them an AS.
> - `peer` defines the communication partner of the node. Can be a target or non-target server node
> - `owner_as` defines into which AS the node is installed
> - `max_reading_time` defines the maximal reading time, i.e., the maximal interval between HTTP requests made by the node. The property is optional.
> 
> ### Target Nodes and Other Server Nodes
> ```
> target_server_nodes:
>   - id: TN1
>     owner_as: AS2
>     http_server_port: 80
>
> non_target_server_nodes:
>   - id: NTN1
>     owner_as: AS1
> ```
> Both target and non-target servers operate the same way. The `owner_as` defines to which AS they belong. Both can define
> on which port their HTTP server runs by setting the via the `http_server_port`.
> Target nodes are attacked by all malicious nodes in the order in which they are listed.

## Attack Settings
> ### Attacker Nodes
> ```
> attacker_nodes:
>   - id: AN1
>     owner_as: AS1
>   - id: AN2
>     owner_as: AS3
>     data_rate: 1Mbps
>     packet_size: 378
>     source_port: 256
>     destination_port: 80
>     max_data_rate_fluctuation: 0.3
> ```
> Malicious nodes are defined as shown above. Like other node types, the `owner_as` defines into which AS they are inserted and is the only required property.
> The other properties control the behaviour of the node. `data_rate` and `packet_size` set the corresponding values for the generated
> attack traffic. 
> 
> The `source_port` and `destination_port` control the corresponding ports on attack vectors that utilize a 
> transport layer protocol (i.e., UDP flooding and TCP SYN flooding). You can either specify specific port numbers or have them be randomized.
> Ports are randomized by default but you can explicitly set them to randomize with the value -1
> 
> Lastly, the `max_data_rate_fluctuation` controls the fluctuation of the attack traffic's datarate based on a uniform distribution. Setting it to 
> zero will remove any fluctuation.
> 
> ### Attack Vectors
> All attacker nodes share the same attack schedule and send DDoS traffic to all nodes by cycling through the list of attack vectors defined in the `global_settings`.
> To further the configuration space, the properties `data_rate`, `packet_size`, `source_port`and `destination_port` are also available on the 
> attack vectors. Setting them there will grant them higher precedence than the settings on the attacker nodes, **whilst the vector in question is active.**
> ```
> global_settings:
>   attack:
>       attack_vectors:
>           - type: icmp_flooding
>           - type: udp_flooding
>             burst_duration_s: 120.0
>             target_switch_duration_s: 1.0
>             packet_size: 256
>             data_rate: 1Mbps
>             source_port: 9
>             destination_port: -1
>```
> The examples above represent a minimal attack vector configuration in form of the icmp flooding vector a vector that makes use
> of all properties as shown with the UDP flooding vector. Possible options for attack vectors are:
> - `icmp_flooding`
> - `udp_flooding`
> - `tcp_syn_flooding`
> 
> As stated above, properties on attack vectors have highest precedence. Therefore all attacker nodes will take on the settings
> specified on the vector during its duration. In this case, as soon as the UDP flooding vector is used, all attacker nodes
> will send packets of 256 Bytes, have a data rate of 1Mbps, use source port 9 and attack random ports on the target.
> 
> Similarly, the burst duration and target switch will override the corresponding values of the global settings section.
> 
> There are some special behaviours that need mentioning:
> - ICMP flooding ignores port settings due to not operating on the transport layer
> - TC SYN flooding ignores the packet size as it uses empty SYN packets of fixed size.
> - All attack vector types generate a corresponding response from the targets. In the case of TCP SYN however that is not a `SYN ACK` packet but rather a connection reset. As such, the vector is not suitable for use cases where you need the target's response to be `SYN ACK`.