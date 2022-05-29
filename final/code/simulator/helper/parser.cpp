#include <algorithm>
#include <fstream>
#include <iostream>

#include "parser.h"

using namespace ns3;

void DebugEvents(const std::vector<simpi_event_tagged_t> &events);
void DebugAllEvents(
    const std::vector<std::vector<simpi_event_tagged_t>> &events);

void HandleBcast(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                   uint32_t size, uint16_t root, uint16_t comm_size);
void HandleGather(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                  uint32_t size, uint16_t root, uint16_t comm_size);
void HandleScatter(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                   uint32_t size, uint16_t root, uint16_t comm_size);

std::vector<std::vector<simpi_event_tagged_t>> Parse(uint16_t num_processes,
                                                     std::string logName) {
  std::vector<std::vector<simpi_event_tagged_t>> events(num_processes);

  for (size_t i = 0; i < num_processes; i++) {
    std::vector<simpi_event_tagged_t> rank_events;
    events[i] = rank_events;
  }

  std::ifstream logs;
  logs.open(logName);
  if (logs.fail()) { //    Check open
    std::cerr << "Can't open log file\n";
    exit(1);
  }

  while (!logs.eof()) {
    uint16_t rank;
    simpi_event_tagged_t tagged;
    simpi_event_t event;

    logs >> rank;
    if (logs.fail() || logs.bad()) {
      break;
    }

    int event_type;
    logs >> event_type;

    switch (event_type) {
    case 0: {
      // Error
      int retval;
      logs >> retval;
      event.compute_event = {40000};
      tagged = {SimpiEventType::Compute, event};
      events[rank].push_back(tagged);
    } break;
    case 1: {
      // Compute
      long long num_instructions;
      logs >> num_instructions;
      event.compute_event = {num_instructions};
      tagged = {SimpiEventType::Compute, event};
      events[rank].push_back(tagged);
    } break;
    case 2: {
      // Recv
      uint32_t size;
      uint16_t from;
      logs >> size >> from;
      event.recv_event = {from, size};
      tagged = {SimpiEventType::Recv, event};
      events[rank].push_back(tagged);
    } break;
    case 3: {
      // Send
      uint32_t size;
      uint16_t to;
      logs >> size >> to;
      event.send_event = {to, size};
      tagged = {SimpiEventType::Send, event};
      events[rank].push_back(tagged);
    } break;
    case 4: {
      // Bcast
      uint32_t size;
      uint16_t root;
      logs >> size >> root;
      HandleBcast(events[rank], rank, size, root, num_processes);
    } break;
    case 5: {
      // Scatter
      uint32_t size;
      uint16_t root;
      logs >> size >> root;
      HandleScatter(events[rank], rank, size, root, num_processes);
    } break;
    case 6: {
      // Gather
      uint32_t size;
      uint16_t root;
      logs >> size >> root;
      HandleGather(events[rank], rank, size, root, num_processes);
    } break;
    }
  }

  DebugAllEvents(events);

  logs.close();
  return events;
}

#define MPIR_CVAR_BCAST_SHORT_MSG_SIZE 12288
#define MPIR_CVAR_BCAST_MIN_PROCS 8
#define MPIR_CVAR_BCAST_LONG_MSG_SIZE 524288

inline bool is_pof2(uint16_t n) { return n && !(n & (n - 1)); }

void HandleBcastBinomial(std::vector<simpi_event_tagged_t> &, uint16_t,
                         uint32_t, uint16_t, uint16_t);
void HandleBcastScatter(std::vector<simpi_event_tagged_t> &, uint16_t, uint32_t,
                        uint16_t, uint16_t);
void HandleBcastScatterDoublingAllgather(std::vector<simpi_event_tagged_t> &,
                                         uint16_t, uint32_t, uint16_t,
                                         uint16_t);
void HandleBcastScatterRingAllgather(std::vector<simpi_event_tagged_t> &,
                                     uint16_t, uint32_t, uint16_t, uint16_t);
void HandleBcast(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                 uint32_t nbytes, uint16_t root, uint16_t comm_size) {
  if (nbytes == 0) {
    return;
  }

  if ((nbytes < MPIR_CVAR_BCAST_SHORT_MSG_SIZE) ||
      (comm_size < MPIR_CVAR_BCAST_MIN_PROCS)) {
    HandleBcastBinomial(events, rank, nbytes, root, comm_size);
  } else {
    if ((nbytes < MPIR_CVAR_BCAST_LONG_MSG_SIZE) && (is_pof2(comm_size))) {
      // TODO: Implement this
      std::cerr << "Unsupported Broadcast type" << std::endl;
      HandleBcastScatterDoublingAllgather(events, rank, nbytes, root,
                                          comm_size);
    } else {
      // TODO: Implement this
      std::cerr << "Unsupported Broadcast type" << std::endl;
      HandleBcastScatterRingAllgather(events, rank, nbytes, root, comm_size);
    }
  }
}

