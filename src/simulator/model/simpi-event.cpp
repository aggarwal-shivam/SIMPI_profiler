#include <vector>

#include <ns3/fatal-error.h>
#include <ns3/log.h>

#include "simpi-event.h"

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("SimpiEvent");

// ATTRIBUTE_VALUE_IMPLEMENT_WITH_NAME(std::vector<simpi_event_tagged_t>,
// SimpiEvent);
SimpiEventValue::SimpiEventValue() : m_value() {}
SimpiEventValue::SimpiEventValue(const std::vector<simpi_event_tagged_t> &value)
    : m_value(value) {}
void SimpiEventValue::Set(const std::vector<simpi_event_tagged_t> &v) {
  m_value = v;
}
std::vector<simpi_event_tagged_t> SimpiEventValue::Get(void) const {
  return m_value;
}
Ptr<AttributeValue> SimpiEventValue::Copy(void) const {
  return ns3::Create<SimpiEventValue>(*this);
}
std::string
SimpiEventValue::SerializeToString(Ptr<const AttributeChecker> checker) const {
  std::ostringstream oss;
  for (auto it = m_value.begin(); it != m_value.end(); ++it) {
    switch (it->event_type) {
    case SimpiEventType::Compute:
      oss << "compute"
          << " " << it->event.compute_event.num_instructions << "\n";
      break;
    case SimpiEventType::Recv:
      oss << "recv"
          << " " << it->event.recv_event.data_size << " "
          << it->event.recv_event.from_rank << "\n";
      break;
    case SimpiEventType::Send:
      oss << "send"
          << " " << it->event.send_event.data_size << " "
          << it->event.send_event.to_rank << "\n";
      break;
    }
  }
  return oss.str();
}
bool SimpiEventValue::DeserializeFromString(
    std::string value, Ptr<const AttributeChecker> checker) {
  std::istringstream iss;
  iss.str(value);
  while (!(iss.eof())) {
    std::string event_type;
    simpi_event_t event;
    SimpiEventType event_t;
    iss >> event_type;
    if (iss.bad() || iss.fail()) {
      goto outside_err_check;
    }
    if (event_type == "compute") {
      long long num_instructions;
      iss >> num_instructions;
      if (iss.bad() || iss.fail()) {
        goto outside_err_check;
      }
      event.compute_event = {num_instructions};
      event_t = SimpiEventType::Compute;
    } else if (event_type == "recv") {
      uint32_t data_size;
      uint16_t from_rank;
      iss >> data_size;
      if (iss.bad() || iss.fail()) {
        goto outside_err_check;
      }
      iss >> from_rank;
      if (iss.bad() || iss.fail()) {
        goto outside_err_check;
      }
      event.recv_event = {from_rank, data_size};
      event_t = SimpiEventType::Recv;
    } else if (event_type == "send") {
      uint32_t data_size;
      uint16_t to_rank;
      iss >> data_size;
      if (iss.bad() || iss.fail()) {
        goto outside_err_check;
      }
      iss >> to_rank;
      if (iss.bad() || iss.fail()) {
        goto outside_err_check;
      }
      event.send_event = {to_rank, data_size};
      event_t = SimpiEventType::Send;
    } else {
      goto outside_err_check;
    }
    simpi_event_tagged_t value = {event_t, event};
    m_value.push_back(value);
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

Ptr<const AttributeChecker> MakeSimpiEventChecker(std::string name) {
  NS_LOG_FUNCTION(name);
  struct Checker : public AttributeChecker {
    Checker(std::string name) : m_name(name) {}
    virtual bool Check(const AttributeValue &value) const {
      NS_LOG_FUNCTION(&value);
      const SimpiEventValue *v = dynamic_cast<const SimpiEventValue *>(&value);
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
      return ns3::Create<SimpiEventValue>();
    }
    virtual bool Copy(const AttributeValue &source,
                      AttributeValue &destination) const {
      NS_LOG_FUNCTION(&source << &destination);
      const SimpiEventValue *src =
          dynamic_cast<const SimpiEventValue *>(&source);
      SimpiEventValue *dst = dynamic_cast<SimpiEventValue *>(&destination);
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
