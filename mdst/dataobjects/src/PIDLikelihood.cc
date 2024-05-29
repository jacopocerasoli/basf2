/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#include <mdst/dataobjects/PIDLikelihood.h>
#include <framework/logging/Logger.h>

#include <TROOT.h>
#include <TColor.h>
#include <TParticlePDG.h>

#include <cmath>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <vector>

using namespace std;
using namespace Belle2;

PIDLikelihood::PIDLikelihood()
{
  for (unsigned short i = 0; i < Const::PIDDetectors::c_size; i++) {
    for (unsigned int k = 0; k < Const::ChargedStable::c_SetSize; k++) {
      m_logl[i][k] = 0.0;
    }
  }
}


void PIDLikelihood::setLogLikelihood(Const::EDetector det,
                                     const Const::ChargedStable& part,
                                     float logl)
{
  int index = Const::PIDDetectors::set().getIndex(det);
  if (index < 0) {
    B2ERROR("PIDLikelihood::setLogLikelihood: detector is not a PID device");
    return;
  }
  if (logl != logl or logl == INFINITY) {
    B2ERROR("PIDLikelihood::setLogLikelihood: log-likelihood for detector " << det << " is " << logl <<
            " (i.e. +inf or NaN)! Ignoring this value. ("
            << Const::SVD << "=SVD, " << Const::CDC << "=CDC, " << Const::TOP << "=TOP, "
            << Const::ARICH << "=ARICH, " << Const::ECL << "=ECL, " << Const::KLM << "=KLM)");

    return;
  }
  m_detectors += det;
  m_logl[index][part.getIndex()] = logl;
}


void PIDLikelihood::subtractMaximum()
{
  for (auto& detectorLLs : m_logl) {
    auto maxLL = detectorLLs[0];
    for (auto LL : detectorLLs) maxLL = std::max(maxLL, LL);
    for (auto& LL : detectorLLs) LL -= maxLL;
  }
}


bool PIDLikelihood::isAvailable(Const::PIDDetectorSet set) const
{
  for (const auto& detector : set) {
    if (m_detectors.contains(detector)) return true;
  }
  return false;
}


float PIDLikelihood::getLogL(const Const::ChargedStable& part,
                             Const::PIDDetectorSet set) const
{
  float result = 0;
  for (Const::DetectorSet::Iterator it = Const::PIDDetectorSet::set().begin();
       it != Const::PIDDetectorSet::set().end(); ++it) {
    if (set.contains(it))
      result += m_logl[it.getIndex()][part.getIndex()];
  }
  return result;
}


double PIDLikelihood::getProbability(const Const::ChargedStable& p1,
                                     const Const::ChargedStable& p2,
                                     double ratio,
                                     Const::PIDDetectorSet set) const
{
  if (ratio < 0) {
    B2ERROR("PIDLikelihood::probability argument 'ratio' is given with negative value");
    return 0;
  }
  if (ratio == 0) return 0;

  double dlogl = getLogL(p2, set) - getLogL(p1, set);
  double result = 0;
  if (dlogl < 0) {
    double elogl = exp(dlogl);
    result = ratio / (ratio + elogl);
  } else {
    double elogl = exp(-dlogl) * ratio; // to prevent overflow for very large dlogl
    result = elogl / (1.0 + elogl);
  }

  return result;
}

double PIDLikelihood::getProbability(const Const::ChargedStable& part,
                                     const double* fractions,
                                     Const::PIDDetectorSet detSet) const
{
  double prob[Const::ChargedStable::c_SetSize];
  probability(prob, fractions, detSet);

  int k = part.getIndex();
  if (k < 0) return 0;

  return prob[k];

}

double PIDLikelihood::getDeltaLogLGlobal(const Const::ChargedStable& part,
                                         const double* fractions,
                                         Const::PIDDetectorSet detSet) const
{
  // Defined as log(p / (1 - p)) where p is global PID probability.
  // In order to save the range of return values it must be implemented as below.

  int k = part.getIndex();
  if (k < 0) return -INFINITY; // p = 0

  if (fractions and fractions[k] <= 0) return -INFINITY; // p = 0

  double logL_part = getLogL(part, detSet);
  std::vector<double> logLs; // all other log likelihoods with priors > 0
  std::vector<double> priorRatios;
  for (int i = 0; i < static_cast<int>(Const::ChargedStable::c_SetSize); i++) {
    if (i == k) continue;
    double ratio = fractions ? fractions[i] / fractions[k] : 1;
    if (ratio <= 0) continue;
    priorRatios.push_back(ratio);
    logLs.push_back(getLogL(Const::chargedStableSet.at(i), detSet));
  }
  if (logLs.empty()) return +INFINITY; // p = 1

  double logL_max = logLs[0]; // maximal log likelihood of the others
  for (auto logl : logLs) logL_max = std::max(logl, logL_max);

  double sum = 0;
  for (size_t i = 0; i < logLs.size(); i++) sum += priorRatios[i] * exp(logLs[i] - logL_max);

  return logL_part - logL_max - log(sum);
}


Const::ChargedStable PIDLikelihood::getMostLikely(const double* fractions,
                                                  Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::ChargedStable::c_SetSize;
  double prob[n];
  probability(prob, fractions, detSet);

  int k = 0;
  double maxProb = prob[k];
  for (unsigned i = 0; i < n; ++i) {
    if (prob[i] > maxProb) {maxProb = prob[i]; k = i;}
  }
  return Const::chargedStableSet.at(k);

}


