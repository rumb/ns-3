#include "my-endnode-app.h"

#include "ns3/log.h"
#include "ns3/nstime.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
//#include "ns3/trace-source-accessor.h"

#include "ns3/double.h"
#include <vector>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("myEndNodeApp");
  NS_OBJECT_ENSURE_REGISTERED (myEndNodeApp);

  uint64_t myEndNodeApp::send_count = 0;

  TypeId myEndNodeApp::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::myEndNodeApp")
      .SetParent<Application> ()
      .AddConstructor<myEndNodeApp> ()
      .AddAttribute ("Role",
          "EndNode operation role.",
          UintegerValue (0),
          MakeUintegerAccessor (&myEndNodeApp::m_role),
          MakeUintegerChecker<uint8_t> ())
      .AddAttribute ("PBRMode",
          "Server node pbr operation mode.",
          UintegerValue (0),
          MakeUintegerAccessor (&myEndNodeApp::m_pbr_mode),
          MakeUintegerChecker<uint8_t> ())
      .AddAttribute ("RequestInterval",
          "The average time to send interest packets.",
          TimeValue (Seconds (60.0)),
          MakeTimeAccessor (&myEndNodeApp::m_request_interval),
          MakeTimeChecker ())
      .AddAttribute ("RoutingInterval",
          "The time to send routing packets.",
          TimeValue (Minutes (30.0)),
          MakeTimeAccessor (&myEndNodeApp::m_routing_interval),
          MakeTimeChecker ())
      .AddAttribute ("PBRInterval",
          "The time to send pbr packets.",
          TimeValue (Seconds (60.0)),
          MakeTimeAccessor (&myEndNodeApp::m_pbr_interval),
          MakeTimeChecker ())
      .AddAttribute ("UpdateInterval",
          "The time to update contents.",
          TimeValue (Seconds (60.0)),
          MakeTimeAccessor (&myEndNodeApp::m_update_interval),
          MakeTimeChecker ())
      .AddAttribute ("CacheTTL",
          "The limit time to expire contents.",
          TimeValue (Seconds (300.0)),
          MakeTimeAccessor (&myEndNodeApp::m_cache_ttl),
          MakeTimeChecker ())
      .AddAttribute ("OriginalContentPotential",
          "Potential Value of the node which has a original content.",
          DoubleValue (0.0),
          MakeDoubleAccessor (&myEndNodeApp::m_potential_origin),
          MakeDoubleChecker<double> ())
      .AddAttribute ("RemoteAddress",
          "The destination Address of the outbound packets.",
          Ipv4AddressValue (Ipv4Address::GetLoopback()),
          MakeIpv4AddressAccessor (&myEndNodeApp::m_peerAddress),
          MakeIpv4AddressChecker ())
      .AddAttribute ("RemotePort",
          "The destination port of the outbound packets.",
          UintegerValue (9),
          MakeUintegerAccessor (&myEndNodeApp::m_peerPort),
          MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("Port", "Port on which we listen for incoming packets.",
          UintegerValue (8),
          MakeUintegerAccessor (&myEndNodeApp::m_port),
          MakeUintegerChecker<uint16_t> ())
      /// MaxPacketsいらないけど消せない
      .AddAttribute ("MaxPackets",
          "The maximum number of packets the application will send.",
          UintegerValue (100),
          MakeUintegerAccessor (&myEndNodeApp::m_count),
          MakeUintegerChecker<uint32_t> ())
      // .AddTraceSource ("Tx", "A new packet is created and is sent",
      //     MakeTraceSourceAccessor (&myEndNodeApp::m_txTrace))
      ;
    return tid;
  }

  myEndNodeApp::myEndNodeApp ()
  {
    NS_LOG_FUNCTION (this);
    m_sent = 0;
    m_send_socket = 0;
    m_listen_socket = 0;

    m_request_event = EventId ();
    m_routing_event = EventId ();
    m_pbr_event = EventId ();
    m_update_event = EventId ();

    // とりあえず
    content_name.push_back("safety");
    //content_name.push_back("chat");

    m_last_send_time = Seconds(0.0);
  }

  myEndNodeApp::~myEndNodeApp()
  {
    NS_LOG_FUNCTION (this);
    m_send_socket = 0;
    m_listen_socket = 0;
  }

  void myEndNodeApp::StartApplication (void)
  {
    NS_LOG_FUNCTION (this);

    // リクエスト送信間隔の指数乱数生成
    ExpRand = CreateObject<ExponentialRandomVariable> ();
    ExpRand->SetAttribute ("Mean",DoubleValue( m_request_interval.GetSeconds()));
    ExpRand->SetAttribute ("Bound",DoubleValue(86400.0 ));
    // 0~1の一様乱数生成
    UniRand =  CreateObject<UniformRandomVariable> ();
    UniRand->SetAttribute ("Min", DoubleValue (0.0));
    UniRand->SetAttribute ("Max", DoubleValue (1.0));

    if( m_role ){
      ScheduleTransmitRoute (Seconds (0.0));
      if( m_pbr_mode ){
        ScheduleTransmitPBR (Seconds (0.0));
      }
      ScheduleUpdate (Seconds (0.0));
    }else{
      ScheduleTransmitInterest (Seconds (m_request_interval.GetSeconds() * UniRand->GetValue() + 60 ));
    }

    if( m_listen_socket == 0 ){ // 待ち受け用
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_listen_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetLoopback(), m_port);
      m_listen_socket->Bind (local);
      m_listen_socket->Listen ();
    }

    m_listen_socket->SetRecvCallback (MakeCallback (&myEndNodeApp::HandleRead, this));

    if( m_send_socket == 0 ){ // 送信用ソケット作成
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_send_socket = Socket::CreateSocket (GetNode (), tid);
      m_send_socket->Connect (InetSocketAddress (m_peerAddress, m_peerPort));
    }
  }

  void myEndNodeApp::StopApplication (void)
  {
    NS_LOG_FUNCTION (this);

    if (m_send_socket != 0)
    {
      m_send_socket->Close ();
      m_send_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_send_socket = 0;
    }
    if (m_listen_socket != 0)
    {
      m_listen_socket->Close ();
      m_listen_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_listen_socket = 0;
    }

    Simulator::Cancel (m_request_event);
    Simulator::Cancel (m_routing_event);
    Simulator::Cancel (m_pbr_event);
    Simulator::Cancel (m_update_event);
  }

  void myEndNodeApp::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
  }

  void  myEndNodeApp::ScheduleTransmitInterest (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_request_event = Simulator::Schedule (dt, &myEndNodeApp::SendInterest, this);
  }

  void  myEndNodeApp::ScheduleTransmitRoute (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_routing_event = Simulator::Schedule (dt, &myEndNodeApp::SendRoute, this);
  }

  void  myEndNodeApp::ScheduleTransmitPBR (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_pbr_event = Simulator::Schedule (dt, &myEndNodeApp::SendPBR, this);
  }

  void  myEndNodeApp::ScheduleUpdate (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_update_event = Simulator::Schedule (dt, &myEndNodeApp::UpdateContents, this);
  }

  void myEndNodeApp::HandleRead (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);

    Ptr<Packet> packet;
    Address from;

    /// ()が二重になっていないとwarningがでる
    while ( (packet = socket->RecvFrom (from)) )
    {
      myPacket myPkt(packet);
      std::string type = myPkt.GetType();

      if( m_role ){
        if( type == myPacket::TYPE_NAME_INTEREST ){
          SendData (myPkt.GetInterest());
        }else{
          NS_LOG_WARN ("Server received wrong packet : " << type);
        }
      }else{
        if( type == myPacket::TYPE_NAME_DATA ){
          RecvData (myPkt.GetData());
        }else{
          NS_LOG_WARN ("Client received wrong packet received : " << type);
        }
      }
    } // exit while loop
  }

  void myEndNodeApp::SendInterest (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_request_event.IsExpired ());

    for( std::vector<std::string>::iterator  itr = content_name.begin(); itr != content_name.end(); ++itr ){
      std::string name = *itr;

      myPacket myPkt (myPacket::TYPE_NAME_INTEREST);

      if( myPkt.AddInterest (name) ){
        SendPacket (myPkt.GetPacket());
      }else{
        NS_LOG_WARN("Failed to send Interest packet");
      }
    }

    if( m_last_send_time != Seconds(0.0) ){
      NS_LOG_INFO("Failed");
    }
    m_last_send_time = Simulator::Now();

    Time next_interval = Seconds (ExpRand->GetValue());
    ScheduleTransmitInterest (next_interval);
  }

  void myEndNodeApp::SendPBR (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_pbr_event.IsExpired ());

    Time current_time = Simulator::Now ();

    for( std::vector<std::string>::iterator itr = content_name.begin(); itr != content_name.end(); ++itr ){
      std::string name = *itr;

      myPacket myPkt (myPacket::TYPE_NAME_PBR);

      if( myPkt.AddPBR (name, -100.0 , 3, myPacket::ORIGINAL_NODE_ID, current_time + Minutes(60) ) ){
        myPkt.AddCacheTTL(m_cache_ttl);
        SendPacket (myPkt.GetPacket());
      }else{
        NS_LOG_WARN("Failed to send PBR packet");
      }
    }

    Time next_interval = m_pbr_interval;
    ScheduleTransmitPBR (next_interval);
  }

  void myEndNodeApp::SendData (Ptr<myPacket::Interest> ptr)
  {
    NS_LOG_FUNCTION (this);
    std::string name = ptr->content_name;

    if( Content[name] == NULL ){
      NS_LOG_WARN ("Content name \"" << name << "\" does not exist");
      return;
    }
    SendPacket (Content[name]);
    send_count++;
  }

  void myEndNodeApp::RecvData (Ptr<myPacket::Data> ptr)
  {
    NS_LOG_FUNCTION (this);
    std::string name = ptr->content_name;
    Time current_time = Simulator::Now ();

    if( m_last_send_time == Seconds(0.0) ){
      return;
    }
    NS_LOG_INFO ( "[Get]\t" << GetNode()->GetId()
        << "\t" << (current_time - m_last_send_time).GetMilliSeconds()
        << "\t" << (current_time - ptr->gen_time).GetMilliSeconds()
        << "\t" << current_time.GetSeconds());
    m_last_send_time = Seconds(0.0);
  }

  void myEndNodeApp::SendRoute (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_routing_event.IsExpired ());

    for( std::vector<std::string>::iterator  itr = content_name.begin(); itr != content_name.end(); ++itr ){

      std::string name = *itr;
      myPacket myPkt (myPacket::TYPE_NAME_ROUTING);

      if( myPkt.AddRouting (name, Simulator::Now().GetMilliSeconds()) ){
        SendPacket (myPkt.GetPacket());
      }else{
        NS_LOG_WARN ("Failed to send Routing packet");
      }
    } // exit for loop

    if( m_sent < 20 ){
      m_sent++;
      ScheduleTransmitRoute (Seconds(0.1));
    }else if( m_sent < m_count ){
      Time next_interval = m_routing_interval;
      ScheduleTransmitRoute (next_interval);
    }
  }

  void myEndNodeApp::SendPacket (Ptr<Packet> ptr)
  {
    NS_LOG_FUNCTION (this);

    if( ptr == NULL ){
      NS_LOG_WARN ("Packet does not exit. ptr is \"NULL\"");
      return;
    }

    m_send_socket->Send (ptr);
  }

  void myEndNodeApp::UpdateContents (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_update_event.IsExpired ());

    Time current_time = Simulator::Now();

    for( std::vector<std::string>::iterator  itr = content_name.begin(); itr != content_name.end(); ++itr ){
      std::string name = *itr;

      myPacket myPkt (myPacket::TYPE_NAME_DATA);
      myPkt.AddData (name, current_time, current_time + m_cache_ttl, "I'm OK!");
      Content[name] = myPkt.GetPacket();
    }

    Time next_interval = m_update_interval;
    ScheduleUpdate (next_interval);
  }

} // Namespace ns3
