#include <utility>
#include <vector>

#include <ns3/bridge-module.h>
#include <ns3/csma-module.h>
#include <ns3/internet-module.h>
#include <ns3/netanim-module.h>

#include "topology-gen.h"

using namespace ns3;

void GenerateTestTopology(std::vector<Ptr<Node>> &nodes,
                          std::vector<Address> &addresses) {
  for (size_t i = 0; i < nodes.size(); i++) {
    nodes[i] = CreateObject<Node>();
  }

  Ptr<Node> bridge = CreateObject<Node>();

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("1000Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

  NodeContainer lan;
  for (size_t i = 0; i < nodes.size(); i++) {
    lan.Add(nodes[i]);
  }

  std::vector<Ptr<NetDevice>> lanDevices(nodes.size());
  NetDeviceContainer bridgeDevices;

  for (size_t i = 0; i < nodes.size(); i++) {
    NetDeviceContainer link =
        csma.Install(NodeContainer(lan.Get(i), bridge));
    lanDevices[i] = (link.Get(0));
    bridgeDevices.Add(link.Get(1));
  }

  NetDeviceContainer lanDevicesContainer;
  for (size_t i = 0; i < lanDevices.size(); i++) {
    lanDevicesContainer.Add(lanDevices[i]);
  }

  BridgeHelper bridgeHelper;
  bridgeHelper.Install(bridge, bridgeDevices);

  NodeContainer routerNodes;
  for (size_t i = 0; i < nodes.size(); i++) {
    routerNodes.Add(nodes[i]);
  }
  InternetStackHelper internet;
  internet.Install(routerNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("172.27.19.0", "255.255.255.0");
  Ipv4InterfaceContainer assigned = ipv4.Assign(lanDevicesContainer);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  for (size_t i = 0; i < addresses.size(); i++) {
    addresses[i] = assigned.GetAddress(i);
  }

  //
  // Configure tracing of all enqueue, dequeue, and NetDevice receive events.
  // Trace output will be sent to the file "csma-bridge-one-hop.tr"
  //
  AsciiTraceHelper ascii;
  csma.EnableAsciiAll(ascii.CreateFileStream("csma-bridge-one-hop.tr"));

  //
  // Also configure some tcpdump traces; each interface will be traced.
  // The output files will be named:
  //     csma-bridge-one-hop-<nodeId>-<interfaceId>.pcap
  // and can be read by the "tcpdump -r" command (use "-tt" option to
  // display timestamps correctly)
  //
  csma.EnablePcapAll("csma-bridge-one-hop", false);
}

void GenerateTopology(std::vector<Ptr<Node>> &nodes,
                      std::vector<Address> &addresses) {
  for (size_t i = 0; i < 30; i++) {
    nodes[i] = CreateObject<Node>();
  }

  Ptr<Node> bridge1 = CreateObject<Node>();
  Ptr<Node> bridge2 = CreateObject<Node>();

  CsmaHelper csma;
  csma.SetChannelAttribute("DataRate", StringValue("1000Mbps"));
  csma.SetChannelAttribute("Delay", TimeValue(MilliSeconds(1)));

  NodeContainer lan1;
  for (size_t i = 0; i < 16; i++) {
    if (i == 12)
      continue;
    lan1.Add(nodes[i]);
  }

  NodeContainer lan2;
  lan2.Add(nodes[12]);
  for (size_t i = 16; i < 30; i++) {
    lan2.Add(nodes[i]);
  }

  std::vector<Ptr<NetDevice>> lanDevices(30);
  NetDeviceContainer lan1Devices;
  NetDeviceContainer bridge1Devices;
  NetDeviceContainer lan2Devices;
  NetDeviceContainer bridge2Devices;

  for (size_t i = 0; i < 15; i++) {
    NetDeviceContainer link1 =
        csma.Install(NodeContainer(lan1.Get(i), bridge1));
    lan1Devices.Add(link1.Get(0));
    bridge1Devices.Add(link1.Get(1));
    if (i < 12) {
      lanDevices[i] = link1.Get(0);
    } else {
      lanDevices[i + 1] = link1.Get(0);
    }

    NetDeviceContainer link2 =
        csma.Install(NodeContainer(lan2.Get(i), bridge2));
    lan2Devices.Add(link2.Get(0));
    bridge2Devices.Add(link2.Get(1));
    if (i == 0) {
      lanDevices[12] = link2.Get(0);
    } else {
      lanDevices[i + 15] = link2.Get(0);
    }
  }

  NetDeviceContainer linkBridges =
      csma.Install(NodeContainer(bridge1, bridge2));
  bridge1Devices.Add(linkBridges.Get(0));
  bridge2Devices.Add(linkBridges.Get(1));

  NetDeviceContainer lanDevicesContainer;
  for (size_t i = 0; i < 30; i++) {
    lanDevicesContainer.Add(lanDevices[i]);
  }

  BridgeHelper bridge;
  bridge.Install(bridge1, bridge1Devices);
  bridge.Install(bridge2, bridge2Devices);

  NodeContainer routerNodes;
  for (size_t i = 0; i < 30; i++) {
    routerNodes.Add(nodes[i]);
  }
  InternetStackHelper internet;
  internet.Install(routerNodes);

  Ipv4AddressHelper ipv4;
  ipv4.SetBase("172.27.19.0", "255.255.255.0");
  Ipv4InterfaceContainer assigned = ipv4.Assign(lanDevicesContainer);

  Ipv4GlobalRoutingHelper::PopulateRoutingTables();

  for (size_t i = 0; i < 30; i++) {
    addresses[i] = assigned.GetAddress(i);
  }
}

void SetupAnimation(const std::vector<Ptr<Node>> &nodes) {
  AnimationInterface anim("animation.xml");
  anim.EnablePacketMetadata(true);

  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i], 100 * i, 0);
  }
  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i + 4], 500 + 100 * i, 0);
  }

  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i + 8], 100 * i, 100);
  }
  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i + 12], 500 + 100 * i, 100);
  }

  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i + 16], 100 * i, 300);
  }
  for (size_t i = 0; i < 4; i++) {
    anim.SetConstantPosition(nodes[i + 20], 500 + 100 * i, 300);
  }

  for (size_t i = 0; i < 3; i++) {
    anim.SetConstantPosition(nodes[i + 24], 100 * i, 400);
  }
  for (size_t i = 0; i < 3; i++) {
    anim.SetConstantPosition(nodes[i + 27], 500 + 100 * i, 400);
  }
}
