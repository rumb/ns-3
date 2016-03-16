#ifndef my_ROUTER_APP_H
#define my_ROUTER_APP_H

#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/address.h"
#include "ns3/application.h"
#include "ns3/nstime.h"
#include "ns3/random-variable-stream.h"

#include <map>

#include "my-app-packet.h"
#include "my-router-module.h"

namespace ns3 {

  class Socket;
  class Packet;

  class myRouterApp : public Application
  {

    public:
      static TypeId GetTypeId (void);
      myRouterApp ();
      virtual ~myRouterApp ();

    protected:
      virtual void DoDispose (void);
    private:
      virtual void StartApplication (void);
      virtual void StopApplication (void);

      void ScheduleTransmitRoute (Time dt);
      void ScheduleTransmitPBR (Time dt);
      void ScheduleRoutine (Time dt);

      void HandleRead (Ptr<Socket> socket);
      void RecvInterest(Ptr<myPacket::Interest> ptr, Ipv4Address from);
      void RecvData(Ptr<myPacket::Data> ptr, Ipv4Address from);
      void RecvRoute(Ptr<myPacket::Routing> ptr, Ipv4Address from);
      // PBR
      void RecvPBR(Ptr<myPacket::PBR> ptr, Ipv4Address from);

      void SendRoute (void);
      void SendPBR (void);
      void SendInterest (void);
      void SendPacket (Ptr<Packet>);
      void SendPacket (Ptr<Packet> ptr, Ipv4Address addr);
      void Routine (void);

      myFIB FIB;
      myPIT PIT;
      myContentStore ContentStore;
      myPBR PBR;

      double m_D;
      double m_rho;
      double m_cache_aware_mode;

      uint16_t m_hop_limit;
      double m_cache_prob;

      uint16_t m_sent;
      uint16_t m_port; //!< Port on which we listen for incoming packets.
      uint16_t m_app_port; //!< Port on which we listen for incoming packets.

      Ptr<Socket> m_broadcast_socket; //!< IPv4 Socket
      Ptr<Socket> m_listen_socket; //!< IPv4 Socket

      Time m_routing_interval; //!< Packet inter-send time
      Time m_pbr_interval; //!< Packet inter-send time
      Time m_cache_ttl;

      EventId m_routing_event; //!< Event to send the next packet
      EventId m_pbr_event; //!< Event to send the next packet
      EventId m_routine_event; //!< Event to send the next packet

      Ptr<UniformRandomVariable>  UniRand;
  };

} // namespace ns3

#endif /* UDP_ECHO_SERVER_H */