void PIDLikelihood::probability(double probabilities[],
                                const double* fractions,
                                Const::PIDDetectorSet detSet) const
{
  const unsigned int n = Const::ChargedStable::c_SetSize;
  double frac[n];
  if (!fractions) {
    for (unsigned int i = 0; i < n; ++i) frac[i] = 1.0; // normalization not needed
    fractions = frac;
  }

  double logL[n];
  double logLmax = 0;
  bool hasMax = false;
  for (unsigned i = 0; i < n; ++i) {
    logL[i] = 0;
    if (fractions[i] > 0) {
      logL[i] = getLogL(Const::chargedStableSet.at(i), detSet);
      if (!hasMax || logL[i] > logLmax) {
        logLmax = logL[i];
        hasMax = true;
      }
    }
  }

  double norm = 0;
  for (unsigned i = 0; i < n; ++i) {
    probabilities[i] = 0;
    if (fractions[i] > 0) probabilities[i] = exp(logL[i] - logLmax) * fractions[i];
    norm += probabilities[i];
  }
  if (norm == 0) return;

  for (unsigned i = 0; i < n; ++i) {
    probabilities[i] /= norm;
  }

}


void PIDLikelihood::printArray() const
{

  const string detectorName[Const::PIDDetectors::c_size] =
  {"SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"};
  string hline("-------");
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    hline += "-----------";
  string Hline("=======");
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    Hline += "===========";

  cout << Hline << endl;

  cout << "PDGcode";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    cout << setw(10) << Const::chargedStableSet.at(i).getPDGCode() << " ";
  cout << endl;

  cout << Hline << endl;

  float sum_logl[Const::ChargedStable::c_SetSize];
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) sum_logl[i] = 0;

  for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++) {
    cout << setw(7) << detectorName[k];
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
      cout << setw(10) << setprecision(4) << m_logl[k][i] << " ";
      sum_logl[i] += m_logl[k][i];
    }
    cout << endl;
  }

  cout << hline << endl;

  cout << setw(7) << "sum";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
    cout << setw(10) << setprecision(4) << sum_logl[i] << " ";
  cout << endl;

  if (isAvailable(Const::SVD) or isAvailable(Const::CDC) or isAvailable(Const::TOP) or
      isAvailable(Const::ARICH) or isAvailable(Const::ECL) or isAvailable(Const::KLM)) {
    unsigned k = 0;
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++)
      if (sum_logl[i] > sum_logl[k]) k = i;
    unsigned pos = 11 * (k + 1) + 3;
    Hline.replace(pos, 1, "^");
  }
  cout << Hline << endl;

}

std::string PIDLikelihood::getInfoHTML() const
{
  const string detectorName[Const::PIDDetectors::c_size] = {"SVD", "CDC", "TOP", "ARICH", "ECL", "KLM"};

  std::stringstream stream;
  stream << std::setprecision(4);

  //colors from event display
  std::string colour[Const::ChargedStable::c_SetSize];
  colour[0] = gROOT->GetColor(kAzure)->AsHexString();
  colour[1] = gROOT->GetColor(kCyan + 1)->AsHexString();
  colour[2] = gROOT->GetColor(kGray + 1)->AsHexString();
  colour[3] = gROOT->GetColor(kRed + 1)->AsHexString();
  colour[4] = gROOT->GetColor(kOrange - 2)->AsHexString();
  colour[5] = gROOT->GetColor(kMagenta)->AsHexString();

  stream << "<b>Probabilities</b><br>";
  stream << "<table border=0 width=100%>";
  stream << "<tr><td>All</td><td>";
  stream << "<table cellspacing=1 border=0 width=100%><tr>";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
    double p = getProbability(Const::chargedStableSet.at(i));
    stream << "<td bgcolor=\"" << colour[i] << "\" width=" << p * 100 << "%></td>";
  }
  stream << "</tr></table></td></tr>";

  for (Const::DetectorSet::Iterator it = Const::PIDDetectorSet::set().begin();
       it != Const::PIDDetectorSet::set().end(); ++it) {
    auto det = *it;
    stream << "<tr><td>" << detectorName[it.getIndex()] << "</td><td>";
    if (!isAvailable(det)) {
      stream << "</td></tr>";
      continue;
    }
    stream << "<table cellspacing=1 border=0 width=100%><tr>";
    for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
      double p = getProbability(Const::chargedStableSet.at(i), 0, det);
      stream << "<td bgcolor=\"" << colour[i] << "\" width=" << p * 100 << "%></td>";
    }
    stream << "</tr></table></td></tr>";
  }
  stream << "</table>\n";

  stream << "<b>Log-Likelihoods</b><br>";
  stream << "<table>";
  stream << "<tr><th>PID / Detector</th>";
  for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++)
    stream << "<th>" << detectorName[k] << "</th>";
  stream << "</tr>";
  for (unsigned i = 0; i < Const::ChargedStable::c_SetSize; i++) {
    stream << "<tr>";
    stream << "<td bgcolor=\"" << colour[i] << "\">" << Const::chargedStableSet.at(i).getParticlePDG()->GetName() << "</td>";
    for (unsigned k = 0; k < Const::PIDDetectors::c_size; k++) {
      stream << "<td>" << m_logl[k][i] << "</td>";
    }
    stream << "</tr>";
  }
  stream << "</table>";
  stream << "<br>";

  return stream.str();
}


