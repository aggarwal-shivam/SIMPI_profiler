#include <ns3/address-utils.h>
#include <ns3/double.h>
#include <ns3/inet-socket-address.h>
#include <ns3/inet6-socket-address.h>
#include <ns3/ipv4-address.h>
#include <ns3/ipv6-address.h>
#include <ns3/log.h>
#include <ns3/nstime.h>
#include <ns3/packet.h>
#include <ns3/simulator.h>
#include <ns3/socket-factory.h>
#include <ns3/socket.h>
#include <ns3/tcp-socket-factory.h>
#include <ns3/tcp-socket.h>
#include <ns3/uinteger.h>

#include "address-map.h"
#include "mpi-header.h"
#include "mpi-node.h"

const uint16_t listen_ports[MPI_NODE_PPN] = {1, 3, 5, 7, 9, 11, 13, 15};

namespace ns3 {

NS_LOG_COMPONENT_DEFINE("MPINodeApplication");

NS_OBJECT_ENSURE_REGISTERED(MPINode);

TypeId MPINode::GetTypeId(void) {
  static TypeId tid =
      TypeId("ns3::MPINode")
          .SetParent<Application>()
          .SetGroupName("Applications")
          .AddConstructor<MPINode>()
          .AddAttribute("Rank", "Rank of current application process on node.",
                        UintegerValue(0),
                        MakeUintegerAccessor(&MPINode::m_rank),
                        MakeUintegerChecker<uint16_t>())
          .AddAttribute(
              "Events", "The various events being simulated by this node.",
              SimpiEventValue(),
              MakeSimpiEventAccessor(&MPINode::m_simpi_events),
              MakeSimpiEventChecker<std::vector<simpi_event_tagged_t>>())
          .AddAttribute("Addresses", "Addresses to the various different nodes",
                        AddressMapValue(),
                        MakeAddressMapAccessor(&MPINode::m_addresses),
                        MakeAddressMapChecker<std::vector<Address>>())
          .AddTraceSource("Tx", "A new packet is created and is send",
                          MakeTraceSourceAccessor(&MPINode::m_txTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource("Rx", "A packet has been received",
                          MakeTraceSourceAccessor(&MPINode::m_rxTrace),
                          "ns3::Packet::TracedCallback")
          .AddTraceSource(
              "TxWithAddresses", "A new packet is created and is sent",
              MakeTraceSourceAccessor(&MPINode::m_txTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback")
          .AddTraceSource(
              "RxWithAddresses", "A packet has been received",
              MakeTraceSourceAccessor(&MPINode::m_rxTraceWithAddresses),
              "ns3::Packet::TwoAddressTracedCallback");
  return tid;
}

MPINode::MPINode()
    : m_listen_socket(0), m_accepted_socket(0), m_recv_buffer_size(0),
      m_send_socket(0), m_send_buffer_size(0), m_total_send_size(0),
      m_current_step_no(0) {
  NS_LOG_FUNCTION(this << m_rank);
}

MPINode::~MPINode() { NS_LOG_FUNCTION(this << m_rank); }

void MPINode::DoDispose(void) {
  NS_LOG_FUNCTION(this << m_rank);
  m_listen_socket = 0;
  m_accepted_socket = 0;

  // chain up
  Application::DoDispose();
}

void MPINode::StartApplication(void) {
  NS_LOG_FUNCTION(this << m_rank);
  Simulator::ScheduleNow(&MPINode::ProcessCurrentStep, this);
}

void MPINode::ProcessCurrentStep(void) {
  NS_ASSERT_MSG(m_current_step_no <= m_simpi_events.size(),
                "Current step can't be greater");
  NS_ASSERT(m_accepted_socket == 0);
  NS_ASSERT(m_send_socket == 0);
  NS_ASSERT(m_listen_socket == 0);

  NS_LOG_FUNCTION(this << m_rank << " step " << m_current_step_no << " of "
                       << m_simpi_events.size());

  // Completed Simulation
  if (m_current_step_no == m_simpi_events.size()) {
    NS_LOG_INFO("Completed Simulation for " << m_rank << ".");
    StopApplication();
    return;
  }

  simpi_event_tagged_t current = m_simpi_events[m_current_step_no];
  if (current.event_type == SimpiEventType::Compute) {
    double delay =
        (double)current.event.compute_event.num_instructions / MPI_NODE_CPU_IPS;
    m_current_step_no++;
    NS_LOG_INFO("Computing for a delay of: " << delay << ".");
    Simulator::Schedule(Time(Seconds(delay)), &MPINode::ProcessCurrentStep,
                        this);
  } else if (current.event_type == SimpiEventType::Recv) {
    Simulator::ScheduleNow(&MPINode::StartListening, this);
  } else if (current.event_type == SimpiEventType::Send) {
    Simulator::ScheduleNow(&MPINode::StartSending, this);
  } else {
    NS_ASSERT_MSG(false, "There can't be two successive compute events");
  }
}

void MPINode::StartListening(void) {
  NS_LOG_FUNCTION(this << m_rank);
  NS_ASSERT(m_listen_socket == 0);

  uint16_t currentNode = m_rank / MPI_NODE_PPN;
  uint16_t targetNode =
      m_simpi_events[m_current_step_no].event.recv_event.from_rank /
      MPI_NODE_PPN;

  m_recv_from_local = currentNode == targetNode;

  int ret;

  m_listen_socket =
      Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
  m_listen_socket->SetAttribute("SegmentSize",
                                UintegerValue(MPI_NODE_LINK_MTU));

  Address m_localAddress = m_addresses[m_rank / MPI_NODE_PPN];
  const Ipv4Address ipv4 = Ipv4Address::ConvertFrom(m_localAddress);
  const InetSocketAddress inetSocket =
      InetSocketAddress(ipv4, listen_ports[m_rank % MPI_NODE_PPN]);
  NS_LOG_INFO(this << " Binding on " << ipv4 << " port "
                   << listen_ports[m_rank % MPI_NODE_PPN] << " / " << inetSocket
                   << ".");
  ret = m_listen_socket->Bind(inetSocket);
  NS_LOG_DEBUG(this << " Bind() return value= " << ret
                    << " GetErrNo= " << m_listen_socket->GetErrno() << ".");

  ret = m_listen_socket->Listen();
  NS_LOG_DEBUG(this << " Listen () return value= " << ret
                    << " GetErrNo= " << m_listen_socket->GetErrno() << ".");
  m_listen_socket->ShutdownSend();
  NS_UNUSED(ret);
  NS_ASSERT_MSG(m_listen_socket != 0, "Failed creating socket.");

  m_listen_socket->SetRecvCallback(MakeCallback(&MPINode::HandleRead, this));
  m_listen_socket->SetAcceptCallback(
      MakeCallback(&MPINode::HandleRequest, this),
      MakeCallback(&MPINode::HandleAccept, this));
  m_listen_socket->SetCloseCallbacks(
      MakeCallback(&MPINode::HandlePeerClose, this),
      MakeCallback(&MPINode::HandlePeerError, this));
}

void MPINode::StopListening(void) {
  NS_LOG_FUNCTION(this << m_rank);

  m_listen_socket->SetAcceptCallback(
      MakeNullCallback<bool, Ptr<Socket>, const Address &>(),
      MakeNullCallback<void, Ptr<Socket>, const Address &>());
  m_listen_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
  m_listen_socket->Close();
  m_listen_socket = 0;

  m_accepted_socket->Close();
  m_accepted_socket = 0;

  m_recv_buffer_size = 0;

  m_current_step_no++;
  if (m_recv_from_local) {
    Simulator::Schedule(Time(MicroSeconds(MPI_COPY_DELAY_US)),
                        &MPINode::ProcessCurrentStep, this);
  } else {
    Simulator::ScheduleNow(&MPINode::ProcessCurrentStep, this);
  }
}

void MPINode::StopApplication(void) {
  NS_LOG_FUNCTION(this << m_rank);
  NS_ASSERT(m_listen_socket == 0);
  NS_ASSERT(m_accepted_socket == 0);
  NS_ASSERT(m_send_socket == 0);
}

void MPINode::HandleRead(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << m_rank << socket);

  Ptr<Packet> packet;
  Address from;
  Address localAddress;

  while ((packet = socket->RecvFrom(from))) {
    if (packet->GetSize() == 0) {
      break;
    }

    if (InetSocketAddress::IsMatchingType(from)) {
      NS_LOG_INFO("At time "
                  << Simulator::Now().GetSeconds() << "s server received "
                  << packet->GetSize() << " bytes from "
                  << InetSocketAddress::ConvertFrom(from).GetIpv4() << " port "
                  << InetSocketAddress::ConvertFrom(from).GetPort());
    } else if (Inet6SocketAddress::IsMatchingType(from)) {
      NS_LOG_INFO("At time "
                  << Simulator::Now().GetSeconds() << "s server received "
                  << packet->GetSize() << " bytes from "
                  << Inet6SocketAddress::ConvertFrom(from).GetIpv6() << " port "
                  << Inet6SocketAddress::ConvertFrom(from).GetPort());
    }

    m_rxTrace(packet);
    m_rxTraceWithAddresses(packet, from, localAddress);

    MPIHeader header;
    packet->RemoveHeader(header);
    m_recv_buffer_size += packet->GetSize();
  }
  NS_LOG_INFO("received total of " << m_recv_buffer_size << " after read");
}
void MPINode::HandlePeerClose(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << m_rank << socket);
  bool is_correctly_sized =
      (m_recv_buffer_size ==
       m_simpi_events[m_current_step_no].event.recv_event.data_size) ||
      (m_recv_from_local && m_recv_buffer_size == 1);

  NS_ASSERT_MSG(is_correctly_sized, "expected "
                                        << m_simpi_events[m_current_step_no]
                                               .event.recv_event.data_size
                                        << " got " << m_recv_buffer_size);
  m_listen_socket->SetCloseCallbacks(MakeNullCallback<void, Ptr<Socket>>(),
                                     MakeNullCallback<void, Ptr<Socket>>());
  StopListening();
};
void MPINode::HandlePeerError(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << m_rank << socket);
};
void MPINode::HandleAccept(Ptr<Socket> s, const Address &from) {
  NS_LOG_FUNCTION(this << m_rank << s << from);
  s->SetRecvCallback(MakeCallback(&MPINode::HandleRead, this));
  m_accepted_socket = s;
  /*
   * A typical connection is established after receiving an empty (i.e., no
   * data) TCP packet with ACK flag. The actual data will follow in a separate
   * packet after that and will be received by ReceivedDataCallback().
   *
   * However, that empty ACK packet might get lost. In this case, we may
   * receive the first data packet right here already, because it also counts
   * as a new connection. The statement below attempts to fetch the data from
   * that packet, if any.
   */
  HandleRead(s);
}
bool MPINode::HandleRequest(Ptr<Socket> s, const Address &from) {
  NS_LOG_FUNCTION(this << m_rank << s
                       << InetSocketAddress::ConvertFrom(from).GetIpv4());
  simpi_recv_t event = m_simpi_events[m_current_step_no].event.recv_event;
  Address targetAddress = m_addresses[event.from_rank / MPI_NODE_PPN];
  return targetAddress == InetSocketAddress::ConvertFrom(from).GetIpv4() &&
         m_accepted_socket == 0;
}

void MPINode::StartSending(void) {
  NS_LOG_FUNCTION(this << m_rank);
  NS_ASSERT(m_send_socket == 0);

  simpi_send_t event = m_simpi_events[m_current_step_no].event.send_event;

  uint16_t currentNode = m_rank / MPI_NODE_PPN;
  uint16_t targetNode = event.to_rank / MPI_NODE_PPN;
  m_send_to_local = currentNode == targetNode;

  Address remoteAddress = m_addresses[event.to_rank / MPI_NODE_PPN];
  uint16_t remotePort = listen_ports[event.to_rank % MPI_NODE_PPN];
  int ret;

  m_send_connected = false;
  m_send_socket =
      Socket::CreateSocket(GetNode(), TcpSocketFactory::GetTypeId());
  m_send_socket->SetAttribute("SegmentSize", UintegerValue(MPI_NODE_LINK_MTU));
  m_send_socket->SetAttribute("ConnTimeout", TimeValue(MilliSeconds(100)));
  m_send_socket->SetAttribute("ConnCount", UintegerValue(100));
  m_send_socket->SetAttribute("MaxSegLifetime", DoubleValue(0.02));

  if (Ipv4Address::IsMatchingType(remoteAddress)) {
    ret = m_send_socket->Bind();
    NS_LOG_DEBUG(this << " Bind() return value= " << ret
                      << " GetErrNo= " << m_send_socket->GetErrno() << ".");

    Ipv4Address ipv4 = Ipv4Address::ConvertFrom(remoteAddress);
    InetSocketAddress inetSocket = InetSocketAddress(ipv4, remotePort);
    NS_LOG_INFO(this << " Connecting to " << ipv4 << " port " << remotePort
                     << " / " << inetSocket << ".");
    ret = m_send_socket->Connect(inetSocket);
    NS_LOG_DEBUG(this << " Connect() return value= " << ret
                      << " GetErrNo= " << m_send_socket->GetErrno() << ".");
  } else if (Ipv6Address::IsMatchingType(remoteAddress)) {
    ret = m_send_socket->Bind6();
    NS_LOG_DEBUG(this << " Bind6() return value= " << ret
                      << " GetErrNo= " << m_send_socket->GetErrno() << ".");

    Ipv6Address ipv6 = Ipv6Address::ConvertFrom(remoteAddress);
    Inet6SocketAddress inet6Socket = Inet6SocketAddress(ipv6, remotePort);
    NS_LOG_INFO(this << " connecting to " << ipv6 << " port " << remotePort
                     << " / " << inet6Socket << ".");
    ret = m_send_socket->Connect(inet6Socket);
    NS_LOG_DEBUG(this << " Connect() return value= " << ret
                      << " GetErrNo= " << m_send_socket->GetErrno() << ".");
  }
  NS_UNUSED(ret);
  NS_ASSERT(m_send_socket != 0);

  m_send_socket->ShutdownRecv();
  m_send_socket->SetConnectCallback(
      MakeCallback(&MPINode::ConnectionSucceeded, this),
      MakeCallback(&MPINode::ConnectionFailed, this));
  m_send_socket->SetSendCallback(MakeCallback(&MPINode::HandleSend, this));
}

void MPINode::StopSending() {
  NS_LOG_FUNCTION(this << m_rank);
  NS_ASSERT(m_send_buffer_size == 0);

  m_send_socket->SetSendCallback(
      MakeNullCallback<void, Ptr<Socket>, uint32_t>());

  m_send_socket->Close();
  m_send_socket = 0;

  m_send_buffer_size = 0;
  m_total_send_size = 0;

  m_current_step_no++;
  Simulator::ScheduleNow(&MPINode::ProcessCurrentStep, this);
}

void MPINode::ConnectionSucceeded(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << m_rank << socket);

  NS_ASSERT(m_simpi_events[m_current_step_no].event_type ==
            SimpiEventType::Send);

  m_send_connected = true;
  m_total_send_size =
      m_send_to_local
          ? 1
          : m_simpi_events[m_current_step_no].event.send_event.data_size;
  m_send_buffer_size = m_total_send_size;

  SendData();
}

void MPINode::SendData() {
  NS_LOG_FUNCTION(this << m_rank);
  while (m_send_buffer_size != 0) {
    uint32_t socketSize = m_send_socket->GetTxAvailable();
    uint32_t packetSize = MPI_NODE_LINK_MTU;
    uint32_t contentSize =
        std::min(m_send_buffer_size, packetSize - MPI_HEADER_SIZE);
    contentSize = std::min(contentSize, socketSize);
    if (contentSize == 0) {
      break;
    }
    Ptr<Packet> packet = Create<Packet>(contentSize);
    MPIHeader header;
    packet->AddHeader(header);
    packetSize += header.GetSerializedSize();
    int actual = m_send_socket->Send(packet);
    if (actual > 0) {
      m_send_buffer_size -= (actual - MPI_HEADER_SIZE);
      m_txTrace(packet);
    }
    if ((unsigned)actual != packetSize) {
      break;
    }
  }
  if (m_send_buffer_size == 0) {
    StopSending();
    m_send_connected = false;
  }
}

void MPINode::HandleSend(Ptr<Socket>, uint32_t) {
  NS_LOG_FUNCTION(this << m_rank);
  if (m_send_connected) {
    SendData();
  }
}

void MPINode::ConnectionFailed(Ptr<Socket> socket) {
  NS_LOG_FUNCTION(this << m_rank << socket);
  m_send_socket->SetRecvCallback(MakeNullCallback<void, Ptr<Socket>>());
  NS_LOG_INFO(this << " ConnectionFailed to target.");
}

} // Namespace ns3
