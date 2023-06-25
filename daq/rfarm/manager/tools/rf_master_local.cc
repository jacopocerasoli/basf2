/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/
#include "daq/rfarm/manager/RFMaster.h"
#include "daq/rfarm/manager/RFNSM.h"

using namespace std;
using namespace Belle2;

int main(int argc, char** argv)
{
  if (argc < 2) return 1;

  RFConf conf(argv[1]);

  RFMaster* master = new RFMaster(argv[1]);
  // Creation of event server instance. evs contains the instance
  //  RFOutputServer& ots = RFOutputServer::Create ( argv[1] );

  RFNSM nsm(conf.getconf("master", "nodename"), master);
  nsm.AllocMem(conf.getconf("system", "nsmdata"));
  master->SetNodeInfo(nsm.GetNodeInfo());
  master->Hook_Message_Handlers();

  master->monitor_loop();

}
