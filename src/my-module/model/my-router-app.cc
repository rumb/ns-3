#include "my-router-app.h"

#include "ns3/log.h"
#include "ns3/ipv4-address.h"
#include "ns3/address-utils.h"
#include "ns3/inet-socket-address.h"
#include "ns3/socket.h"
#include "ns3/udp-socket.h"
#include "ns3/simulator.h"
#include "ns3/socket-factory.h"
#include "ns3/packet.h"
#include "ns3/uinteger.h"
#include "ns3/ipv4.h"
#include "ns3/double.h"

#include <vector>

namespace ns3 {

  NS_LOG_COMPONENT_DEFINE ("myRouterApp");
  NS_OBJECT_ENSURE_REGISTERED (myRouterApp);

  TypeId  myRouterApp::GetTypeId (void)
  {
    static TypeId tid = TypeId ("ns3::myRouterApp")
      .SetParent<Application> ()
      .AddConstructor<myRouterApp> ()
      .AddAttribute ("CacheAwareMode", "Select formula that calculates potential value.",
          UintegerValue (0),
          MakeUintegerAccessor (&myRouterApp::m_cache_aware_mode),
          MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("Port", "Port on which we listen for incoming packets.",
          UintegerValue (9),
          MakeUintegerAccessor (&myRouterApp::m_port),
          MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("AppPort", "Port for which we send packets to other application.",
          UintegerValue (8),
          MakeUintegerAccessor (&myRouterApp::m_app_port),
          MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("RoutingInterval",
          "The time to send routing packets.",
          TimeValue (Minutes (30)),
          MakeTimeAccessor (&myRouterApp::m_routing_interval),
          MakeTimeChecker ())
      .AddAttribute ("PBRInterval",
          "The time to send PBR packets.",
          TimeValue (Seconds (1.0)),
          MakeTimeAccessor (&myRouterApp::m_pbr_interval),
          MakeTimeChecker ())
      .AddAttribute ("HopLimit",
          "The number of hop to which PBR packets will be sent.",
          UintegerValue (2),
          MakeUintegerAccessor (&myRouterApp::m_hop_limit),
          MakeUintegerChecker<uint16_t> ())
      .AddAttribute ("D",
          "Variable for Potential Based routing.",
          DoubleValue (0.2),
          MakeDoubleAccessor (&myRouterApp::m_D),
          MakeDoubleChecker<double> ())
      .AddAttribute ("Rho",
          "Variable for Potential Based routing.",
          DoubleValue (0.02),
          MakeDoubleAccessor (&myRouterApp::m_rho),
          MakeDoubleChecker<double> ())
      .AddAttribute ("CacheProbability",
          "Probability of caching a content.",
          DoubleValue (0.2),
          MakeDoubleAccessor (&myRouterApp::m_cache_prob),
          MakeDoubleChecker<double> ())
      .AddAttribute ("CacheTTL",
          "The limit time to expire contents.",
          TimeValue (Seconds (300.0)),
          MakeTimeAccessor (&myRouterApp::m_cache_ttl),
          MakeTimeChecker ())
      ;
    return tid;
  }

  myRouterApp::myRouterApp ()
  {
    NS_LOG_FUNCTION (this);

    m_sent  = 0;
    m_listen_socket = 0;
    m_broadcast_socket = 0;

    m_routing_event = EventId();
    m_pbr_event = EventId();
    m_routine_event = EventId();
  }

  myRouterApp::~myRouterApp()
  {
    NS_LOG_FUNCTION (this);

    m_listen_socket = 0;
    m_broadcast_socket = 0;
  }

  void myRouterApp::DoDispose (void)
  {
    NS_LOG_FUNCTION (this);
    Application::DoDispose ();
  }

  void myRouterApp::StartApplication (void)
  {
    NS_LOG_FUNCTION (this);

    PBR.Init (GetNode()->GetId(), m_D, m_rho, m_hop_limit, m_pbr_interval, m_cache_ttl);
    PBR.SetMode(m_cache_aware_mode);

    UniRand =  CreateObject<UniformRandomVariable> ();
    UniRand->SetAttribute ("Min", DoubleValue (0.0));
    UniRand->SetAttribute ("Max", DoubleValue (1.0));

    ScheduleTransmitRoute (Seconds (0.1));
    ScheduleTransmitPBR (Seconds (UniRand->GetValue() * m_pbr_interval.GetSeconds()));
    ScheduleRoutine (Seconds (10));

    if (m_listen_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_listen_socket = Socket::CreateSocket (GetNode (), tid);
      InetSocketAddress local = InetSocketAddress (Ipv4Address::GetAny (), m_port);
      m_listen_socket->Bind (local);
    }

    m_listen_socket->SetRecvCallback (MakeCallback (&myRouterApp::HandleRead, this));

    if (m_broadcast_socket == 0)
    {
      TypeId tid = TypeId::LookupByName ("ns3::UdpSocketFactory");
      m_broadcast_socket = Socket::CreateSocket (GetNode (), tid);
      m_broadcast_socket->SetAllowBroadcast (true);
      m_broadcast_socket->Connect (InetSocketAddress (Ipv4Address::GetBroadcast(), m_port));
    }
  }

  void myRouterApp::StopApplication ()
  {
    NS_LOG_FUNCTION (this);

    if (m_listen_socket != 0)
    {
      m_listen_socket->Close ();
      m_listen_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_listen_socket = 0;
    }
    if (m_broadcast_socket != 0)
    {
      m_broadcast_socket->Close ();
      m_broadcast_socket->SetRecvCallback (MakeNullCallback<void, Ptr<Socket> > ());
      m_broadcast_socket = 0;
    }

    Simulator::Cancel (m_routing_event);
    Simulator::Cancel (m_pbr_event);
    Simulator::Cancel (m_routine_event);
  }

  void myRouterApp::ScheduleTransmitRoute (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_routing_event = Simulator::Schedule (dt, &myRouterApp::SendRoute, this);
  }

  void  myRouterApp::ScheduleTransmitPBR (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_pbr_event = Simulator::Schedule (dt, &myRouterApp::SendPBR, this);
  }

  void  myRouterApp::ScheduleRoutine (Time dt)
  {
    NS_LOG_FUNCTION (this << dt);
    m_routine_event = Simulator::Schedule (dt, &myRouterApp::Routine, this);
  }

  void myRouterApp::HandleRead (Ptr<Socket> socket)
  {
    NS_LOG_FUNCTION (this << socket);
    Ptr<Packet> packet;
    Address from;

    while ( (packet = socket->RecvFrom (from)) )
    {
      Ipv4Address ipv4_from;
      ipv4_from = InetSocketAddress::ConvertFrom(from).GetIpv4();

      myPacket myPkt (packet);

      //NS_LOG_INFO(Simulator::Now().GetSeconds() << "\tsec Router Node:\t" << GetNode()->GetId() << "\t" << myPkt.GetType() );
      if( myPkt.GetType() == myPacket::TYPE_NAME_INTEREST ){ // Interestパケットを受信した時
        RecvInterest (myPkt.GetInterest(), ipv4_from);
      }else if( myPkt.GetType() == myPacket::TYPE_NAME_DATA ){ // Dataパケットを受信した時
        RecvData (myPkt.GetData(), ipv4_from);
      }else if( myPkt.GetType() == myPacket::TYPE_NAME_ROUTING ){ // Routingパケットを受信した時
        RecvRoute (myPkt.GetRouting(), ipv4_from);
      }else if( myPkt.GetType() == myPacket::TYPE_NAME_PBR ){ // PBRパケットを受信した時
        RecvPBR (myPkt.GetPBR(), ipv4_from);
      }
    } // while() end
  } // HandleRead() end

  void  myRouterApp::RecvInterest (Ptr<myPacket::Interest> ptr, Ipv4Address from)
  {
    NS_LOG_FUNCTION (this);
    std::string name = ptr->content_name;
    Ipv4Address addr;
    Ptr<Packet> packet;

    if( ContentStore.CheckEntry(name) ){
      // キャッシュがあった場合
      addr = from;
      packet = ContentStore.GetDataPacket (name);
      SendPacket (packet, addr);
      return;
    }
    // キャッシュがない場合

    if( PIT.CheckForwardedAddr(name, from) ){
      // Interestを送信した先からInterestが送られてきた場合
      addr = FIB.GetNextAddr (name);
      PIT.AddEntry (name, from);
    }else if( PIT.CheckEntry(name) ){
      // 既にPITにエントリーがある場合
      PIT.AddEntry (name, from);
      return;
    }else{

      PIT.AddEntry (name, from);

      // 通常Interest送信パターン
      if( PBR.CheckEntry(name) ){
        // PBRにエントリーがあった場合
        addr = PBR.GetNextAddr (name);
        if( addr == from || addr == Ipv4Address::GetZero() || addr == Ipv4Address::GetLoopback() ){
          addr = FIB.GetNextAddr (name);
          PBR.PBRmap.erase (name);
        }
      }else{
        // PBRにエントリーがなかった場合
        addr = FIB.GetNextAddr (name);
      }
    }

    myPacket myPkt (myPacket::TYPE_NAME_INTEREST);
    myPkt.AddInterest (name);
    packet = myPkt.GetPacket();

    SendPacket (packet, addr);
    PIT.SetForwardedAddr(name, addr);
  }

  void  myRouterApp::RecvData (Ptr<myPacket::Data> ptr, Ipv4Address from)
  {
    NS_LOG_FUNCTION (this);
    std::string name = ptr->content_name;

    if( ContentStore.CheckEntry(name) ){
      ContentStore.AddEntry(ptr);
    }else if( UniRand->GetValue() < m_cache_prob ){
      ContentStore.AddEntry (ptr);
    }

    std::vector<Ipv4Address>* addr_vec = PIT.GetNextAddrs (name);

    if( addr_vec == NULL ){
      // エントリーがない場合の処理
      return ;
    }

    for ( uint32_t i=0; i < addr_vec->size(); ++i){
      Ipv4Address addr;
      addr = addr_vec->at(i);

      myPacket myPkt (myPacket::TYPE_NAME_DATA);
      myPkt.AddData (ptr);
      Ptr<Packet> packet = myPkt.GetPacket();

      SendPacket (packet, addr);
    } // exit for loop

    PIT.ClearEntry (name);
  } // RecvData() end

  void myRouterApp::RecvRoute (Ptr<myPacket::Routing> ptr, Ipv4Address from)
  {
    NS_LOG_FUNCTION (this);
    FIB.AddEntry (ptr->content_name, from, ptr->serial_num);
  }

  void  myRouterApp::RecvPBR (Ptr<myPacket::PBR> ptr, Ipv4Address from)
  {
    NS_LOG_FUNCTION (this);
    if( PBR.isOverHopLimit (ptr) ){
      // ポテンシャルを広告する範囲を超えている場合には無視
      return;
    }
    if( ptr->node_num == GetNode()->GetId() ){
      // 自分がポテンシャルのソースだった場合には無視
      return;
    }
    // ついか
    if( !PBR.CheckEntry (ptr->content_name){
        PBR.AddEntry (ptr, from);
        SendPacket (PBR.GetPBRPacket(ptr->content_name));
        }else{
    PBR.AddEntry (ptr, from);
    }
  }

  void myRouterApp::SendRoute (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_routing_event.IsExpired ());

    for (std::map<std::string, Ptr<myFIB::FIB_Entry> >::iterator itr = FIB.FIBmap.begin(); itr != FIB.FIBmap.end(); itr++) {
      std::string name = itr->first;
      SendPacket (FIB.GetRoutingPacket(name));
    }

    if( m_sent < 20 ){
      m_sent++;
      ScheduleTransmitRoute (Seconds(0.1));
    }else{
      ScheduleTransmitRoute (m_routing_interval);
    }
  }

  void myRouterApp::SendPBR (void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_pbr_event.IsExpired ());

    PBR.CheckExpire();
    PBR.UpdateEntry();

    for (std::map<std::string, Ptr<myPBR::PBR_Entry> >::iterator itr = PBR.PBRmap.begin(); itr != PBR.PBRmap.end(); itr++) {
      std::string name = itr->first;
      if( ! PBR.isOverHopLimit( itr->second ) ){
        SendPacket (PBR.GetPBRPacket(name));
      }
    }

    ScheduleTransmitPBR (m_pbr_interval);
  }

  void myRouterApp::SendPacket (Ptr<Packet> ptr)
  {
    NS_LOG_FUNCTION (this);

    if( ptr == NULL ){
      NS_LOG_WARN ("Packet does not exit. ptr is \"NULL\"");
      return;
    }
    m_broadcast_socket->Send (ptr);
  }

  void myRouterApp::SendPacket (Ptr<Packet> ptr, Ipv4Address addr)
  {
    NS_LOG_FUNCTION (this);
    uint16_t port;

    if( ptr == NULL ){
      NS_LOG_WARN ("Packet does not exit. ptr is \"NULL\"");
      return;
    }
    if( addr == Ipv4Address::GetZero() ){
      NS_LOG_WARN ("Destination address does not exit. addr is \"0.0.0.0\"");
      return;
    }

    if( addr == Ipv4Address::GetLoopback() ){
      port = m_app_port;
    }else{
      port = m_port;
    }

    ns3::TypeId tid = ns3::TypeId::LookupByName ("ns3::UdpSocketFactory");
    Ptr<Socket> socket = Socket::CreateSocket (GetNode (), tid);
    int b_connect = socket->Connect (InetSocketAddress (addr, port));
    if( b_connect ){
      NS_LOG_WARN ("Failed to connect socket");
    }

    int byte = socket->Send (ptr);
    if( byte != (int) ptr->GetSize() ){
      NS_LOG_WARN ("Can not send all bytes of packet");
    }

    int b_close = socket->Close ();
    if( b_close){
      NS_LOG_WARN ("Failed to close socket");
    }
  }

  void myRouterApp::Routine(void)
  {
    NS_LOG_FUNCTION (this);
    NS_ASSERT (m_routine_event.IsExpired ());

    ContentStore.CheckExpire();
    PBR.CheckExpire();

    for (std::map<std::string, Ptr<myPIT::PIT_Entry> >::iterator itr = PIT.PITmap.begin(); itr != PIT.PITmap.end(); itr++) {
      std::string name = itr->first;
      if( PIT.CheckExpire(name) ){
        myPacket myPkt (myPacket::TYPE_NAME_INTEREST);
        myPkt.AddInterest (name);
        Ptr<Packet> packet = myPkt.GetPacket();

        Ipv4Address addr = FIB.GetNextAddr (name);
        SendPacket (packet, addr);
        PIT.SetForwardedAddr(name, addr);
      }
    }

    for (std::map<std::string, Ptr<myContentStore::CS_Entry> >::iterator itr = ContentStore.CSmap.begin(); itr != ContentStore.CSmap.end(); itr++) {
      std::string name = itr->first;
      Ptr<myContentStore::CS_Entry> entry = itr->second;
      if( entry->expire_time > Simulator::Now() + m_pbr_interval ){
        PBR.AddEntry(name, entry->expire_time, entry->expire_time - entry->gen_time);
      }
    }

    ScheduleRoutine (Seconds(1.0));
  }

} // Namespace ns3