void HandleBcastBinomial(std::vector<simpi_event_tagged_t> &events,
                         uint16_t rank, uint32_t nbytes, uint16_t root,
                         uint16_t comm_size) {
  simpi_event_tagged_t tagged;
  simpi_event_t event;

  int src, dst, relative_rank, mask;
  if (comm_size == 1) {
    return;
  }

  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

  mask = 0x1;
  while (mask < comm_size) {
    if (relative_rank & mask) {
      src = rank - mask;
      if (src < 0)
        src += comm_size;
      event.recv_event = {(uint16_t)src, nbytes};
      tagged = {SimpiEventType::Recv, event};
      events.push_back(tagged);
      break;
    }
    mask <<= 1;
  }

  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      dst = rank + mask;
      if (dst >= comm_size)
        dst -= comm_size;
      event.send_event = {(uint16_t)dst, nbytes};
      tagged = {SimpiEventType::Send, event};
      events.push_back(tagged);
    }
    mask >>= 1;
  }
}

int calcRecvSizeBcastScatter(int rank, int dst_rank, int nbytes, int root,
                             int comm_size) {
  int src, dst;
  int relative_rank, mask;
  int scatter_size, curr_size, recv_size, send_size;
  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;
  scatter_size = (nbytes + comm_size - 1) / comm_size; /* ceiling division */
  curr_size = (rank == root) ? nbytes : 0; /* root starts with all the
                                              data */

  mask = 0x1;
  while (mask < comm_size) {
    if (relative_rank & mask) {
      src = rank - mask;
      if (src < 0)
        src += comm_size;
      recv_size = nbytes - relative_rank * scatter_size;
      if (recv_size <= 0) {
        curr_size = 0; /* this process doesn't receive any data
                          because of uneven division */
      } else {
        recv_size =
            calcRecvSizeBcastScatter(src, rank, nbytes, root, comm_size);
      }
      break;
    }
    mask <<= 1;
  }

  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      send_size = curr_size - scatter_size * mask;
      /* mask is also the size of this process's subtree */

      if (send_size > 0) {
        dst = rank + mask;
        if (dst >= comm_size)
          dst -= comm_size;
        if (dst == dst_rank) {
          return send_size;
        }
        curr_size -= send_size;
      }
    }
    mask >>= 1;
  }

  return 0;
}

void HandleBcastScatter(std::vector<simpi_event_tagged_t> &events, int rank,
                        int nbytes, int root, int comm_size) {
  simpi_event_tagged_t tagged;
  simpi_event_t event;
  int src, dst;
  int relative_rank, mask;
  int scatter_size, curr_size, recv_size, send_size;

  relative_rank = (rank >= root) ? rank - root : rank - root + comm_size;

  scatter_size = (nbytes + comm_size - 1) / comm_size; /* ceiling division */
  curr_size = (rank == root) ? nbytes : 0; /* root starts with all the
                                              data */

  mask = 0x1;
  while (mask < comm_size) {
    if (relative_rank & mask) {
      src = rank - mask;
      if (src < 0)
        src += comm_size;
      recv_size = nbytes - relative_rank * scatter_size;
      if (recv_size <= 0) {
        curr_size = 0; /* this process doesn't receive any data
                          because of uneven division */
      } else {
        /* recv_size is larger than what might actually be sent by the
           sender. We don't need compute the exact value because MPI
           allows you to post a larger recv.*/
        // TODO: calculate actual receive

        recv_size =
            calcRecvSizeBcastScatter(src, rank, nbytes, root, comm_size);
        event.recv_event = {(uint16_t)src, (uint32_t)recv_size};
        tagged = {SimpiEventType::Recv, event};
        events.push_back(tagged);

        curr_size = recv_size;
      }
      break;
    }
    mask <<= 1;
  }

  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      send_size = curr_size - scatter_size * mask;
      /* mask is also the size of this process's subtree */

      if (send_size > 0) {
        dst = rank + mask;
        if (dst >= comm_size)
          dst -= comm_size;
        event.send_event = {(uint16_t)dst, (uint32_t)send_size};
        tagged = {SimpiEventType::Send, event};
        events.push_back(tagged);

        curr_size -= send_size;
      }
    }
    mask >>= 1;
  }
}

void HandleBcastScatterDoublingAllgather(
    std::vector<simpi_event_tagged_t> &events, uint16_t rank, uint32_t nbytes,
    uint16_t root, uint16_t comm_size) {
  HandleBcastBinomial(events, rank, nbytes, root, comm_size);
}

