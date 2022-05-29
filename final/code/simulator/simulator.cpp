#include <iostream>
#include <sstream>

#include <ns3/applications-module.h>
#include <ns3/bridge-module.h>
#include <ns3/core-module.h>
#include <ns3/csma-module.h>
#include <ns3/global-route-manager.h>
#include <ns3/internet-module.h>
#include <ns3/network-module.h>

#include "helper/mpi-node-helper.h"
#include "helper/parser.h"
#include "helper/topology-gen.h"
#include "model/mpi-node.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("MPISimulator");

int main(int argc, char *argv[]) {

  NS_LOG_INFO("Parsing CommandLine arguments.");

  CommandLine cmd;
  std::stringstream ss;
  ss << "usage: " << argv[0]
     << " --file hostfile --number number_of_processes --logs agg_log_file";
  cmd.Usage(ss.str());
  std::string filename = "";
  std::string logFilename = "";
  uint16_t number = 0;
  cmd.AddValue("file", "Hostfile from which to read hosts", filename);
  cmd.AddValue("number", "Hostfile from which to read hosts", number);
  cmd.AddValue("logs", "File containing simpi logs", logFilename);
  cmd.Parse(argc, argv);

  if (filename == "") {
    std::cerr << "Filename must be provided" << std::endl;
    return 1;
  }

  if (logFilename == "") {
    std::cerr << "LogFilename must be provided" << std::endl;
    return 1;
  }

  if (number == 0) {
    std::cerr << "Number of processes must be provided" << std::endl;
    return 1;
  }

  std::vector<std::vector<simpi_event_tagged_t>> events =
      Parse(number, logFilename);

  /* Parse hostfile for required number of nodes */
  std::ifstream hostfile(filename);
  uint16_t number_nodes = number / MPI_NODE_PPN;
  if (number_nodes * MPI_NODE_PPN < number) {
    number_nodes++;
  }
  std::vector<size_t> node_indices(number_nodes);
  for (size_t i = 0; i < number_nodes; i++) {
    std::string host;
    hostfile >> host;
    int n;
    if (host.compare(0, 5, "csews") == 0) {
      n = std::stoi(host.substr(5));
    } else if (host.compare(0, 10, "172.27.19.") == 0) {
      n = std::stoi(host.substr(10));
    } else {
      std::cerr << "Can't recognize node in hostfile\n";
      exit(1);
    }
    if (n > 30) {
      std::cerr << "Can't recognize node in hostfile\n";
      exit(1);
    }
    node_indices[i] = n - 1;
  }
  hostfile.close();

  /* Build All Nodes */
  NS_LOG_INFO("Building Nodes and Topology.");
  std::vector<Ptr<Node>> nodesAll;
  std::vector<Address> addressesAll;
#ifdef TEST_SIM
  nodesAll.resize(2);
  addressesAll.resize(2);
  GenerateTestTopology(nodesAll, addressesAll);
#else
  nodesAll.resize(30);
  addressesAll.resize(30);
  GenerateTopology(nodesAll, addressesAll);
#endif


  std::vector<Address> addresses(number_nodes);
  for (size_t i = 0; i < number_nodes; i++) {
    addresses[i] = addressesAll[node_indices[i]];
  }

  MPINodeHelper nodeHelper(addresses);

  for (size_t i = 0; i < number; i++) {
    size_t index = i / MPI_NODE_PPN;
    nodeHelper.SetRankEvents(i, events[i]);
    ApplicationContainer app = nodeHelper.Install(nodesAll[node_indices[index]]);
    app.Start(Seconds(0.0));
  }

  /* Simulation. */
  /* Pcap output. */
  /* Stop the simulation after x seconds. */
  // uint32_t stopTime = 300;
  // Simulator::Stop(Seconds(stopTime));

  /* Start and clean simulation. */
  Simulator::Run();
  Simulator::Destroy();
}
