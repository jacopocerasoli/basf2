/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

//THIS MODULE
#include <dqm/analysis/modules/DQMHistAnalysisECLSummary.h>

//ECL
#include <ecl/geometry/ECLGeometryPar.h>

//ROOT
#include <TProfile.h>
#include <TStyle.h>
#include <TLine.h>
#include <TText.h>

//std
#include <iostream>
#include <fstream>

//boost
#include "boost/format.hpp"

using namespace Belle2;

REG_MODULE(DQMHistAnalysisECLSummary);

DQMHistAnalysisECLSummaryModule::DQMHistAnalysisECLSummaryModule()
  : DQMHistAnalysisModule(),
    m_neighbours_obj("F", 0.1)
{

  B2DEBUG(20, "DQMHistAnalysisECLSummary: Constructor done.");
  addParam("pvPrefix", m_pvPrefix, "Prefix to use for PVs registered by this module",
           std::string("ECL:DQM:channels_info:"));
  addParam("useChannelMask", m_useChannelMask,
           "Mask Cell IDs based on information from ECL PVs",
           true);
}


DQMHistAnalysisECLSummaryModule::~DQMHistAnalysisECLSummaryModule()
{
}

void DQMHistAnalysisECLSummaryModule::initialize()
{
  m_mapper.initFromFile();

  //=== Set up ECL alarms and corresponding PVs

  m_ecl_alarms = {
    {"dead",     "#splitline{dead}{channels}",         0,  0,  1e5},
    {"cold",     "#splitline{cold}{channels}",         0,  1,  1e5},
    {"hot",      "#splitline{hot}{channels}",          25, 50, 1e5},
    {"bad_chi2", "#splitline{bad #chi^{2}}{channels}", 5,  10, 1e6},
    {"bad_fit",  "#splitline{fit incon-}{sistencies}", 5,  10, 0  },
  };

  // Prepare EPICS PVs
  for (auto& alarm : m_ecl_alarms) {
    // By crate
    for (int crate_id = 1; crate_id <= ECL::ECL_CRATES; crate_id++) {
      std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
      registerEpicsPV(m_pvPrefix + pv_name, pv_name);
    }
    // Totals
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = (boost::format("%s:%s") % ecl_part % alarm.name).str();
      registerEpicsPV(m_pvPrefix + pv_name, pv_name);
    }
    // Masked Cell IDs
    std::string mask_pv_name = (boost::format("mask:%s") % alarm.name).str();
    registerEpicsPV(m_pvPrefix + mask_pv_name, mask_pv_name);
  }

  m_monObj = getMonitoringObject("ecl");

  //=== Set up the histogram to indicate alarm status

  TString title = "#splitline{ECL errors monitoring}";
  title += "{E - Error, W - Warning, L - Low statistics}";
  title += ";ECLCollector ID (same as Crate ID)";
  h_channels_summary = new TH2F("channels_summary", title,
                                ECL::ECL_CRATES, 1, ECL::ECL_CRATES + 1,
                                m_ecl_alarms.size(), 0, m_ecl_alarms.size());

  h_channels_summary->SetStats(0);
  h_channels_summary->SetMinimum(0);
  h_channels_summary->SetMaximum(1);

  //=== Set X axis labels

  for (int i = 1; i <= ECL::ECL_CRATES; i++) {
    h_channels_summary->GetXaxis()->SetBinLabel(i, std::to_string(i).c_str());
  }
  h_channels_summary->LabelsOption("v", "X"); // Rotate X axis labels 90 degrees
  h_channels_summary->SetTickLength(0, "XY");

  //=== Customize offsets and margins

  h_channels_summary->GetXaxis()->SetTitleOffset(0.95);
  h_channels_summary->GetXaxis()->SetTitleSize(0.05);
  h_channels_summary->GetXaxis()->SetLabelSize(0.04);
  h_channels_summary->GetYaxis()->SetLabelSize(0.06);

  c_channels_summary = new TCanvas("ECL/c_channels_summary_analysis");
  c_channels_summary->SetTopMargin(0.10);
  c_channels_summary->SetLeftMargin(0.20);
  c_channels_summary->SetRightMargin(0.005);
  c_channels_summary->SetBottomMargin(0.10);

  //=== Additional canvases/histograms to display which channels have problems
  c_occupancy = new TCanvas("ECL/c_cid_Thr5MeV_overlaid_analysis");
  c_bad_chi2  = new TCanvas("ECL/c_bad_quality_overlaid_analysis");

  h_bad_occ_overlay = new TH1F("bad_occ_overlay", "", ECL::ECL_TOTAL_CHANNELS,
                               1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_occ_overlay->SetLineColor(kRed);
  h_bad_occ_overlay->SetLineStyle(kDashed);
  h_bad_occ_overlay->SetFillColor(kRed);
  h_bad_occ_overlay->SetFillStyle(3007);

  h_bad_chi2_overlay = new TH1F("bad_chi2_overlay", "", ECL::ECL_TOTAL_CHANNELS,
                                1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_chi2_overlay->SetLineColor(kRed);
  h_bad_chi2_overlay->SetLineStyle(kDashed);
  h_bad_chi2_overlay->SetFillColor(kRed);
  h_bad_chi2_overlay->SetFillStyle(3007);

  h_bad_occ_overlay_green = new TH1F("bad_occ_overlay_green", "", ECL::ECL_TOTAL_CHANNELS,
                                     1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_occ_overlay_green->SetLineColor(kGreen);
  h_bad_occ_overlay_green->SetLineStyle(kDotted);
  h_bad_occ_overlay_green->SetFillColor(kGreen);
  h_bad_occ_overlay_green->SetFillStyle(3013);

  h_bad_chi2_overlay_green = new TH1F("bad_chi2_overlay_green", "", ECL::ECL_TOTAL_CHANNELS,
                                      1, ECL::ECL_TOTAL_CHANNELS + 1);
  h_bad_chi2_overlay_green->SetLineColor(kGreen);
  h_bad_chi2_overlay_green->SetLineStyle(kDotted);
  h_bad_chi2_overlay_green->SetFillColor(kGreen);
  h_bad_chi2_overlay_green->SetFillStyle(3013);

  B2DEBUG(20, "DQMHistAnalysisECLSummary: initialized.");
}

void DQMHistAnalysisECLSummaryModule::beginRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLSummary: beginRun called.");

  //=== Update m_ecl_alarms based on PV limits

  updateAlarmConfig();

  //=== Set Y axis labels from m_ecl_alarms

  for (size_t i = 0; i < m_ecl_alarms.size(); i++) {
    TString label = m_ecl_alarms[i].title;
    label += " > ";
    label += m_ecl_alarms[i].alarm_limit;
    h_channels_summary->GetYaxis()->SetBinLabel(i + 1, label);
  }
}

void DQMHistAnalysisECLSummaryModule::event()
{
  h_channels_summary->Reset();

  TH1* h_total_events = findHist("ECL/event");
  if (!h_total_events) return;
  m_total_events = h_total_events->GetEntries();

  // [alarm_type][crate_id - 1] -> number of problematic channels in that crate
  std::vector< std::vector<int> > alarm_counts = updateAlarmCounts();

  //=== Set warning and error indicators on the histogram

  const double HISTCOLOR_RED    = 0.9;
  const double HISTCOLOR_GREEN  = 0.45;
  const double HISTCOLOR_ORANGE = 0.65;
  const double HISTCOLOR_BLUE   = 0.01;

  std::vector<TText*> labels;

  for (size_t alarm_idx = 0; alarm_idx < alarm_counts.size(); alarm_idx++) {
    for (size_t crate = 0; crate < alarm_counts[alarm_idx].size(); crate++) {
      double color;
      char label_text[2] = {0};
      int alarm_limit   = m_ecl_alarms[alarm_idx].alarm_limit;
      int warning_limit = m_ecl_alarms[alarm_idx].warning_limit;
      if (m_total_events < m_ecl_alarms[alarm_idx].required_statistics) {
        color = HISTCOLOR_BLUE;
        label_text[0] = 'L';
      } else if (alarm_counts[alarm_idx][crate] > alarm_limit) {
        color = HISTCOLOR_RED;
        label_text[0] = 'E';
      } else if (alarm_counts[alarm_idx][crate] > warning_limit) {
        color = HISTCOLOR_ORANGE;
        label_text[0] = 'W';
      } else {
        color = HISTCOLOR_GREEN;
      }
      if (label_text[0] == 'E' || label_text[0] == 'W') {
        B2DEBUG(100, "Non-zero (" << alarm_counts[alarm_idx][crate]
                << ") for alarm_idx, crate = " << alarm_idx << ", " <<  crate);
      }
      h_channels_summary->SetBinContent(crate + 1, alarm_idx + 1, color);
      if (label_text[0] != 0) {
        auto text = new TText((crate + 1.5), (alarm_idx + 0.5), label_text);
        text->SetTextColor(kWhite);
        text->SetTextAlign(22); // centered
        labels.push_back(text);
      }
    }
  }

  //=== Draw histogram, labels and grid

  // Customize title
  int gstyle_title_h = gStyle->GetTitleH();
  int gstyle_title_x = gStyle->GetTitleX();
  int gstyle_title_y = gStyle->GetTitleY();
  gStyle->SetTitleH(0.04);
  gStyle->SetTitleX(0.60);
  gStyle->SetTitleY(1.00);

  c_channels_summary->Clear();
  c_channels_summary->cd();

  //=== Prepare special style objects to use correct color palette
  //    and use it only for this histogram
  // Based on https://root-forum.cern.ch/t/different-color-palettes-for-different-plots-with-texec/5250/3
  // and https://root.cern/doc/master/multipalette_8C.html

  if (!m_ecl_style) delete m_ecl_style;
  if (!m_default_style) delete m_default_style;

  m_ecl_style     = new TExec("ecl_style",
                              "gStyle->SetPalette(kRainBow);"
                              "channels_summary->SetDrawOption(\"col\");");
  h_channels_summary->GetListOfFunctions()->Add(m_ecl_style);
  m_default_style = new TExec("default_style",
                              "gStyle->SetPalette(kBird);");

  //=== Draw with special style
  //    https://root.cern.ch/js/latest/examples.htm#th2_colpal77
  h_channels_summary->Draw("");
  h_channels_summary->Draw("colpal55;same");
  for (auto& text : labels) {
    text->Draw();
  }
  drawGrid(h_channels_summary);
  m_default_style->Draw("same");

  //
  c_channels_summary->Modified();
  c_channels_summary->Update();
  c_channels_summary->Draw();

  gStyle->SetTitleH(gstyle_title_h);
  gStyle->SetTitleX(gstyle_title_x);
  gStyle->SetTitleY(gstyle_title_y);
}

void DQMHistAnalysisECLSummaryModule::endRun()
{
  B2DEBUG(20, "DQMHistAnalysisECLSummary: endRun called");

  updateAlarmCounts(true);
}


void DQMHistAnalysisECLSummaryModule::terminate()
{
  B2DEBUG(20, "terminate called");
  delete c_channels_summary;
  delete c_occupancy;
  delete c_bad_chi2;
  delete h_bad_occ_overlay;
  delete h_bad_chi2_overlay;
}

std::pair<int, DQMHistAnalysisECLSummaryModule::ECLAlarmType> DQMHistAnalysisECLSummaryModule::getAlarmByName(std::string name)
{
  int index = 0;
  for (auto& alarm_info : m_ecl_alarms) {
    if (alarm_info.name == name) return {index, alarm_info};
    index++;
  }
  // TODO: Do only one of those?
  B2FATAL("Could not get ECL alarm " + name);
  throw std::out_of_range("Could not get ECL alarm " + name);
}

void DQMHistAnalysisECLSummaryModule::updateAlarmConfig()
{
  static const int MASK_SIZE = 200;
  /* structure to get an array of long values from EPICS */
  struct dbr_sts_long_array {
    dbr_short_t     status;                 /* status of value */
    dbr_short_t     severity;               /* severity of alarm */
    dbr_long_t      value[MASK_SIZE];       /* current value */
  };

  static std::map<std::string, dbr_ctrl_long> limits_info;
  static std::map<std::string, dbr_sts_long_array> mask_info;

  for (auto& alarm : m_ecl_alarms) {
    // In the current version, use only first crate PV to get alarm limits
    int crate_id = 1;
    std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
    chid limits_chid = getEpicsPVChID(pv_name);

    if (limits_chid == nullptr) return;
    // ca_get data is only valid after ca_pend_io
    auto r = ca_get(DBR_CTRL_LONG, limits_chid, &limits_info[pv_name]);
    if (r != ECA_NORMAL) return;

    std::string mask_pv_name = (boost::format("mask:%s") % alarm.name).str();
    chid mask_chid = getEpicsPVChID(mask_pv_name);

    if (mask_chid == nullptr) return;
    // ca_array_get data is only valid after ca_pend_io
    r = ca_array_get(DBR_STS_LONG, MASK_SIZE, mask_chid, &mask_info[alarm.name]);
    if (r != ECA_NORMAL) return;
  }

  auto r = ca_pend_io(5.0);
  if (r != ECA_NORMAL) {
    B2WARNING("Could not get alarm config");
    return;
  }

  for (auto& alarm : m_ecl_alarms) {
    // In the current version, use only first crate PV to get alarm limits
    int crate_id = 1;
    std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
    if (std::isnan(limits_info[pv_name].upper_alarm_limit)) return;
    if (std::isnan(limits_info[pv_name].upper_warning_limit)) return;
    // Looks like for integer PVs, alarms are set at value >= upper_limit
    alarm.alarm_limit   = limits_info[pv_name].upper_alarm_limit   - 1;
    alarm.warning_limit = limits_info[pv_name].upper_warning_limit - 1;

    m_mask[alarm.name].clear();
    for (int i = 0; i < MASK_SIZE; i++) {
      int cell_id = mask_info[alarm.name].value[i];
      if (cell_id == 0) break;
      m_mask[alarm.name].insert(cell_id);
    }
  }
}

std::vector< std::vector<int> > DQMHistAnalysisECLSummaryModule::updateAlarmCounts(bool update_mirabelle)
{
  std::vector< std::vector<int> > alarm_counts(
    m_ecl_alarms.size(), std::vector<int>(ECL::ECL_CRATES));

  //=== Get number of fit inconsistencies

  TH1* h_fail_crateid = findHist("ECL/fail_crateid");

  const int fit_alarm_index = getAlarmByName("bad_fit").first;
  for (int crate_id = 1; crate_id <= ECL::ECL_CRATES; crate_id++) {
    int errors_count = 0;
    if (h_fail_crateid) {
      errors_count = h_fail_crateid->GetBinContent(crate_id);
    } else {
      errors_count = 999999;
    }

    alarm_counts[fit_alarm_index][crate_id - 1] += errors_count;
  }

  // [Cell ID] -> error_bitmask
  std::map<int, int> error_bitmasks;

  //=== Get number of dead/cold/hot channels
  // cppcheck-suppress unassignedVariable
  for (auto& [cell_id, error_bitmask] : getChannelsWithOccupancyProblems()) {
    error_bitmasks[cell_id] |= error_bitmask;
  }
  //=== Get number of channels with bad_chi2
  // cppcheck-suppress unassignedVariable
  for (auto& [cell_id, error_bitmask] : getChannelsWithChi2Problems()) {
    error_bitmasks[cell_id] |= error_bitmask;
  }

  //=== Combine the information

  if (!update_mirabelle) {
    h_bad_chi2_overlay->Reset();
    h_bad_occ_overlay->Reset();
    h_bad_chi2_overlay_green->Reset();
    h_bad_occ_overlay_green->Reset();
  }

  static std::vector<std::string> indices = {"dead", "cold", "hot", "bad_chi2"};
  for (auto& index_name : indices) {
    int alarm_index = getAlarmByName(index_name).first;
    int alarm_bit   = 1 << alarm_index;
    for (auto& [cid, error_bitmask] : error_bitmasks) {
      int crate_id = m_mapper.getCrateID(cid);
      if ((error_bitmask & alarm_bit) == 0) continue;

      bool masked = (m_mask[index_name].find(cid) != m_mask[index_name].end());

      if (!masked) alarm_counts[alarm_index][crate_id - 1] += 1;

      if (update_mirabelle) continue;

      if (index_name == "bad_chi2") {
        if (!masked) h_bad_chi2_overlay->SetBinContent(cid, 1);
        else h_bad_chi2_overlay_green->SetBinContent(cid, 1);
      } else if (index_name == "dead" || index_name == "cold" || index_name == "hot") {
        if (!masked) h_bad_occ_overlay->SetBinContent(cid, 1);
        else h_bad_occ_overlay_green->SetBinContent(cid, 1);
      }
    }

    if (update_mirabelle) continue;

    if (index_name == "hot" || index_name == "bad_chi2") {
      TCanvas* current_canvas;
      TH1* main_hist;
      TH1F* overlay_hist;
      TH1F* overlay_hist_green;
      if (index_name == "hot") {
        main_hist          = findHist("ECL/cid_Thr5MeV");
        overlay_hist       = h_bad_occ_overlay;
        overlay_hist_green = h_bad_occ_overlay_green;
        current_canvas     = c_occupancy;
      } else {
        main_hist          = findHist("ECL/bad_quality");
        overlay_hist       = h_bad_chi2_overlay;
        overlay_hist_green = h_bad_chi2_overlay_green;
        current_canvas     = c_bad_chi2;
      }

      for (auto& overlay : {overlay_hist, overlay_hist_green}) {
        for (int bin_id = 1; bin_id <= ECL::ECL_TOTAL_CHANNELS; bin_id++) {
          if (overlay->GetBinContent(bin_id) == 0) continue;
          // Do not adjust bin height for dead channels
          if (main_hist->GetBinContent(bin_id) == 0) continue;
          overlay->SetBinContent(bin_id, main_hist->GetBinContent(bin_id));
        }
      }

      current_canvas->Clear();
      current_canvas->cd();
      main_hist->Draw("hist");
      overlay_hist->Draw("hist;same");
      overlay_hist_green->Draw("hist;same");
      current_canvas->Modified();
      current_canvas->Update();
      current_canvas->Draw();
    }
  }

  //== Update EPICS PVs or MiraBelle monObjs

  for (size_t alarm_idx = 0; alarm_idx < alarm_counts.size(); alarm_idx++) {
    auto& alarm = m_ecl_alarms[alarm_idx];
    std::map<std::string, int> total;
    // Convert values per crate to totals
    for (size_t crate = 0; crate < alarm_counts[alarm_idx].size(); crate++) {
      int crate_id = crate + 1;
      int value    = alarm_counts[alarm_idx][crate];

      if (!update_mirabelle) {
        std::string pv_name = (boost::format("crate%02d:%s") % crate_id % alarm.name).str();
        setEpicsPV(pv_name, value);
      }

      total["All"] += value;
      if (crate_id <= ECL::ECL_BARREL_CRATES) {
        total["Barrel"] += value;
      } else if (crate_id <= ECL::ECL_BARREL_CRATES + ECL::ECL_FWD_CRATES) {
        total["FWDEndcap"] += value;
      } else {
        total["BWDEndcap"] += value;
      }
    }
    // Export totals
    for (auto& ecl_part : {"All", "FWDEndcap", "Barrel", "BWDEndcap"}) {
      std::string pv_name = (boost::format("%s:%s") % ecl_part % alarm.name).str();
      if (update_mirabelle) {
        std::string var_name = pv_name;
        std::replace(var_name.begin(), var_name.end(), ':', '_');
        m_monObj->setVariable(var_name, total[ecl_part]);
      } else {
        setEpicsPV(pv_name, total[ecl_part]);
      }
    }
  }

  if (!update_mirabelle) updateEpicsPVs(5.0);

  return alarm_counts;
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getChannelsWithOccupancyProblems()
{
  static std::vector< std::vector<short> > neighbours(ECL::ECL_TOTAL_CHANNELS);
  if (neighbours[0].size() == 0) {
    for (int cid0 = 0; cid0 < ECL::ECL_TOTAL_CHANNELS; cid0++) {
      // [0]First is the crystal itself.
      // [1]Second is PHI neighbour, phi+1.
      // [2]Third is PHI neighbour, phi-1.
      // [3]Next one (sometimes two) are THETA neighbours in theta-1.
      // [4]Next one (sometimes two) are THETA neighbours in theta+1.
      neighbours[cid0] = m_neighbours_obj.getNeighbours(cid0 + 1);
      // Remove first element (the crystal itself)
      neighbours[cid0].erase(neighbours[cid0].begin());
    }
  }

  TH1* h_occupancy = findHist("ECL/cid_Thr5MeV");
  // TODO: Maybe set this as a module parameter
  const double max_deviation = 0.28;
  return getSuspiciousChannels(h_occupancy, m_total_events, neighbours,
                               max_deviation, true);
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getChannelsWithChi2Problems()
{
  ECL::ECLGeometryPar* geom = ECL::ECLGeometryPar::Instance();

  static std::vector< std::vector<short> > neighbours(ECL::ECL_TOTAL_CHANNELS);
  if (neighbours[0].size() == 0) {
    for (int cid_center = 1; cid_center <= ECL::ECL_TOTAL_CHANNELS; cid_center++) {
      geom->Mapping(cid_center - 1);
      const int theta_id_center  = geom->GetThetaID();
      int phi_id_center          = geom->GetPhiID();
      const int crystals_in_ring = m_neighbours_obj.getCrystalsPerRing(theta_id_center);
      phi_id_center              = phi_id_center * 144 / crystals_in_ring;
      for (int cid0 = 0; cid0 < 8736; cid0++) {
        if (cid0 == cid_center - 1) continue;
        geom->Mapping(cid0);
        int theta_id = geom->GetThetaID();
        int phi_id   = geom->GetPhiID();
        phi_id       = phi_id * 144 / m_neighbours_obj.getCrystalsPerRing(theta_id);
        if (std::abs(theta_id - theta_id_center) <= 2 &&
            std::abs(phi_id   - phi_id_center)   <= 2) {
          neighbours[cid_center - 1].push_back(cid0);
        }
      }
    }
  }

  TH1* h_bad_chi2 = findHist("ECL/bad_quality");
  // TODO: Maybe set this as a module parameter
  const double max_deviation = 2.5;
  return getSuspiciousChannels(h_bad_chi2, m_total_events, neighbours,
                               max_deviation, false);
}

std::map<int, int> DQMHistAnalysisECLSummaryModule::getSuspiciousChannels(
  TH1* hist, double total_events,
  const std::vector< std::vector<short> >& neighbours,
  double max_deviation, bool occupancy_histogram)
{
  std::map<int, int> retval;

  if (!hist) return retval;
  //== A bit hacky solution to skip incorrectly
  //   filled histograms
  if (hist->Integral() <= 0) return retval;

  //=== Extract alarm details
  // cppcheck-suppress unassignedVariable
  const auto& [dead_index, dead_alarm] = getAlarmByName("dead");
  // cppcheck-suppress unassignedVariable
  const auto& [cold_index, cold_alarm] = getAlarmByName("cold");
  // cppcheck-suppress unassignedVariable
  const auto& [hot_index,  hot_alarm ] = getAlarmByName("hot");
  // cppcheck-suppress unassignedVariable
  const auto& [chi2_index, chi2_alarm] = getAlarmByName("bad_chi2");

  double min_required_events;

  if (occupancy_histogram) {
    min_required_events = std::min({
      dead_alarm.required_statistics,
      cold_alarm.required_statistics,
      hot_alarm.required_statistics,
    });
  } else {
    min_required_events = chi2_alarm.required_statistics;
  }

  if (total_events < min_required_events) return retval;

  int dead_bit = 1 << dead_index;
  int cold_bit = 1 << cold_index;
  int hot_bit  = 1 << hot_index;
  int chi2_bit = 1 << chi2_index;

  // == Search for dead channels

  if (occupancy_histogram) {
    if (total_events >= dead_alarm.required_statistics) {
      // There should be registered signals in at least 1% of all events.
      double min_occupancy = 0.01;
      if (findCanvas("ECL/c_cid_Thr5MeV_analysis") == nullptr) {
        // The histogram is not normalized, multiply the threshold by evt count
        min_occupancy *= total_events;
      }
      for (int cid = 1; cid <= ECL::ECL_TOTAL_CHANNELS; cid++) {
        if (hist->GetBinContent(cid) > min_occupancy) continue;
        retval[cid] |= dead_bit;
      }
    }
  }

  // == Search for cold and hot channels (or channels with bad chi2)

  for (int cid = 1; cid <= ECL::ECL_TOTAL_CHANNELS; cid++) {
    double actual_value = hist->GetBinContent(cid);

    std::vector<short> neighb = neighbours[cid - 1];
    std::multiset<double> values_sorted;
    for (auto& neighbour_cid : neighb) {
      values_sorted.insert(hist->GetBinContent(neighbour_cid));
    }

    double expected_value;
    if (occupancy_histogram) {
      // Use median value:
      expected_value = *std::next(values_sorted.begin(), neighb.size() / 2);
    } else {
      // Use "upper ~75%" value (48 is the expected number of neighbours)
      expected_value = *std::next(values_sorted.begin(), 35 * neighb.size() / 48);
    }
    double deviation = std::abs((actual_value - expected_value) /
                                expected_value);

    if (!occupancy_histogram) {
      // Min occupancy for high-energy (> 1GeV) hits
      double min_occupancy = 1.51e-5;
      if (findCanvas("ECL/c_bad_quality_analysis") == nullptr) {
        // The histogram is not normalized, multiply the threshold by evt count
        min_occupancy *= total_events;
      }

      if (actual_value < min_occupancy) continue;
    }

    if (deviation < max_deviation) continue;

    if (occupancy_histogram) {
      if (actual_value < expected_value) retval[cid] |= cold_bit;
      if (actual_value > expected_value) retval[cid] |= hot_bit;
    } else {
      if (actual_value > expected_value) retval[cid] |= chi2_bit;
    }
  }

  return retval;
}

void DQMHistAnalysisECLSummaryModule::drawGrid(TH2* hist)
{
  int x_min = hist->GetXaxis()->GetXmin();
  int x_max = hist->GetXaxis()->GetXmax();
  int y_min = hist->GetYaxis()->GetXmin();
  int y_max = hist->GetYaxis()->GetXmax();
  for (int x = x_min + 1; x < x_max; x++) {
    auto l = new TLine(x, 0, x, 5);
    l->SetLineStyle(kDashed);
    l->Draw();
  }
  for (int y = y_min + 1; y < y_max; y++) {
    auto l = new TLine(1, y, ECL::ECL_CRATES + 1, y);
    l->SetLineStyle(kDashed);
    l->Draw();
  }
}

