/**************************************************************************
 * basf2 (Belle II Analysis Software Framework)                           *
 * Author: The Belle II Collaboration                                     *
 *                                                                        *
 * See git log for contributors and copyright holders.                    *
 * This file is licensed under LGPL-3.0, see LICENSE.md.                  *
 **************************************************************************/

#define TRG_SHORT_NAMES

#include <iostream>
#include <fstream>
#include <string>
#include "trg/trg/Utilities.h"

using namespace std;
using namespace Belle2;

#define DEBUG_LEVEL   0
#define NAME          "TRGUT2Template"
#define VERSION       "version 0.00"
#define NOT_CONNECTED 99999

int vhdlVersion0(ofstream&);
int ucfVersion0(ofstream&);

int
main(int argc, const char* argv[])
{

  cout << NAME << " ... " << VERSION << endl;
  const string tab = "    ";

  //...Check arguments...
  if (argc > 2) {
    cout << NAME << " !!! two many arguments" << endl
         << tab << " 1 : template version" << endl;
    return -1;
  }

  //...Template version...
  string version = "latest";
  if (argc == 2)
    version = argv[1];
  if (version == "latest")
    version = "0";

  //...Version check...
  if (version != "0") {
    cout << NAME << " !!! no such a version" << endl
         << tab << " 0 : 1st version" << endl;
    return -1;
  }

  //...Date...
  string ts0 = TRGUtil::dateStringF();
  string ts1 = TRGUtil::dateString();

  //...1st argument...
  const string vname = "TRGUT2Template_" + version + "_" + ts0 + ".vhd";
  const string uname = "TRGUT2Template_" + version + "_" + ts0 + ".ucf";

  //...Open vhdl file...
  ofstream vout(vname.c_str(), ios::out);
  if (vout.fail()) {
    cout << NAME << " !!! can not open file" << endl
         << "    " << vname << endl;
    return -2;
  }

  //...Main...
  cout << NAME << " ... generating VHDL file" << endl;
  vout << "-- Generated by " << NAME << " " << VERSION << endl;
  vout << "-- Template version " << version << endl;
  vout << "-- " << ts1 << endl;
  vout << "--" << endl;
  vhdlVersion0(vout);
  vout.close();

  //...Open ucf file...
  ofstream uout(uname.c_str(), ios::out);
  if (uout.fail()) {
    cout << NAME << " !!! can not open file" << endl
         << "    " << uname << endl;
    return -3;
  }

  //...Main...
  cout << NAME << " ... generating UCF file" << endl;
  uout << "-- Generated by " << NAME << " " << VERSION << endl;
  uout << "-- Template version " << version << endl;
  uout << "-- " << ts1 << endl;
  uout << "--" << endl;
  ucfVersion0(uout);
  uout.close();

  //...Termination...
  cout << NAME << " ... terminated" << endl;
  return 0;
}

int
vhdlVersion0(ofstream&)
{
  return 0;
}

