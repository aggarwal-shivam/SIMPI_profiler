#ifndef MPI_NODE_HELPER_H
#define MPI_NODE_HELPER_H

#include <stdint.h>

#include <ns3/application-container.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/node-container.h>
#include <ns3/object-factory.h>

#include "../model/simpi-event.h"

namespace ns3 {

class MPINodeHelper {
public:
  MPINodeHelper(uint16_t rank, std::vector<simpi_event_tagged_t> events,
                std::vector<Address> addresses);

  MPINodeHelper(std::vector<Address> addresses);

  void SetRankEvents(uint16_t rank, std::vector<simpi_event_tagged_t> events);

  void SetAttribute(std::string name, const AttributeValue &value);

  ApplicationContainer Install(Ptr<Node> node) const;

  ApplicationContainer Install(std::string nodeName) const;

  ApplicationContainer Install(NodeContainer c) const;

  static simpi_event_tagged_t ComputeEvent(long long);
  static simpi_event_tagged_t SendEvent(uint16_t to_rank, uint32_t size);
  static simpi_event_tagged_t RecvEvent(uint16_t from_rank, uint32_t size);

private:
  Ptr<Application> InstallPriv(Ptr<Node> node) const;

  ObjectFactory m_factory; //!< Object factory.
};

} // namespace ns3

#endif /* MPI_NODE_HELPER_H */
