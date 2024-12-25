#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/aodv-module.h"
#include "ns3/applications-module.h"
#include "ns3/flow-monitor-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("AODVExample");

int 
main (int argc, char *argv[])
{
  // 设置默认参数
  uint32_t numNodes = 8;
  double simulationTime = 20.0; // 秒

  // 解析命令行参数
  CommandLine cmd;
  cmd.Parse (argc, argv);

  // 创建节点
  NodeContainer nodes;
  nodes.Create (numNodes);

  // 设置节点移动模型（这里使用静态位置）
  MobilityHelper mobility;
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();
  
  // 设定节点位置，示例为8个节点排列在正方形网格中
  double distance = 50.0; // 米
  for (uint32_t i = 0; i < numNodes; ++i)
    {
      double x = (i % 4) * distance;
      double y = (i / 4) * distance;
      positionAlloc->Add (Vector (x, y, 0.0));
    }

  mobility.SetPositionAllocator (positionAlloc);
  mobility.Install (nodes);

 WifiHelper wifi;
    wifi.SetStandard (WIFI_PHY_STANDARD_80211b); // 修正后的枚举值
    
    // 正确初始化 YansWifiPhyHelper 并设置信道
    YansWifiChannelHelper wifiChannel = YansWifiChannelHelper::Default ();
    YansWifiPhyHelper wifiPhy = YansWifiPhyHelper::Default ();
    wifiPhy.SetChannel (wifiChannel.Create ());

    WifiMacHelper wifiMac;
    wifiMac.SetType ("ns3::AdhocWifiMac");

    NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, nodes);

  // 安装互联网协议栈并使用 AODV 路由协议
  AodvHelper aodv;
  InternetStackHelper internet;
  internet.SetRoutingHelper (aodv);
  internet.Install (nodes);

  // 分配 IP 地址
  Ipv4AddressHelper ipv4;
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer interfaces = ipv4.Assign (devices);

  // 安装应用程序
  // 在节点 0 上安装接收端
  uint16_t port = 9; // Discard port
  Address sinkAddress (InetSocketAddress (interfaces.GetAddress (0), port));
  PacketSinkHelper packetSinkHelper ("ns3::UdpSocketFactory", 
                                     InetSocketAddress (Ipv4Address::GetAny (), port));
  ApplicationContainer sinkApps = packetSinkHelper.Install (nodes.Get (0));
  sinkApps.Start (Seconds (0.0));
  sinkApps.Stop (Seconds (simulationTime));

  // 在节点 7 上安装发送端
  OnOffHelper onOff ("ns3::UdpSocketFactory", sinkAddress);
  onOff.SetConstantRate (DataRate ("1Mbps"));
  onOff.SetAttribute ("PacketSize", UintegerValue (1024));

  ApplicationContainer sourceApps = onOff.Install (nodes.Get (7));
  sourceApps.Start (Seconds (1.0));
  sourceApps.Stop (Seconds (simulationTime));

  // 启用 FlowMonitor
  FlowMonitorHelper flowmonHelper;
  Ptr<FlowMonitor> flowmon = flowmonHelper.InstallAll ();

  // 运行模拟
  Simulator::Stop (Seconds (simulationTime));
  Simulator::Run ();

  // 获取 FlowMonitor 数据
  flowmon->CheckForLostPackets ();
  Ptr<Ipv4FlowClassifier> classifier = DynamicCast<Ipv4FlowClassifier> (flowmonHelper.GetClassifier ());
  std::map<FlowId, FlowMonitor::FlowStats> stats = flowmon->GetFlowStats ();

  for (auto const &flow : stats)
    {
      Ipv4FlowClassifier::FiveTuple t = classifier->FindFlow (flow.first);
      // 只关注节点 7 到节点 0 的流量
      if (t.sourceAddress == interfaces.GetAddress (7) && t.destinationAddress == interfaces.GetAddress (0))
        {
          std::cout << "Flow ID: " << flow.first << std::endl;
          std::cout << "Src Addr: " << t.sourceAddress << " Src Port: " << t.sourcePort << std::endl;
          std::cout << "Dst Addr: " << t.destinationAddress << " Dst Port: " << t.destinationPort << std::endl;
          std::cout << "Tx Packets: " << flow.second.txPackets << std::endl;
          std::cout << "Rx Packets: " << flow.second.rxPackets << std::endl;
          std::cout << "Lost Packets: " << flow.second.lostPackets << std::endl;
          double lossRate = (double)flow.second.lostPackets / flow.second.txPackets * 100.0;
          std::cout << "Packet Loss Rate: " << lossRate << " %" << std::endl;

          // 吞吐量计算（比特/秒）
          double throughput = (flow.second.rxBytes * 8.0) / simulationTime / 1000 / 1000; // Mbps
          std::cout << "Throughput: " << throughput << " Mbps" << std::endl;

          // 端到端时延计算（秒）
          double delay = flow.second.delaySum.GetSeconds () / flow.second.rxPackets;
          std::cout << "Average End-to-End Delay: " << delay << " s" << std::endl;
        }
    }

  flowmon->SerializeToXmlFile ("aodv-flowmon.xml", true, true);

  Simulator::Destroy ();
  return 0;
}