int
ucfVersion0(ofstream& uout)
{
  uout << "###############################################################################" << endl;
  uout << "###############################################################################" << endl;
  uout << "#...GTP CLOCK Locations..." << endl;
  uout << "NET REFCLK_P  LOC=V4;" << endl;
  uout << "NET REFCLK_N  LOC=V3;" << endl;
  uout << "" << endl;
  uout << "#...Clock..." << endl;
  uout << "NET CLK42M    LOC=AL27;" << endl;
  uout << "" << endl;
  uout << "#...LEDs..." << endl;
  uout << "NET LED_STAT_X<0> LOC=N25;" << endl;
  uout << "NET LED_STAT_X<1> LOC=P25;" << endl;
  uout << "NET LED_STAT_X<2> LOC=P18;" << endl;
  uout << "NET LED_STAT_X<3> LOC=P17;" << endl;
  uout << "NET LED_PWR_X     LOC=P26;" << endl;
  uout << "NET LED_CLK_X     LOC=N26;" << endl;
  uout << "" << endl;
  uout << "#...NIM..." << endl;
  uout << "NET TRG_IN   LOC=R8;" << endl;
  uout << "NET TRG_OUT  LOC=F6;" << endl;
  uout << "NET BUSY_IN  LOC=F7;" << endl;
  uout << "NET BUSY_OUT LOC=R7;" << endl;
  uout << "NET CLK_IN   LOC=AN15;" << endl;
  uout << "NET CLK_OUT  LOC=N9;" << endl;
  uout << "" << endl;
  uout << "#...VME bus..." << endl;
  uout << "NET LWORD_X LOC=H41;" << endl;
  uout << "NET DS0_X LOC=G42;" << endl;
  uout << "NET DS1_X LOC=F41;" << endl;
  uout << "NET WRITE_X LOC=F42;" << endl;
  uout << "NET IACK_X LOC=R39;" << endl;
  uout << "NET IACKIN_X LOC=J41;" << endl;
  uout << "NET BERR_X LOC=N38;" << endl;
  uout << "NET IACKOUT_X LOC=P37;" << endl;
  uout << "NET DIRL LOC=T42;" << endl;
  uout << "NET DIRH LOC=U42;" << endl;
  uout << "NET OEBL LOC=U41;" << endl;
  uout << "NET OEBH LOC=V41;" << endl;
  uout << "NET DTACK_X LOC=R37;" << endl;
  uout << "NET IRQ_X LOC=P38;" << endl;
  uout << "NET AM<0> LOC=V40;" << endl;
  uout << "NET AM<1> LOC=W41;" << endl;
  uout << "NET AM<2> LOC=W42;" << endl;
  uout << "NET AM<3> LOC=Y42;" << endl;
  uout << "NET AM<4> LOC=AA42;" << endl;
  uout << "NET AM<5> LOC=AA41;" << endl;
  uout << "NET AS_X LOC=G41;" << endl;
  uout << "" << endl;
  uout << "NET VMEA<1> LOC=AL40;" << endl;
  uout << "NET VMEA<2> LOC=AL41;" << endl;
  uout << "NET VMEA<3> LOC=AK42;" << endl;
  uout << "NET VMEA<4> LOC=AL42;" << endl;
  uout << "NET VMEA<5> LOC=AM42;" << endl;
  uout << "NET VMEA<6> LOC=AM41;" << endl;
  uout << "NET VMEA<7> LOC=AN41;" << endl;
  uout << "NET VMEA<8> LOC=AP42;" << endl;
  uout << "NET VMEA<9> LOC=AP41;" << endl;
  uout << "NET VMEA<10> LOC=AR42;" << endl;
  uout << "NET VMEA<11> LOC=AT42;" << endl;
  uout << "NET VMEA<12> LOC=AT41;" << endl;
  uout << "NET VMEA<13> LOC=AU41;" << endl;
  uout << "NET VMEA<14> LOC=AU42;" << endl;
  uout << "NET VMEA<15> LOC=AV41;" << endl;
  uout << "NET VMEA<16> LOC=J42;" << endl;
  uout << "NET VMEA<17> LOC=K42;" << endl;
  uout << "NET VMEA<18> LOC=L40;" << endl;
  uout << "NET VMEA<19> LOC=L41;" << endl;
  uout << "NET VMEA<20> LOC=L42;" << endl;
  uout << "NET VMEA<21> LOC=M41;" << endl;
  uout << "NET VMEA<22> LOC=M42;" << endl;
  uout << "NET VMEA<23> LOC=N41;" << endl;
  uout << "NET VMEA<24> LOC=H36;" << endl;
  uout << "NET VMEA<25> LOC=G37;" << endl;
  uout << "NET VMEA<26> LOC=F36;" << endl;
  uout << "NET VMEA<27> LOC=G36;" << endl;
  uout << "NET VMEA<28> LOC=F37;" << endl;
  uout << "NET VMEA<29> LOC=E37;" << endl;
  uout << "NET VMEA<30> LOC=E38;" << endl;
  uout << "NET VMEA<31> LOC=D37;" << endl;
  uout << "" << endl;
  uout << "NET VMED<0> LOC=N40;" << endl;
  uout << "NET VMED<1> LOC=P40;" << endl;
  uout << "NET VMED<2> LOC=W40;" << endl;
  uout << "NET VMED<3> LOC=Y40;" << endl;
  uout << "NET VMED<4> LOC=AA40;" << endl;
  uout << "NET VMED<5> LOC=AA39;" << endl;
  uout << "NET VMED<6> LOC=Y39;" << endl;
  uout << "NET VMED<7> LOC=Y38;" << endl;
  uout << "NET VMED<8> LOC=Y37;" << endl;
  uout << "NET VMED<9> LOC=AA37;" << endl;
  uout << "NET VMED<10> LOC=R42;" << endl;
  uout << "NET VMED<11> LOC=P42;" << endl;
  uout << "NET VMED<12> LOC=P41;" << endl;
  uout << "NET VMED<13> LOC=R40;" << endl;
  uout << "NET VMED<14> LOC=T40;" << endl;
  uout << "NET VMED<15> LOC=T41;" << endl;
  uout << "NET VMED<16> LOC=H38;" << endl;
  uout << "NET VMED<17> LOC=H39;" << endl;
  uout << "NET VMED<18> LOC=G38;" << endl;
  uout << "NET VMED<19> LOC=G39;" << endl;
  uout << "NET VMED<20> LOC=F39;" << endl;
  uout << "NET VMED<21> LOC=F40;" << endl;
  uout << "NET VMED<22> LOC=E39;" << endl;
  uout << "NET VMED<23> LOC=E40;" << endl;
  uout << "NET VMED<24> LOC=L36;" << endl;
  uout << "NET VMED<25> LOC=L35;" << endl;
  uout << "NET VMED<26> LOC=K35;" << endl;
  uout << "NET VMED<27> LOC=J35;" << endl;
  uout << "NET VMED<28> LOC=H35;" << endl;
  uout << "NET VMED<29> LOC=J36;" << endl;
  uout << "NET VMED<30> LOC=K37;" << endl;
  uout << "NET VMED<31> LOC=J37;" << endl;
  uout << "" << endl;
  uout << "#...Logic Analyzer (LA1)..." << endl;
  uout << "NET LA1<0> LOC=P7;" << endl;
  uout << "NET LA1<1> LOC=P8;" << endl;
  uout << "NET LA1<2> LOC=D7;" << endl;
  uout << "NET LA1<3> LOC=V9;" << endl;
  uout << "NET LA1<4> LOC=V10;" << endl;
  uout << "NET LA1<5> LOC=F9;" << endl;
  uout << "NET LA1<6> LOC=G9;" << endl;
  uout << "NET LA1<7> LOC=G7;" << endl;
  uout << "NET LA1<8> LOC=G8;" << endl;
  uout << "NET LA1<9> LOC=U8;" << endl;
  uout << "NET LA1<10> LOC=U9;" << endl;
  uout << "NET LA1<11> LOC=H8;" << endl;
  uout << "NET LA1<12> LOC=H9;" << endl;
  uout << "NET LA1<13> LOC=T10;" << endl;
  uout << "NET LA1<14> LOC=T11;" << endl;
  uout << "NET LA1<15> LOC=J8;" << endl;
  uout << "NET LA1<16> LOC=J7;" << endl;
  uout << "NET LA1<17> LOC=U11;" << endl;
  uout << "NET LA1<18> LOC=V11;" << endl;
  uout << "NET LA1<19> LOC=K8;" << endl;
  uout << "NET LA1<20> LOC=K9;" << endl;
  uout << "NET LA1<21> LOC=K7;" << endl;
  uout << "NET LA1<22> LOC=L7;" << endl;
  uout << "NET LA1<23> LOC=M7;" << endl;
  uout << "NET LA1<24> LOC=J12;" << endl;
  uout << "NET LA1<25> LOC=H11;" << endl;
  uout << "NET LA1<26> LOC=G12;" << endl;
  uout << "NET LA1<27> LOC=G11;" << endl;
  uout << "NET LA1<28> LOC=F12;" << endl;
  uout << "NET LA1<29> LOC=F11;" << endl;
  uout << "NET LA1<30> LOC=F10;" << endl;
  uout << "NET LA1<31> LOC=K14;" << endl;
  uout << "NET LA1_CLK1P LOC=J16;" << endl;
  uout << "NET LA1_CLK1N LOC=J15;" << endl;
  uout << "NET LA1_CLK2P LOC=L14;" << endl;
  uout << "NET LA1_CLK2N LOC=K15;" << endl;
  uout << "" << endl;
  uout << "#...Logic Analyzer (LA2)..." << endl;
  uout << "NET LA2<0> LOC=K13;" << endl;
  uout << "NET LA2<1> LOC=K12;" << endl;
  uout << "NET LA2<2> LOC=J11;" << endl;
  uout << "NET LA2<3> LOC=J13;" << endl;
  uout << "NET LA2<4> LOC=H13;" << endl;
  uout << "NET LA2<5> LOC=H10;" << endl;
  uout << "NET LA2<6> LOC=J10;" << endl;
  uout << "NET LA2<7> LOC=H14;" << endl;
  uout << "NET LA2<8> LOC=H15;" << endl;
  uout << "NET LA2<9> LOC=K10;" << endl;
  uout << "NET LA2<10> LOC=L10;" << endl;
  uout << "NET LA2<11> LOC=L12;" << endl;
  uout << "NET LA2<12> LOC=L11;" << endl;
  uout << "NET LA2<13> LOC=G13;" << endl;
  uout << "NET LA2<14> LOC=G14;" << endl;
  uout << "NET LA2<15> LOC=M11;" << endl;
  uout << "NET LA2<16> LOC=M12;" << endl;
  uout << "NET LA2<17> LOC=F14;" << endl;
  uout << "NET LA2<18> LOC=E13;" << endl;
  uout << "NET LA2<19> LOC=N11;" << endl;
  uout << "NET LA2<20> LOC=P12;" << endl;
  uout << "NET LA2<21> LOC=E12;" << endl;
  uout << "NET LA2<22> LOC=D12;" << endl;
  uout << "NET LA2<23> LOC=P11;" << endl;
  uout << "NET LA2<24> LOC=L24;" << endl;
  uout << "NET LA2<25> LOC=M24;" << endl;
  uout << "NET LA2<26> LOC=E18;" << endl;
  uout << "NET LA2<27> LOC=E17;" << endl;
  uout << "NET LA2<28> LOC=K24;" << endl;
  uout << "NET LA2<29> LOC=L25;" << endl;
  uout << "NET LA2<30> LOC=F16;" << endl;
  uout << "NET LA2<31> LOC=F17;" << endl;
  uout << "NET LA2_CLK1P LOC=J17;" << endl;
  uout << "NET LA2_CLK1N LOC=K17;" << endl;
  uout << "NET LA2_CLK2P LOC=L17;" << endl;
  uout << "NET LA2_CLK2N LOC=M17;" << endl;
  uout << "" << endl;
  uout << "#...Logic Analyzer (LA3)..." << endl;
  uout << "NET LA3<0> LOC=K25;" << endl;
  uout << "NET LA3<1> LOC=J25;" << endl;
  uout << "NET LA3<2> LOC=G16;" << endl;
  uout << "NET LA3<3> LOC=H16;" << endl;
  uout << "NET LA3<4> LOC=H26;" << endl;
  uout << "NET LA3<5> LOC=J26;" << endl;
  uout << "NET LA3<6> LOC=G18;" << endl;
  uout << "NET LA3<7> LOC=G17;" << endl;
  uout << "NET LA3<8> LOC=J28;" << endl;
  uout << "NET LA3<9> LOC=J27;" << endl;
  uout << "NET LA3<10> LOC=J18;" << endl;
  uout << "NET LA3<11> LOC=H18;" << endl;
  uout << "NET LA3<12> LOC=K18;" << endl;
  uout << "NET LA3<13> LOC=K19;" << endl;
  uout << "NET LA3<14> LOC=K27;" << endl;
  uout << "NET LA3<15> LOC=L26;" << endl;
  uout << "NET LA3<16> LOC=N20;" << endl;
  uout << "NET LA3<17> LOC=P20;" << endl;
  uout << "NET LA3<18> LOC=G27;" << endl;
  uout << "NET LA3<19> LOC=F27;" << endl;
  uout << "NET LA3<20> LOC=M19;" << endl;
  uout << "NET LA3<21> LOC=N19;" << endl;
  uout << "NET LA3<22> LOC=G28;" << endl;
  uout << "NET LA3<23> LOC=H28;" << endl;
  uout << "NET LA3<24> LOC=M18;" << endl;
  uout << "NET LA3<25> LOC=N18;" << endl;
  uout << "NET LA3<26> LOC=G29;" << endl;
  uout << "NET LA3<27> LOC=F29;" << endl;
  uout << "NET LA3<28> LOC=L20;" << endl;
  uout << "NET LA3<29> LOC=L19;" << endl;
  uout << "NET LA3<30> LOC=H29;" << endl;
  uout << "NET LA3<31> LOC=H30;" << endl;
  uout << "NET LA3_CLK1P LOC=M26;" << endl;
  uout << "NET LA3_CLK1N LOC=L27;" << endl;
  uout << "NET LA3_CLK2P LOC=L29;" << endl;
  uout << "NET LA3_CLK2N LOC=K28;" << endl;
  uout << "" << endl;
  uout << "#...FPGAIO..." << endl;
  uout << "NET FPGAIO<0>  LOC=N33;" << endl;
  uout << "NET FPGAIO<1>  LOC=N34;" << endl;
  uout << "NET FPGAIO<2>  LOC=M34;" << endl;
  uout << "NET FPGAIO<3>  LOC=M33;" << endl;
  uout << "NET FPGAIO<4>  LOC=M32;" << endl;
  uout << "NET FPGAIO<5>  LOC=M31;" << endl;
  uout << "NET FPGAIO<6>  LOC=N31;" << endl;
  uout << "NET FPGAIO<7>  LOC=P31;" << endl;
  uout << "NET FPGAIO<8>  LOC=H34;" << endl;
  uout << "NET FPGAIO<9>  LOC=G34;" << endl;
  uout << "NET FPGAIO<10> LOC=G33;" << endl;
  uout << "NET FPGAIO<11> LOC=H33;" << endl;
  uout << "NET FPGAIO<12> LOC=G32;" << endl;
  uout << "NET FPGAIO<13> LOC=G31;" << endl;
  uout << "NET FPGAIO<14> LOC=H31;" << endl;
  uout << "NET FPGAIO<15> LOC=J31;" << endl;
  uout << "NET FPGAIO<16> LOC=F35;" << endl;
  uout << "NET FPGAIO<17> LOC=E35;" << endl;
  uout << "NET FPGAIO<18> LOC=E34;" << endl;
  uout << "NET FPGAIO<19> LOC=F34;" << endl;
  uout << "NET FPGAIO<20> LOC=F31;" << endl;
  uout << "NET FPGAIO<21> LOC=F32;" << endl;
  uout << "NET FPGAIO<22> LOC=E32;" << endl;
  uout << "NET FPGAIO<23> LOC=E33;" << endl;
  uout << "NET FPGAIO<24> LOC=L34;" << endl;
  uout << "NET FPGAIO<25> LOC=K34;" << endl;
  uout << "NET FPGAIO<26> LOC=K33;" << endl;
  uout << "NET FPGAIO<27> LOC=J33;" << endl;
  uout << "NET FPGAIO<28> LOC=K32;" << endl;
  uout << "NET FPGAIO<29> LOC=J32;" << endl;
  uout << "NET FPGAIO<30> LOC=L32;" << endl;
  uout << "NET FPGAIO<31> LOC=L31;" << endl;
  uout << "NET FPGAIO<32> LOC=P33;" << endl;
  uout << "NET FPGAIO<33> LOC=P32;" << endl;
  uout << "NET FPGAIO<34> LOC=R33;" << endl;
  uout << "NET FPGAIO<35> LOC=R32;" << endl;
  return 0;
}
