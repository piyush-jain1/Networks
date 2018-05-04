/* Group 36
Piyush Jain, Shivam Gupta, Hrushikesh Turkhade
150101046, 150101068, 150123044
Networks Lab Assignment 4
*/

/* Packet size justification

RTS Size

Taken From: https://mrncciew.com/2014/10/26/cwap-802-11-ctrl-rtscts/

The RTS frame contains five fields, which are:
Frame Control
Duration
RA (Receiver Address)
TA (Transmitter Address)
FCS

rts_size = 2 + 2 + 6 + 6 + 4 = 20

********************************************

CTS Size

The CTS frame contains four fields, which are:
Frame Control
Duration
RA (Receiver Address)
FCS

cts_size = 2 + 2 + 6 + 4 = 14

**************************
http://archive.oreilly.com/wireless/2003/08/08/wireless_throughput.html

rts_cts_ack_size = 14

tcp_ack_size = 74

*/

#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/flow-monitor-module.h"
#include <iostream>


NS_LOG_COMPONENT_DEFINE ("group36");

using namespace ns3;

Ptr<PacketSink> sink;                         
uint64_t lastTotalRx = 0; 
uint32_t tcpSegmentSize = 1000; 
double simulationTime = 50.0;                           

void calculateAnalytics()
{
  FILE* fp = fopen("server.pcap","r");
  char line[1024];
  uint32_t rtsctsack_size = 14;
  uint32_t tcpack_size = 74;
  uint32_t cts_size = 14;
  uint32_t rts_size = 20;
  uint32_t data_size = tcpSegmentSize+34;

  std::string rtsctsack = "Acknowledgment";
  std::string tcpack = "ack";
  std::string rts = "Request-To-Send";
  std::string cts = "Clear-To-Send";
  std::string seq = "seq";

  uint32_t rtsctsack_count = 0;
  uint32_t tcpack_count = 0;
  uint32_t rts_count = 0;
  uint32_t cts_count = 0;
  uint32_t data_count = 0;
  while(fgets(line, 1024, fp))
  {
  	std::string s1(line);
  	if(s1.find(rtsctsack) != std::string::npos)	rtsctsack_count++;
  	else if(s1.find(tcpack) != std::string::npos and s1.find(seq) == std::string::npos)	tcpack_count++;
  	else if(s1.find(rts) != std::string::npos)	rts_count++;
  	else if(s1.find(cts) != std::string::npos)	cts_count++;
  	else if(s1.find(seq) != std::string::npos)	data_count++;
  }
  double rts_bw = (rts_size*rts_count*8/simulationTime)/1000;
	double cts_bw = (cts_size*cts_count*8/simulationTime)/1000;
	double rtsctsack_bw = (rtsctsack_size*rts_count*8/simulationTime)/1000;
	double tcpack_bw = (tcpack_size*tcpack_count*8/simulationTime)/1000;
	double data_bw = (data_size*tcpack_count*8/simulationTime)/1000;

	printf("rts_bw : %f\n",rts_bw );
	printf("cts_bw : %f\n",cts_bw );
	printf("rtsctsack_bw : %f\n",rtsctsack_bw );
	printf("tcpack_bw : %f\n",tcpack_bw );
	printf("data_bw : %f\n",data_bw );		
}

uint32_t MacTxDropCount = 0, PhyTxDropCount = 0, PhyRxDropCount = 0;

void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}

