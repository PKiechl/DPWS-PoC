#include "attack-schedule-helper.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("AttackScheduleHelper");

AttackScheduleHelper::AttackScheduleHelper(std::vector<AttackVectorConfiguration> configs,
                                           int numTargets)
{
    m_vectorConfigs = configs;
    m_numTargets = numTargets;
    BuildSchedule();
}

void
AttackScheduleHelper::BuildSchedule()
{
    CalculateOnOffTimes();
    CalculateApplicationStartTimes();
    CalculateRemoteSwitchIntervals();

    // only logs from here in this method
    NS_LOG_DEBUG("total cycle duration: " << m_totalCycleDuration);
    NS_LOG_DEBUG("AppStartTimes: ");
    int index = 0;
    for (auto a : m_startTimes)
    {
        NS_LOG_DEBUG(index << ": (vector " << index + 1 << ") - " << a);
        index++;
    }
    NS_LOG_DEBUG("OnOffTimes: ");
    index = 0;
    for (auto [a, b] : m_onOffTimes)
    {
        NS_LOG_DEBUG(index << ": (vector " << index + 1 << ") - " << a << ", " << b);
        index++;
    }
    NS_LOG_DEBUG("RemoteSwitchIntervals: ");
    index = 0;
    for (auto [a, b] : m_remoteSwitchTimes)
    {
        NS_LOG_DEBUG(index << ": (vector " << index + 1 << ") - " << a << ", " << b);
        index++;
    }
}

void
AttackScheduleHelper::CalculateApplicationStartTimes()
{
    // calculates the initial starting offset for each application
    double previousVectorDuration = 0.0;
    // application for first vector starts immediately, thus 0.0 initially
    for (AttackVectorConfiguration config : m_vectorConfigs)
    {
        m_startTimes.push_back(previousVectorDuration);
        // subsequent applications start once the previous vector(s) has(have) visited all targets
        // once
        previousVectorDuration +=
            (config.GetBurstDuration() + config.GetTargetSwitchDuration()) * m_numTargets;
    }
}

void
AttackScheduleHelper::CalculateOnOffTimes()
{
    double total = 0.0;
    for (AttackVectorConfiguration config : m_vectorConfigs)
    {
        // total cycle duration := time until each vector has attacked each target once
        double vectorDuration = config.GetBurstDuration() + config.GetTargetSwitchDuration();
        total += m_numTargets * vectorDuration;
    }
    m_totalCycleDuration = total;

    for (AttackVectorConfiguration config : m_vectorConfigs)
    {
        // each remote-change will re-set the application to onState, thus onTime is always simply
        // the burst duration
        double onTime = config.GetBurstDuration();
        // offTime: each offTime will be cut off when a target switch happens, because the
        // application is reset to onState. Thus the only offTime that matters is the one after the
        // vector has visited each target, because all other offTimes will be interrupted by the
        // remote-change.
        // offTime := total cycle duration
        //              - (numTargets * burstDuration)
        //              - ((numTargets-1) * switchDuration)
        // numTargets-1 because the last switchDuration is part of the offTime, as the application
        // will now switch over to onState again until it is this vector's turn again to attack
        double offTime = m_totalCycleDuration - (m_numTargets * config.GetBurstDuration()) -
                         ((m_numTargets - 1) * config.GetTargetSwitchDuration());
        m_onOffTimes.push_back({onTime, offTime});
    }
}

void
AttackScheduleHelper::CalculateRemoteSwitchIntervals()
{
    // calculate interval until next remote change for that vector's application
    // for each application: as long as it is not targeting the last target, the next remote change
    // will be in (burstDuration + targetSwitchDuration)
    // if it is targeting the last target, then the next remote change has to be:
    // (burstDuration + targetSwitchDuration + time until vector is next active) = (burstDuration +
    // offTime)
    int index = 0;
    for (AttackVectorConfiguration config : m_vectorConfigs)
    {
        double standardCase = config.GetBurstDuration() + config.GetTargetSwitchDuration();
        auto [onTime, offTime] = m_onOffTimes[index];
        double lastTargetCase = config.GetBurstDuration() + offTime;
        m_remoteSwitchTimes.push_back({standardCase, lastTargetCase});
        index++;
    }
}

double
AttackScheduleHelper::GetNextRemoteChangeInterval(int vectorIndex, int targetIndex)
{
    auto [standardCase, lastTargetCase] = m_remoteSwitchTimes[vectorIndex];
    if (targetIndex + 1 == m_numTargets)
    {
        // on last target
        return lastTargetCase;
    }
    else
    {
        return standardCase;
    }
}

std::pair<double, double>
AttackScheduleHelper::GetOnOffTime(int index)
{
    return m_onOffTimes[index];
}

double
AttackScheduleHelper::GetStartTime(int index)
{
    return m_startTimes[index];
}

std::vector<AttackVectorConfiguration>
AttackScheduleHelper::GetVectorConfigurations()
{
    return m_vectorConfigs;
}

int
AttackScheduleHelper::GetNextTargetIndex(int index)
{
    if (index + 1 == m_numTargets)
    {
        // loop around to 0, if index is currently pointing to last target in list
        return 0;
    }
    index += 1;
    return index;
}