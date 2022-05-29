#include <vector>

#include "../model/simpi-event.h"

using namespace ns3;

std::vector<std::vector<simpi_event_tagged_t>> Parse(uint16_t num_processes,
                                                     std::string logName);
