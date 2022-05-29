#include <ns3/log.h>

#include "mpi-header.h"

NS_LOG_COMPONENT_DEFINE("MPIHeader");

namespace ns3 {
NS_OBJECT_ENSURE_REGISTERED(MPIHeader);

MPIHeader::MPIHeader() : Header() { NS_LOG_FUNCTION(this); }

TypeId MPIHeader::GetTypeId() {
  static TypeId tid =
      TypeId("ns3::MPIHeader").SetParent<Header>().AddConstructor<MPIHeader>();
  return tid;
}

TypeId MPIHeader::GetInstanceTypeId() const { return GetTypeId(); }

uint32_t MPIHeader::GetSerializedSize() const { return MPI_HEADER_SIZE; }

void MPIHeader::Serialize(Buffer::Iterator start) const {
  NS_LOG_FUNCTION(this << &start);
  start.WriteU8(0x2a, MPI_HEADER_SIZE);
}

uint32_t MPIHeader::Deserialize(Buffer::Iterator start) {
  NS_LOG_FUNCTION(this << &start);
  uint32_t bytesRead = 0;

  for (uint32_t i = 0; i < MPI_HEADER_SIZE; i++) {
    uint8_t byte = start.ReadU8();
    if (byte != 0x2a) return 0;
    bytesRead++;
  }
  return bytesRead;
}

void MPIHeader::Print(std::ostream &os) const {
  NS_LOG_FUNCTION(this << &os);
  os << "(MPIHEADER)";
}

} // namespace ns3
