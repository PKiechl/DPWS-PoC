#ifndef ATTACK_SCHEDULE_HELPER_H
#define ATTACK_SCHEDULE_HELPER_H

#include "ns3/attack-vector-configuration.h"
#include "ns3/core-module.h"
#include "ns3/onoff-retarget-application.h"

namespace ns3
{

/*
 * This helper class provides a central location for calculations regarding attack scheduling,
 * including:
 *      - start times (offsets) of the individual attack applications based on which vector a
 *          given application implements.
 *      - on/off times of individual attack applications, taking into account burst- and switch-
 *          durations.
 *      - intervals for scheduling a dynamic target switch of individual applications (i.e., switch
 *          target).
 *
 * This way, these calculations only need to be performed once and not each time a new attacker node
 * is instantiated.
 */

class AttackScheduleHelper : public SimpleRefCount<AttackScheduleHelper>
{
  public:
    AttackScheduleHelper(std::vector<AttackVectorConfiguration> configs, int numTargets);

    std::vector<AttackVectorConfiguration> GetVectorConfigurations();
    // simply return m_vectorConfigs
    double GetNextRemoteChangeInterval(int vectorIndex, int targetIndex);
    // returns interval determining when the next remote change is supposed to be scheduled, taking
    // into account if the last target has been visited, and thus a (if other vectors are configured)
    // longer interval has to be scheduled until this particular vector attacks again,
    std::pair<double, double> GetOnOffTime(int index);
    // retrieve pair for application (attack vector) at specific index
    double GetStartTime(int index);
    // returns start time for application (attack vector) at specific index
    int GetNextTargetIndex(int index);
    // returns index of next target based on total number of targets.

  private:
    void BuildSchedule();
    // sequentially call calculation methods below
    void CalculateApplicationStartTimes();
    void CalculateOnOffTimes();
    void CalculateRemoteSwitchIntervals();
    // cf. .cc file implementation for detailed comments on what each function does.

  protected:
    std::vector<AttackVectorConfiguration> m_vectorConfigs;
    int m_numTargets;
    double m_totalCycleDuration;
    // total cycle duration => time until each vector has cycled through all targets

    // the following vectors are per attack vector, in same order as vectorConfigs
    std::vector<std::pair<double, double>> m_onOffTimes;
    std::vector<double> m_startTimes;
    std::vector<std::pair<double, double>> m_remoteSwitchTimes;
};

} // namespace ns3

#endif /* ATTACK_SCHEDULE_HELPER_H */