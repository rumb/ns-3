#ifndef my_ENDNODE_APP_H
#define my_ENDNODE_APP_H

#include "ns3/application.h"
#include "ns3/event-id.h"
#include "ns3/ptr.h"
#include "ns3/ipv4-address.h"
#include "ns3/random-variable-stream.h"
//#include "ns3/traced-callback.h"
#include <map>

#include "ns3/my-app-packet.h"

namespace ns3 {

  class Socket;
  class Packet;

  class myEndNodeApp : public Application
  {
    public:
      static uint64_t send_count;
      static TypeId GetTypeId (void);
      myEndNodeApp ();
      virtual ~myEndNodeApp ();
    protected:
      virtual void DoDispose (void);
    private:
      virtual void StartApplication (void);
      virtual void StopApplication (void);
      void ScheduleTransmitInterest (Time dt);
      void ScheduleTransmitRoute (Time dt);
      void ScheduleTransmitPBR (Time dt);
      void ScheduleUpdate (Time dt);

      void HandleRead (Ptr<Socket> socket);

      void SendInterest (void);
      void SendData (Ptr<myPacket::Interest> ptr);
      void RecvData (Ptr<myPacket::Data> ptr);
      void SendRoute (void);
      void SendPBR (void);
      void SendPacket (Ptr<Packet>);
      void UpdateContents (void);

      std::map<std::string, Ptr<Packet> > Content;
      std::vector<std::string> content_name;

      // m_role = 0 Interestを送信するモード
      // m_role = 1 Dataを送信するモード
      uint8_t m_role;
      uint8_t m_pbr_mode;

      uint32_t m_count; //!< Maximum number of packets the application will send
      uint32_t m_sent; //!< Counter for sent packets

      double m_potential_origin; //!< Counter for sent packets

      Ipv4Address m_peerAddress; //!< Remote peer address
      uint16_t m_peerPort; //!< Remote peer port
      uint16_t m_port; //!< Port on which we listen for incoming packets.

      Ptr<Socket> m_listen_socket; //!< Socket
      Ptr<Socket> m_send_socket; //!< Socket

      EventId m_request_event; //!< Event to send the next packet
      EventId m_routing_event; //!< Event to send the next packet
      EventId m_pbr_event; //!< Event to send the next packet
      EventId m_update_event; //!< Event to send the next packet

      Time m_routing_interval; //!< Packet inter-send time
      Time m_request_interval; //!< Packet inter-send time
      Time m_pbr_interval;
      Time m_update_interval;
      Time m_cache_ttl;

      Ptr<ExponentialRandomVariable> ExpRand;
      Ptr<UniformRandomVariable> UniRand;

      Time m_last_send_time;

      // Callbacks for tracing the packet Tx events
      // TracedCallback<Ptr<const Packet> > m_txTrace;
  };

} // namespace ns3

#endif /* my_ENDNODE_APP_H */
