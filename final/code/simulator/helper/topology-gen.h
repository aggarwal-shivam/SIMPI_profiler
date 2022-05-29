#include <utility>
#include <vector>

#include <ns3/core-module.h>
#include <ns3/network-module.h>

void GenerateTopology(std::vector<ns3::Ptr<ns3::Node>> &,
                      std::vector<ns3::Address> &);

void GenerateTestTopology(std::vector<ns3::Ptr<ns3::Node>> &,
                          std::vector<ns3::Address> &);

void SetupAnimation(const std::vector<ns3::Ptr<ns3::Node>> &);
