#include "dpws-attacker-node.h"

#include "ns3/on-off-retarget-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("DPWSAttackerNode");

DPWSAttackerNode::DPWSAttackerNode(AttackerNodeConfiguration config,
                                   Ptr<Node> ns3Node,
                                   Ipv4Address address,
                                   std::vector<Ipv4Address> targetList,
                                   Ptr<AttackScheduleHelper> attackScheduler)
{
    m_config = config;
    m_ns3Node = ns3Node;
    m_assignedAddress = address;
    m_targetList = targetList;
    m_scheduleHelper = attackScheduler;
}

std::string
DPWSAttackerNode::GetNodeId()
{
    return m_config.GetNodeId();
}

InetSocketAddress
DPWSAttackerNode::GetRemoteAtTarget(AttackVector vector, int targetIndex)
{
    int port;
    switch (vector)
    {
    case AttackVector::udp_flooding:
        // port irrelevant, controlled directly on udp header not on socket
        port = 0;
        break;
    case AttackVector::icmp_flooding:
        // port irrelevant, ICMP runs on network layer, thus no src/dest ports
        port = 0;
        break;
    case AttackVector::tcp_syn_flooding:
        // port irrelevant, controlled directly on syn header not on socket
        port = 0;
        break;
    default:
        NS_FATAL_ERROR("Unknown vector, double check your configuration.");
    }

    InetSocketAddress remote(m_targetList[targetIndex], port);
    return remote;
}

void
DPWSAttackerNode::CreateApplications()
{
    // create basic on-off-retarget application per configured vector and set attributes
    int index = 0;
    for (AttackVectorConfiguration config : m_scheduleHelper->GetVectorConfigurations())
    {
        // initially point to first target address/port
        OnOffRetargetHelper oortHelper(GetSocketFactoryString(config.GetAttackVectorType()),
                                       GetRemoteAtTarget(config.GetAttackVectorType(), 0));

        // resolve precedence of application attributes between per-vector and per-node settings
        oortHelper.SetAttribute("DataRate", StringValue(ResolveDataRate(config.GetDataRate())));
        oortHelper.SetAttribute("PacketSize",
                                UintegerValue(ResolvePacketSize(config.GetPacketSize())));
        oortHelper.SetAttribute("SourcePort", IntegerValue(ResolveSourcePort(config.GetSourcePort())));
        oortHelper.SetAttribute("DestinationPort", IntegerValue(ResolveDestinationPort(config.GetDestinationPort())));

        // assign on/off timings
        auto [onTime, offTime] = m_scheduleHelper->GetOnOffTime(index);
        NS_LOG_DEBUG("creating app with on/off timings: " << onTime << ", " << offTime);
        oortHelper.SetAttribute("OnTime", StringValue(CreateConstantRandomVariableString(onTime)));
        oortHelper.SetAttribute("OffTime",
                                StringValue(CreateConstantRandomVariableString(offTime)));

        // configure behaviour regarding startup, remote change and data rate fluctuation
        oortHelper.SetAttribute("SkipFirstOffTime", BooleanValue(true));
        oortHelper.SetAttribute("SkipFirstScheduleAfterRemoteChange", BooleanValue(true));
        oortHelper.SetAttribute("MaxDataRateFluctuation", DoubleValue(m_config.GetMaxDataRateFluctuation()));

        // resolve attack-vector specific attributes (e.g., Protocol Identifier for vectors that use
        // Ipv4RawSocket socket type
        oortHelper.SetAttribute("AttackVector", IntegerValue(config.GetAttackVectorType()));
        if (config.GetAttackVectorType() == AttackVector::icmp_flooding)
        {
            // ICMP -> assigned protocol nr: 1
            // (https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml)
            oortHelper.SetAttribute("RawProtocol", IntegerValue(1));
        }
        if (config.GetAttackVectorType() == AttackVector::tcp_syn_flooding)
        {
            // TCP -> assigned protocol nr: 6
            // (https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml)
            oortHelper.SetAttribute("RawProtocol", IntegerValue(6));
        }
        if (config.GetAttackVectorType() == AttackVector::udp_flooding)
        {
            // TCP -> assigned protocol nr: 6
            // (https://www.iana.org/assignments/protocol-numbers/protocol-numbers.xhtml)
            oortHelper.SetAttribute("RawProtocol", IntegerValue(17));
        }

        m_applications.Add(oortHelper.Install(m_ns3Node));
        index++;
    }
}

