/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <reconstruction/modules/EventT0Combiner/EventT0Combiner.h>

#include <cmath>

using namespace Belle2;

REG_MODULE(EventT0Combiner);

EventT0CombinerModule::EventT0CombinerModule() : Module()
{
  setDescription("Module to combine the EventT0 values from multiple sub-detectors");

  addParam("combinationLogic", m_paramCombinationMode, "Method of how the final T0 is selected.\n"
           "Currently '" + m_combinationModePreferSVD + ", " + m_combinationModePreferCDC + "' and '" + m_combinationModeCombineSVDandECL +
           "' is available\n" +
           m_combinationModePreferSVD + ": the SVD t0 value (if available) will be set as the final T0 value."
           "Only if no SVD value could be found "
           "(which is very rare for BBbar events, and around 5% of low multiplicity events), the best ECL value will be set\n" +
           m_combinationModeCombineSVDandECL + ": In this mode, the SVD t0 value (if available) will be used to "
           "select the ECL t0 information which is closest in time "
           "to the best SVD value and these two values will be combined to one final value." +
           m_paramCombinationMode);

  setPropertyFlags(c_ParallelProcessingCertified);
}

void EventT0CombinerModule::event()
{
  if (!m_eventT0.isValid()) {
    B2DEBUG(20, "EventT0 object not created, cannot do EventT0 combination");
    return;
  }

  // We have an SVD based EventT0 and it currently is set as *THE* EventT0 -> nothing to do
  if (m_eventT0->isSVDEventT0()) {
    return;
  }

  // If we don't have an SVD based EventT0, the second choice is the EventT0 estimate using CDC information calculabed by the
  // FullGridChi2TrackTimeExtractor method. In principle, this algorithm can create EventT0 estimates using two methods:
  // "grid" and "chi2". We are only interested in the latter one.
  // If no SVD based EventT0 is present, but a CDC based one using the "chi2" algorithm is available -> nothing to do
  if (m_eventT0->isCDCEventT0()) {
    const auto bestCDCT0 = m_eventT0->getBestCDCTemporaryEventT0();
    if ((*bestCDCT0).algorithm == "chi2") {
      return;
    }
  }

  const auto& bestECLT0 = m_eventT0->getBestECLTemporaryEventT0();
  const auto& cdcT0Candidates = m_eventT0->getTemporaryEventT0s(Const::CDC);
  const auto hitBasedCDCT0Candiate = std::find_if(cdcT0Candidates.begin(), cdcT0Candidates.end(), [](const auto & a) { return a.algorithm == "hit based";});

  // Strategy in case none of the SVD based or the CDC chi2 based EventT0 values is available:
  // 1) If we have both an EventT0 estimate from ECL and a CDC hit based value, combine the two
  // 2) If we only have one of the two, take that value
  // 3) If we don't have either, we have a problem -> issue a B2WARNING and clear the EventT0
  // If we arrive at 3), this means that we could only have TOP EventT0, or an EventT0 from a
  // CDC based algorithm other than "hit based" or "chi2", and so far we don't want to use these.
  if (bestECLT0 and hitBasedCDCT0Candiate != cdcT0Candidates.end()) {
    const auto combined = computeCombination({ *bestECLT0, *hitBasedCDCT0Candiate });
    m_eventT0->setEventT0(combined);
    return;
  } else if (bestECLT0 and hitBasedCDCT0Candiate == cdcT0Candidates.end()) {
    m_eventT0->setEventT0(*bestECLT0);
    return;
  } else if (hitBasedCDCT0Candiate != cdcT0Candidates.end() and not bestECLT0) {
    m_eventT0->setEventT0(*hitBasedCDCT0Candiate);
    return;
  } else {
    B2WARNING("There is no EventT0 from neither \n" \
              " * the SVD based algorithm\n" \
              " * the CDC based chi^2 algorithm\n" \
              " * the CDC based hit-based algorithm\n" \
              " * the ECL algorithm.\n" \
              "Thus, no EventT0 value can be calculated.");
    m_eventT0->clearEventT0();
  }


}

EventT0::EventT0Component EventT0CombinerModule::computeCombination(std::vector<EventT0::EventT0Component> measurements) const
{
  if (measurements.size() == 0) {
    B2FATAL("Need at least one EvenT0 Measurement to do a sensible combination.");
  }

  double eventT0 = 0.0f;
  double preFactor = 0.0f;

  Const::DetectorSet usedDetectorSet;

  for (auto const& meas : measurements) {
    usedDetectorSet += meas.detectorSet;
    const double oneOverUncertaintySquared = 1.0f / std::pow(meas.eventT0Uncertainty, 2.0);
    eventT0 += meas.eventT0 * oneOverUncertaintySquared;
    preFactor += oneOverUncertaintySquared;
  }

  eventT0 /= preFactor;
  const auto eventT0unc = std::sqrt(1.0f / preFactor);

  return EventT0::EventT0Component(eventT0, eventT0unc, usedDetectorSet);
}


