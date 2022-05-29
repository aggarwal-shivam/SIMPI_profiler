#ifndef ADDRESS_MAP_H
#define ADDRESS_MAP_H

#include <vector>

#include <ns3/attribute-helper.h>
#include <ns3/attribute.h>
#include <ns3/address.h>

namespace ns3 {

ATTRIBUTE_VALUE_DEFINE_WITH_NAME(std::vector<Address>, AddressMap);
ATTRIBUTE_ACCESSOR_DEFINE(AddressMap);

template <typename T> Ptr<const AttributeChecker> MakeAddressMapChecker(void);

} // namespace ns3

#include <ns3/type-name.h>

namespace ns3 {

namespace internal {

Ptr<const AttributeChecker> MakeAddressMapChecker(std::string name);

} // namespace internal

template <typename T> Ptr<const AttributeChecker> MakeAddressMapChecker(void) {
  return internal::MakeAddressMapChecker(TypeNameGet<T>());
}
} // namespace ns3
#endif /* SIMPI_EVENT_H */