std::string
DPWSAttackerNode::ResolveDataRate(std::string vectorValue)
{
    // per-vector value (if configured) gets precedence over per-attacker-node value
    if (!vectorValue.empty())
    {
        return vectorValue;
    }
    return m_config.GetDataRate();
}

int
DPWSAttackerNode::ResolveSourcePort(int vectorValue)
{
    // per-vector value (if configured) gets precedence over per-attacker-node value
    if (vectorValue != -2) {
        return vectorValue;
    }
    return m_config.GetSourcePort();
}

int
DPWSAttackerNode::ResolveDestinationPort(int vectorValue)
{
    // per-vector value (if configured) gets precedence over per-attacker-node value
    if (vectorValue != -2) {
        return vectorValue;
    }
    return m_config.GetDestinationPort();
}

int
DPWSAttackerNode::ResolvePacketSize(int vectorValue)
{
    // per-vector value (if configured) gets precedence over per-attacker-node value
    if (vectorValue != -1)
    {
        return vectorValue;
    }
    return m_config.GetPacketSize();
}

std::string
DPWSAttackerNode::GetSocketFactoryString(AttackVector vector)
{
    switch (vector)
    {
    case AttackVector::udp_flooding:
        return "ns3::Ipv4RawSocketFactory";
    case AttackVector::icmp_flooding:
        return "ns3::Ipv4RawSocketFactory";
    case AttackVector::tcp_syn_flooding:
        return "ns3::Ipv4RawSocketFactory";
    default:
        NS_FATAL_ERROR("Unknown vector, double check your configuration.");
    }
}

std::string
DPWSAttackerNode::CreateConstantRandomVariableString(double value)
{
    // Attributes "OnTime" and "OffTime" require constant variable specifications as a string with 1
    // decimal precision
    std::ostringstream stream;
    stream << "ns3::ConstantRandomVariable[Constant=" << std::setprecision(1) << std::fixed << value
           << "]";
    return stream.str();
}

void
DPWSAttackerNode::StartApplications(double stop)
{
    // start all applications. Start times are individually set, hence only stop is respected
    for (int i = 0; i < m_applications.GetN(); i++)
    {
        Ptr<OnOffRetargetApplication> app =
            DynamicCast<OnOffRetargetApplication>(m_applications.Get(i));

        double startTime = m_scheduleHelper->GetStartTime(i);

        app->SetStartTime(Seconds(startTime));
        app->SetStopTime(Seconds(stop));

        if (m_targetList.size() > 1)
        // avoid performance cost of switching if not actually multiple targets
        {
            double firstRemoteChangeInterval = m_scheduleHelper->GetNextRemoteChangeInterval(i, 0);
            // application is not started yet, or better put, scheduling the start does not lead to
            // an immediate start for all applications (only for the first vector), hence the
            // initial scheduling here needs to include the startTime

            NS_LOG_DEBUG(Simulator::Now().As(Time::S)
                         << " scheduling new remote at target: " << m_targetList[0]
                         << " with  targetIndex: " << 0 << ", and vectorApplicationIndex: " << i
                         << std::endl);

            // initial round of scheduling, done for each application
            Simulator::Schedule(Seconds(startTime + firstRemoteChangeInterval),
                                &DPWSAttackerNode::ScheduleDynamicTargetChange,
                                this,
                                app,
                                m_scheduleHelper,
                                m_targetList,
                                i,
                                m_scheduleHelper->GetNextTargetIndex(0));
        }
    }
}

void
DPWSAttackerNode::ScheduleDynamicTargetChange(Ptr<OnOffRetargetApplication> app,
                                              Ptr<AttackScheduleHelper> scheduleHelper,
                                              std::vector<Ipv4Address> targetList,
                                              int vectorApplicationIndex,
                                              int targetIndex)
{
    int peerPort = app->GetRemotePort();
    Ipv4Address nextAddress = targetList[targetIndex];
    app->SetRemote(InetSocketAddress(nextAddress, peerPort));

    // schedule next remote change for this particular application
    NS_LOG_DEBUG(Simulator::Now().As(Time::S)
                 << " scheduling new remote at target: " << targetList[targetIndex]
                 << " with  targetIndex: " << targetIndex
                 << ", and vectorApplicationIndex: " << vectorApplicationIndex << std::endl);

    Simulator::Schedule(
        Seconds(scheduleHelper->GetNextRemoteChangeInterval(vectorApplicationIndex, targetIndex)),
        &DPWSAttackerNode::ScheduleDynamicTargetChange,
        this,
        app,
        scheduleHelper,
        targetList,
        vectorApplicationIndex,
        scheduleHelper->GetNextTargetIndex(targetIndex));
}