void run(uint32_t rtsCtsThreshold)
{
   /* Setting the RTS/CTS threshold */
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", UintegerValue(rtsCtsThreshold));
  Config::SetDefault ("ns3::WifiRemoteStationManager::FragmentationThreshold", StringValue ("2000"));

  /* Set TCP segment size */
  Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (tcpSegmentSize));

  WifiHelper wifi;
  wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);

  /* Set up Legacy Channel */
  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::RandomPropagationLossModel");

  /* Setup Physical Layer */
  YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
  wifiPhy.SetChannel (wifiChannel.Create ());
  wifiPhy.SetErrorRateModel ("ns3::YansErrorRateModel");
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                    "DataMode", StringValue ("HtMcs7"),
                    "ControlMode", StringValue ("HtMcs0"));

  /* Create Nodes */
  NodeContainer nodes;
  nodes.Create (3);
  Ptr<Node> serverNode = nodes.Get (1);
  NodeContainer clientNodes;
  clientNodes = NodeContainer(nodes.Get(0),nodes.Get(2));

  WifiMacHelper wifiMac;
  wifiMac.SetType ("ns3::AdhocWifiMac");

  NetDeviceContainer serverDevice;
  serverDevice = wifi.Install (wifiPhy, wifiMac, serverNode);

  NetDeviceContainer clientDevice;
  clientDevice = wifi.Install (wifiPhy, wifiMac, clientNodes);

  /* Mobility model */
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (250.0, 0.0, 0.0));
  positionAlloc->Add (Vector (500.0, 0.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (nodes);


  /* Internet stack */
  InternetStackHelper stack;
  stack.Install (nodes);

  Ipv4AddressHelper address;
  address.SetBase ("10.0.0.0", "255.255.255.0");
  Ipv4InterfaceContainer apInterface;
  apInterface = address.Assign (serverDevice);
  Ipv4InterfaceContainer staInterface;
  staInterface = address.Assign (clientDevice);

   /* Install TCP Receiver on the access point */
  PacketSinkHelper serverUtil ("ns3::TcpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), 9));
  ApplicationContainer serverApp = serverUtil.Install (serverNode);
  sink = StaticCast<PacketSink> (serverApp.Get (0));

  /* Install TCP/UDP Transmitter on the station */
  OnOffHelper onOffHelper ("ns3::TcpSocketFactory", (InetSocketAddress (apInterface.GetAddress (0), 9)));
  onOffHelper.SetAttribute ("PacketSize", UintegerValue (tcpSegmentSize));
  onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
  onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
  onOffHelper.SetAttribute ("DataRate", DataRateValue (DataRate ("300kbps")));
  ApplicationContainer clientApp = onOffHelper.Install (clientNodes);

  /* Start Applications */
  double time_to_start = rand()%5 + 1;
  serverApp.Start (Seconds (time_to_start));
  clientApp.Start (Seconds (time_to_start));

  /* PCAP tracing */
  wifiPhy.EnablePcap ("Server", serverDevice);
  wifiPhy.EnablePcap ("Station", clientDevice);

  /* FLow Monitor */
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll ();

  // Trace Collisions
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  /* Start Simulation */
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();
  Simulator::Destroy ();

  monitor->SerializeToXmlFile("group36.xml", true, true);

  monitor->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmon.GetClassifier ());
  FlowMonitor::FlowStatsContainer stats = monitor->GetFlowStats ();
  for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator i = stats.begin (); i != stats.end (); i++)
  {
  Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (i->first);
  std::cout << "Flow " << i->first - 2 << " (" << t.sourceAddress << " -> " << t.destinationAddress << ")\n";
  std::cout << "  Tx Packets: " << i->second.txPackets << "\n";
  std::cout << "  Tx Bytes:   " << i->second.txBytes << "\n";
  std::cout << "  TxOffered:  " << i->second.txBytes * 8.0 / 49.0 / 1000 / 1000  << " Mbps\n";
  std::cout << "  Rx Packets: " << i->second.rxPackets << "\n";
  std::cout << "  Rx Bytes:   " << i->second.rxBytes << "\n";
  std::cout << "  Throughput: " << i->second.rxBytes * 8.0 / 49.0 / 1000 / 1000  << " Mbps\n";
  }

  double averageThroughput = ((sink->GetTotalRx () * 8) / (1e6  * simulationTime));
  std::cout << "\nAverage throughtput: " << averageThroughput << " Mbit/s" << std::endl;
}

int main (int argc, char *argv[])
{
                     
  uint32_t thresholds[] = {500,1000,1500,2000};
  for(int i=0;i<4; i++)
  {
    uint32_t rtsCtsThreshold = thresholds[i];
    std::cout<<"Results with RTS Threshold : "<<rtsCtsThreshold<<std::endl;
    run(rtsCtsThreshold);
    system("tcpdump -nn -tt -r Server-1-0.pcap > server.pcap");
    calculateAnalytics();
    std::cout<<"Collisions : "<<MacTxDropCount+PhyTxDropCount+PhyRxDropCount<<std::endl;
    std::cout<<"---------------------------------------------------------------"<<std::endl;
  }
  return 0;
}
