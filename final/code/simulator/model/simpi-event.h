#ifndef SIMPI_EVENT_H
#define SIMPI_EVENT_H

#include <vector>

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>

namespace ns3 {

struct simpi_send_t {
  uint16_t to_rank;
  uint32_t data_size;
};

struct simpi_recv_t {
  uint16_t from_rank;
  uint32_t data_size;
};

struct simpi_compute_t {
  long long num_instructions;
};

enum SimpiEventType { Compute, Recv, Send };

union simpi_event_t {
  simpi_compute_t compute_event;
  simpi_recv_t recv_event;
  simpi_send_t send_event;
};

struct simpi_event_tagged_t {
  SimpiEventType event_type;
  simpi_event_t event;
};

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(std::vector<simpi_event_tagged_t>, SimpiEvent);
ATTRIBUTE_ACCESSOR_DEFINE(SimpiEvent);

template <typename T> Ptr<const AttributeChecker> MakeSimpiEventChecker(void);

} // namespace ns3

#include <ns3/type-name.h>

namespace ns3 {

namespace internal {

Ptr<const AttributeChecker> MakeSimpiEventChecker(std::string name);

} // namespace internal

template <typename T> Ptr<const AttributeChecker> MakeSimpiEventChecker(void) {
  return internal::MakeSimpiEventChecker(TypeNameGet<T>());
}
} // namespace ns3
#endif /* SIMPI_EVENT_H */
