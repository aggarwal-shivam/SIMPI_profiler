#ifndef MPI_NODE_H
#define MPI_NODE_H

#include <vector>

#include <ns3/address.h>
#include <ns3/application.h>
#include <ns3/event-id.h>
#include <ns3/ptr.h>
#include <ns3/traced-callback.h>

#include "simpi-event.h"

#define MPI_NODE_PPN 8
#define MPI_NODE_LINK_MTU 1460
#define MPI_NODE_CPU_IPS 6384000000
#define MPI_MAX_WAIT 20
#define MPI_COPY_DELAY_US 20

namespace ns3 {

class Socket;
class Packet;

class MPINode : public Application {
public:
  /**
   * \brief Get the type ID.
   * \return the object TypeId
   */
  static TypeId GetTypeId(void);
  MPINode();
  virtual ~MPINode();

protected:
  virtual void DoDispose(void);

private:
  virtual void StartApplication(void);
  virtual void StopApplication(void);

  /**
   * \brief Handle a packet reception.
   *
   * This function is called by lower layers.
   *
   * \param socket the socket the packet was received to.
   */
  // Receiver
  void StartListening(void);
  void StopListening(void);
  void HandleRead(Ptr<Socket> socket);
  void HandleAccept(Ptr<Socket> socket, const Address &from);
  bool HandleRequest(Ptr<Socket> socket, const Address &from);
  void HandlePeerClose(Ptr<Socket> socket);
  void HandlePeerError(Ptr<Socket> socket);

  // Sending
  void HandleSend(Ptr<Socket> socket, uint32_t availableBufferSize);
  void StartSending(void);
  void StopSending(void);
  void SendData(void);
  void SendBuffer();
  void SentBuffer();
  void ConnectionSucceeded(Ptr<Socket> socket);
  void ConnectionFailed(Ptr<Socket> socket);
  void ConnectionCloseNormal(Ptr<Socket> socket);
  void ConnectionCloseError(Ptr<Socket> socket);

  // Processing
  void ProcessCurrentStep(void);

  // Attribute Set variables
  uint16_t m_rank;
  std::vector<simpi_event_tagged_t> m_simpi_events;
  std::vector<Address> m_addresses;

  // Internal Variables
  //   For receiving
  Ptr<Socket> m_listen_socket;
  Ptr<Socket> m_accepted_socket;
  uint32_t m_recv_buffer_size;
  bool m_recv_from_local;
  //   For sending
  Ptr<Socket> m_send_socket;
  uint32_t m_send_buffer_size;
  uint32_t m_total_send_size;
  bool m_send_connected;
  bool m_send_to_local;
  //   Basic processing
  size_t m_current_step_no;

  /// Callbacks for tracing the packet Tx events
  TracedCallback<Ptr<const Packet>> m_txTrace;

  /// Callbacks for tracing the packet Rx events
  TracedCallback<Ptr<const Packet>> m_rxTrace;

  /// Callbacks for tracing the packet Tx events, includes source and
  /// destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_txTraceWithAddresses;

  /// Callbacks for tracing the packet Rx events, includes source and
  /// destination addresses
  TracedCallback<Ptr<const Packet>, const Address &, const Address &>
      m_rxTraceWithAddresses;
};

} // namespace ns3

#endif /* MPI_NODE_H */
