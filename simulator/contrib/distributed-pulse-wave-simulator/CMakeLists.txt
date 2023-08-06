# instruction for how to set up a module: https://www.nsnam.org/docs/manual/html/new-modules.html

set(source_files
        # dynamic traffic application
        helper/on-off-retarget-helper.cc
        model/onoff-retarget-application.cc
        # benign traffic model (https://github.com/saulodamata/ns-3-http-traffic-generator)
        helper/http-client-server-helper.cc
        model/http-client.cc
        model/http-server.cc
        model/http-header.cc
        # configuration
        service/configuration/config-file-reader.cc
        service/configuration/objects/configuration.cc
        service/configuration/objects/node-configuration.cc
        service/configuration/objects/central-network-configuration.cc
        service/configuration/objects/global-settings-configuration.cc
        service/configuration/objects/server-node-configuration.cc
        service/configuration/objects/benign-node-configuration.cc
        service/configuration/objects/attacker-node-configuration.cc
        service/configuration/objects/autonomous-system-configuration.cc
        service/configuration/objects/attack-vector-configuration.cc
        # address service
        service/address/address-provider.cc
        # mapper service
        service/mapper/node-lookup-mapper.cc
        # scheduling service
        service/scheduling/attack-schedule-helper.cc
        # wrapper classes
        wrapper/central-network/implementations/full-mesh-central-network.cc
        wrapper/central-network/implementations/randomized-partial-mesh-central-network.cc
        wrapper/autonomous-systems/autonomous-system.cc
        wrapper/autonomous-systems/implementations/csma-autonomous-system.cc
        wrapper/autonomous-systems/implementations/p2p-autonomous-system.cc
        wrapper/dpws-nodes/dpws-node.cc
        wrapper/dpws-nodes/implementations/dpws-server-node.cc
        wrapper/dpws-nodes/implementations/dpws-benign-node.cc
        wrapper/dpws-nodes/implementations/dpws-attacker-node.cc
        )

set(header_files
        # dynamic traffic application
        helper/on-off-retarget-helper.h
        model/onoff-retarget-application.h
        # benign traffic model (https://github.com/saulodamata/ns-3-http-traffic-generator)
        helper/http-client-server-helper.h
        model/http-client.h
        model/http-server.h
        model/http-header.h
        # configuration service
        service/configuration/config-file-reader.h
        service/configuration/objects/configuration.h
        service/configuration/objects/node-configuration.h
        service/configuration/objects/central-network-configuration.h
        service/configuration/objects/global-settings-configuration.h
        service/configuration/objects/server-node-configuration.h
        service/configuration/objects/benign-node-configuration.h
        service/configuration/objects/attacker-node-configuration.h
        service/configuration/objects/autonomous-system-configuration.h
        service/configuration/objects/attack-vector-configuration.h
        # address service
        service/address/address-provider.h
        # mapper service
        service/mapper/node-lookup-mapper.h
        # scheduling service
        service/scheduling/attack-schedule-helper.h
        # wrapper classes
        wrapper/central-network/central-network.h
        wrapper/central-network/implementations/full-mesh-central-network.h
        wrapper/central-network/implementations/randomized-partial-mesh-central-network.h
        wrapper/autonomous-systems/autonomous-system.h
        wrapper/autonomous-systems/implementations/p2p-autonomous-system.h
        wrapper/dpws-nodes/dpws-node.h
        wrapper/dpws-nodes/implementations/dpws-server-node.h
        wrapper/dpws-nodes/implementations/dpws-benign-node.h
        wrapper/dpws-nodes/implementations/dpws-attacker-node.h
        )


check_include_file_cxx(stdint.h HAVE_STDINT_H)
if (HAVE_STDINT_H)
    add_definitions(-DHAVE_STDINT_H)
endif ()


build_lib(
        LIBNAME distributed-pulse-wave-simulator
        SOURCE_FILES ${source_files}
        HEADER_FILES ${header_files}
        LIBRARIES_TO_LINK
        ${libinternet}
        ${libpoint-to-point}
        ${libcsma}
        ${libnetwork}
        ${libapplications}
)

