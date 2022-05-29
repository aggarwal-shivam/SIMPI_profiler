#include <ns3/names.h>
#include <ns3/uinteger.h>

#include "../model/address-map.h"
#include "../model/mpi-node.h"
#include "mpi-node-helper.h"

namespace ns3 {

MPINodeHelper::MPINodeHelper(uint16_t rank,
                             std::vector<simpi_event_tagged_t> events,
                             std::vector<Address> addresses) {
  m_factory.SetTypeId(MPINode::GetTypeId());
  SetAttribute("Rank", UintegerValue(rank));
  SetAttribute("Events", SimpiEventValue(events));
  SetAttribute("Addresses", AddressMapValue(addresses));
}

MPINodeHelper::MPINodeHelper(std::vector<Address> addresses) {
  m_factory.SetTypeId(MPINode::GetTypeId());
  SetAttribute("Addresses", AddressMapValue(addresses));
}

void MPINodeHelper::SetRankEvents(uint16_t rank,
                                  std::vector<simpi_event_tagged_t> events) {
  SetAttribute("Rank", UintegerValue(rank));
  SetAttribute("Events", SimpiEventValue(events));
}

void MPINodeHelper::SetAttribute(std::string name,
                                 const AttributeValue &value) {
  m_factory.Set(name, value);
}

ApplicationContainer MPINodeHelper::Install(Ptr<Node> node) const {
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MPINodeHelper::Install(std::string nodeName) const {
  Ptr<Node> node = Names::Find<Node>(nodeName);
  return ApplicationContainer(InstallPriv(node));
}

ApplicationContainer MPINodeHelper::Install(NodeContainer c) const {
  ApplicationContainer apps;
  for (NodeContainer::Iterator i = c.Begin(); i != c.End(); ++i) {
    apps.Add(InstallPriv(*i));
  }

  return apps;
}

Ptr<Application> MPINodeHelper::InstallPriv(Ptr<Node> node) const {
  Ptr<Application> app = m_factory.Create<MPINode>();
  node->AddApplication(app);

  return app;
}

simpi_event_tagged_t MPINodeHelper::ComputeEvent(long long num_instructions) {
  simpi_event_t event;
  event.compute_event = {num_instructions};
  return {SimpiEventType::Compute, event};
}

simpi_event_tagged_t MPINodeHelper::SendEvent(uint16_t rank, uint32_t size) {
  simpi_event_t event;
  event.send_event = {rank, size};
  return {SimpiEventType::Send, event};
}

simpi_event_tagged_t MPINodeHelper::RecvEvent(uint16_t rank, uint32_t size) {
  simpi_event_t event;
  event.recv_event = {rank, size};
  return {SimpiEventType::Recv, event};
}

} // namespace ns3