void HandleBcastScatterRingAllgather(std::vector<simpi_event_tagged_t> &events,
                                     uint16_t rank, uint32_t nbytes,
                                     uint16_t root, uint16_t comm_size) {
  HandleBcastBinomial(events, rank, nbytes, root, comm_size);
}

void HandleGather(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                  uint32_t size, uint16_t root, uint16_t comm_size) {

  simpi_event_tagged_t tagged;
  simpi_event_t event;

  uint16_t relative_rank =
      (rank >= root) ? rank - root : rank - root + comm_size;
  uint32_t nbytes = size;
  uint16_t mask;
  for (mask = 1; mask < comm_size; mask <<= 1)
    ;
  --mask;
  while (relative_rank & mask)
    mask >>= 1;
  int missing = (relative_rank | mask) - comm_size + 1;
  if (missing < 0)
    missing = 0;
  uint16_t tmp_buf_size = mask - missing;
  uint32_t curr_cnt = nbytes;
  mask = 0x1;
  while (mask < comm_size) {

    if ((mask & relative_rank) == 0) {
      int src = relative_rank | mask;
      if (src < comm_size) {
        src = (src + root) % comm_size;
        /* Estimate the amount of data that is going to come in */
        uint16_t recvblks = mask;
        uint16_t relative_src =
            ((src - root) < 0) ? (src - root + comm_size) : (src - root);
        if (relative_src + mask > comm_size)
          recvblks -= (relative_src + mask - comm_size);

        event.recv_event = {(uint16_t)src, recvblks * nbytes};
        tagged = {SimpiEventType::Recv, event};
        events.push_back(tagged);
        curr_cnt += (recvblks * nbytes);
      }
    } else {
      uint16_t dst = relative_rank ^ mask;
      dst = (dst + root) % comm_size;

      if (!tmp_buf_size) {
        /* leaf nodes send directly from sendbuf */
        event.send_event = {dst, size};
      } else {
        event.send_event = {dst, curr_cnt};
      }
      tagged = {SimpiEventType::Send, event};
      events.push_back(tagged);
      break;
    }
    mask <<= 1;
  }
}

void HandleScatter(std::vector<simpi_event_tagged_t> &events, uint16_t rank,
                   uint32_t size, uint16_t root, uint16_t comm_size) {

  // Based off src/mpi/coll/scatter.c of mpich-3.2.1

  simpi_event_tagged_t tagged;
  simpi_event_t event;

  uint16_t relative_rank =
      (rank >= root) ? rank - root : rank - root + comm_size;
  uint32_t curr_cnt = 0;
  if (rank == root) {
    curr_cnt = size * comm_size;
  }
  uint16_t mask = 0x1;
  while (mask < comm_size) {
    if (relative_rank & mask) {
      int src = rank - mask;
      if (src < 0)
        src += comm_size;
      event.recv_event = {(uint16_t)src, size * mask};
      tagged = {SimpiEventType::Recv, event};
      events.push_back(tagged);
      curr_cnt = size * mask;
      break;
    }
    mask <<= 1;
  }
  mask >>= 1;
  while (mask > 0) {
    if (relative_rank + mask < comm_size) {
      uint16_t dst = rank + mask;
      if (dst >= comm_size)
        dst -= comm_size;
      uint32_t send_subtree_cnt = curr_cnt - size * mask;
      event.send_event = {dst, send_subtree_cnt};
      tagged = {SimpiEventType::Send, event};
      events.push_back(tagged);
      curr_cnt -= send_subtree_cnt;
    }
    mask >>= 1;
  }
}

void DebugAllEvents(
    const std::vector<std::vector<simpi_event_tagged_t>> &events) {
  for (size_t i = 0; i < events.size(); i++) {
    std::cout << "Rank " << i << std::endl;
    std::cout << "=========================" << std::endl;
    DebugEvents(events[i]);
    std::cout << "=========================" << std::endl;
  }
}

void DebugEvents(const std::vector<simpi_event_tagged_t> &events) {
  for (size_t i = 0; i < events.size(); i++) {
    if (events[i].event_type == SimpiEventType::Compute) {
      std::cout << "compute " << events[i].event.compute_event.num_instructions
                << std::endl;
    } else if (events[i].event_type == SimpiEventType::Recv) {
      std::cout << "recv " << events[i].event.recv_event.data_size << " "
                << events[i].event.recv_event.from_rank << std::endl;
    } else if (events[i].event_type == SimpiEventType::Send) {
      std::cout << "send " << events[i].event.send_event.data_size << " "
                << events[i].event.send_event.to_rank << std::endl;
    }
  }
}
