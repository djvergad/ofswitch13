/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014 University of Campinas (Unicamp)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Luciano Chaves <luciano@lrc.ic.unicamp.br>
 */

#include <iostream>
#include <fstream>

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/applications-module.h"
#include "ns3/log.h"

NS_LOG_COMPONENT_DEFINE ("OFSwitch13Example");

using namespace ns3;

#ifdef NS3_OFSWITCH13
#include "ns3/ofswitch13-module.h"

int 
main (int argc, char *argv[])
{
  bool verbose = true;

  CommandLine cmd;
  cmd.AddValue ("verbose", "Tell application to log if true", verbose);

  cmd.Parse (argc,argv);

  if (verbose)
    {
      LogComponentEnable ("OFSwitch13Example", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Helper", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13NetDevice", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Interface", LOG_LEVEL_ALL);
      LogComponentEnable ("OFSwitch13Controller", LOG_LEVEL_ALL);
    }
    
  // Enabling Checksum computations
  GlobalValue::Bind ("ChecksumEnabled", BooleanValue (true));
  // ns3::Packet::EnablePrinting ();

  // Create the terminal nodes
  NodeContainer terminals;
  terminals.Create (2);

  // Create the switches node
  NodeContainer switches;
  switches.Create (1);
  Ptr<Node> switchNode = switches.Get (0);

  // Create the controller node
  NodeContainer controller;
  controller.Create (1);
  Ptr<Node> controllerNode = controller.Get (0);

  // Connect the terminals to the switch using csma links
  CsmaHelper csmaHelper;
  csmaHelper.SetChannelAttribute ("DataRate", DataRateValue (DataRate ("100Mbps")));
  csmaHelper.SetChannelAttribute ("Delay", TimeValue (MilliSeconds (2)));
  NetDeviceContainer terminalDevices;
  NetDeviceContainer switchDevices;
  for (size_t i = 0; i < terminals.GetN (); i++)
    {
      NetDeviceContainer link = csmaHelper.Install (NodeContainer (terminals.Get (i), switches));
      terminalDevices.Add (link.Get (0));
      switchDevices.Add (link.Get (1));
    }

  // Install the OFSwitch13NetDevice onto the switch
  NetDeviceContainer of13Device;
  Ptr<OFSwitch13Helper> ofHelper = Create<OFSwitch13Helper> ();
  of13Device = ofHelper->InstallSwitch (switchNode, switchDevices);

  // Install the controller app (creating links between controller and switches)
  Ptr<OFSwitch13Controller> controlApp = ofHelper->InstallController (controllerNode);

  // Some OpenFlow flow-mod commands for tests
  Ptr<OFSwitch13NetDevice> ofswitchNetDev = of13Device.Get (0)->GetObject<OFSwitch13NetDevice> ();
  Simulator::Schedule (Seconds (1), &OFSwitch13Controller::SendFlowModMsg, controlApp, ofswitchNetDev, 
      "cmd=add,table=0,prio=0 apply:output=ctrl");
  Simulator::Schedule (Seconds (1), &OFSwitch13Controller::SendFlowModMsg, controlApp, ofswitchNetDev, 
      "cmd=add,table=0 in_port=1 apply:output=2");
  Simulator::Schedule (Seconds (1), &OFSwitch13Controller::SendFlowModMsg, controlApp, ofswitchNetDev, 
      "cmd=add,table=0 in_port=2 apply:output=1");

  // Installing the tcp/ip stack onto terminals
  InternetStackHelper internet;
  internet.Install (terminals);

  // Set IPv4 terminal address
  Ipv4AddressHelper ipv4switches;
  Ipv4InterfaceContainer internetIpIfaces;
  ipv4switches.SetBase ("10.1.1.0", "255.255.255.0");
  internetIpIfaces = ipv4switches.Assign (terminalDevices);

  // Create a ping application from terminal 0 to 1 
  Ipv4Address destAddr = internetIpIfaces.GetAddress (1);
  V4PingHelper ping = V4PingHelper (destAddr);
  ApplicationContainer apps = ping.Install (terminals.Get (0));
  apps.Start (Seconds (1.0));

  // Enable pcap traces
  ofHelper->EnableOpenFlowPcap ();
  csmaHelper.EnablePcap ("ofswitch-p0", switchDevices.Get (0));
  csmaHelper.EnablePcap ("ofswitch-p1", switchDevices.Get (1));

  controlApp->SetStopTime (Seconds (10));

  // Run the simulation
  Simulator::Stop (Seconds (30));
  Simulator::Run ();
  Simulator::Destroy ();
}

#else

int
main (int argc, char *argv[])
{
  NS_LOG_UNCOND ("OpenFlow 1.3 not enabled! Aborting...");
}

#endif // NS3_OFSWITCH13
