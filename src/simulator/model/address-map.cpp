#include <vector>

#include <ns3/address.h>
#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include "address-map.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("AddressMap");

AddressMapValue::AddressMapValue() : m_value() {}

AddressMapValue::AddressMapValue(const std::vector<Address> &value)
    : m_value(value) {}

void AddressMapValue::Set(const std::vector<Address> &v) {
  m_value = v;
}

std::vector<Address> AddressMapValue::Get(void) const {
  return m_value;
}

Ptr<AttributeValue> AddressMapValue::Copy(void) const {
  return ns3::Create<AddressMapValue>(*this);
}

std::string
AddressMapValue::SerializeToString(Ptr<const AttributeChecker> checker) const {
  std::ostringstream oss;
  for (auto it = m_value.begin(); it != m_value.end(); ++it) {
    oss << (*it);
  }
  return oss.str();
}
bool AddressMapValue::DeserializeFromString(
    std::string value, Ptr<const AttributeChecker> checker) {
  std::istringstream iss;
  iss.str(value);
  while (!(iss.eof())) {
    Address address;
    iss >> address;
    if (iss.bad() || iss.fail()) {
      goto outside_err_check;
    }
    m_value.push_back(address);
  }
outside_err_check:
  do {
    if (!(iss.eof())) {
      std::cerr << "aborted. cond=\""
                << "!(iss.eof ())"
                << "\", ";
      do {
        std::cerr << "msg=\""
                  << "Attribute value "
                  << "\"" << value << "\""
                  << " is not properly formatted"
                  << "\", ";
        do {
          std::cerr << "file="
                    << "model/simpi-event.cpp"
                    << ", line=" << 12 << std::endl;
          ::ns3::FatalImpl::FlushStreams();
          if (true)
            std::terminate();
        } while (false);
      } while (false);
    }
  } while (false);
  return !iss.bad() && !iss.fail();
};

namespace internal {

Ptr<const AttributeChecker> MakeAddressMapChecker(std::string name) {
  NS_LOG_FUNCTION(name);
  struct Checker : public AttributeChecker {
    Checker(std::string name) : m_name(name) {}
    virtual bool Check(const AttributeValue &value) const {
      NS_LOG_FUNCTION(&value);
      const AddressMapValue *v = dynamic_cast<const AddressMapValue *>(&value);
      return v != 0;
    }

    virtual std::string GetValueTypeName(void) const {
      NS_LOG_FUNCTION_NOARGS();
      return "ns3::SimpiEventValue";
    }
    virtual bool HasUnderlyingTypeInformation(void) const {
      NS_LOG_FUNCTION_NOARGS();
      return true;
    }
    virtual std::string GetUnderlyingTypeInformation(void) const {
      NS_LOG_FUNCTION_NOARGS();
      return m_name;
    }
    virtual Ptr<AttributeValue> Create(void) const {
      NS_LOG_FUNCTION_NOARGS();
      return ns3::Create<AddressMapValue>();
    }
    virtual bool Copy(const AttributeValue &source,
                      AttributeValue &destination) const {
      NS_LOG_FUNCTION(&source << &destination);
      const AddressMapValue *src =
          dynamic_cast<const AddressMapValue *>(&source);
      AddressMapValue *dst = dynamic_cast<AddressMapValue *>(&destination);
      if (src == 0 || dst == 0) {
        return false;
      }
      *dst = *src;
      return true;
    }
    std::string m_name;
  } *checker = new Checker(name);
  return Ptr<const AttributeChecker>(checker, false);
}

} // namespace internal

} // namespace ns3
