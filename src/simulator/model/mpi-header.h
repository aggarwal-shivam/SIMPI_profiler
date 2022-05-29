#ifndef MPI_HEADER_H
#define MPI_HEADER_H

#include <ns3/header.h>

#define MPI_HEADER_SIZE 20

namespace ns3 {
  class Packet;

  class MPIHeader : public Header {
  public:
    MPIHeader();

    static TypeId GetTypeId();
    virtual TypeId GetInstanceTypeId() const;
    virtual uint32_t GetSerializedSize(void) const;
    virtual void Serialize(Buffer::Iterator start) const;
    virtual uint32_t Deserialize(Buffer::Iterator start);
    virtual void Print(std::ostream &os) const;
  };
}
#endif
