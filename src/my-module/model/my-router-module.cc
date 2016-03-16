
#include "my-router-module.h"

#include "ns3/log.h"
#include "ns3/packet.h"
#include "ns3/object.h"
#include "ns3/simulator.h"

NS_LOG_COMPONENT_DEFINE ("myRouterModule");

namespace ns3 {

  const double myPBR::INITIAL_POTENTIAL = 1000000.0;
  uint64_t myContentStore::cache_count = 0;
  uint64_t myPBR::pbr_count = 0;

  /// myFIB
  myFIB::myFIB()
  {
    NS_LOG_FUNCTION (this);
  }

  myFIB::FIB_Entry::FIB_Entry (std::string name, Ipv4Address addr, uint32_t serial)
  {
    NS_LOG_FUNCTION (this);
    content_name = name;
    next_addr = addr;
    serial_num = serial;
  }

  Ipv4Address myFIB::GetNextAddr (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( FIBmap.find(name) == FIBmap.end() ){
      NS_LOG_WARN ("FIB entry was not found");
      return Ipv4Address::GetZero();
    }
    return FIBmap[name]->next_addr;
  }

  void myFIB::AddEntry (std::string name, Ipv4Address addr, uint32_t serial)
  {
    NS_LOG_FUNCTION (this);
    if( FIBmap.find(name) == FIBmap.end() ){
      FIBmap[name] = Create<FIB_Entry> (name, addr, serial);
    }else{
      if( serial > FIBmap[name]->serial_num ){
        FIBmap[name]->next_addr = addr;
        FIBmap[name]->serial_num = serial;
      }
    }
  }

  Ptr<Packet> myFIB::GetRoutingPacket (std::string name)
  {
    myPacket myPkt (myPacket::TYPE_NAME_ROUTING);
    if( myPkt.AddRouting(name, FIBmap[name]->serial_num) ){
      return  myPkt.GetPacket();
    }
    NS_LOG_WARN("Failed to make Routing Packet");
    return NULL;
  }

  /// myPIT
  myPIT::myPIT()
  {
    NS_LOG_FUNCTION (this);
  }

  myPIT::PIT_Entry::PIT_Entry (std::string name, Ipv4Address addr)
  {
    NS_LOG_FUNCTION (this);
    content_name = name;
    next_addr.push_back(addr);
  }

  std::vector<Ipv4Address>* myPIT::GetNextAddrs (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PITmap.find(name) == PITmap.end() ){
      NS_LOG_WARN ("PIT entry was not found");
      return NULL;
    }
    return &PITmap[name]->next_addr;
  }

  void myPIT::AddEntry (std::string name, Ipv4Address addr)
  {
    NS_LOG_FUNCTION (this);
    if( PITmap.find(name) == PITmap.end() ){
      PITmap[name] = Create<PIT_Entry> (name, addr);
    }else{
      PITmap[name]->next_addr.push_back(addr);
    }
  }

  bool myPIT::CheckExpire (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PITmap.find(name) == PITmap.end() ){
      return false;
    }else if( PITmap[name]->forwarded_time + MilliSeconds(200) > Simulator::Now() ){
      return true;
    }else{
      return false;
    }
  }

  void myPIT::SetForwardedAddr (std::string name, Ipv4Address addr)
  {
    if( PITmap.find(name) == PITmap.end() ){
      NS_LOG_WARN ("Can not set forwarded address. PIT entry was not found");
      return;
    }else{
      PITmap[name]->forwarded_addr = addr;
      PITmap[name]->forwarded_time = Simulator::Now();
    }
  }

  void myPIT::ClearEntry(std::string name)
  {
    PITmap.erase(name);
  }

  bool myPIT::CheckEntry (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PITmap.find(name) == PITmap.end() ){
      return false;
    }
    return true;
  }

  bool myPIT::CheckForwardedAddr (std::string name, Ipv4Address addr)
  {
    NS_LOG_FUNCTION (this);
    if( PITmap.find(name) == PITmap.end() ){
      return false;
    }else if( PITmap[name]->forwarded_addr == addr ){
      return true;
    }else{
      return false;
    }
  }

  /// myContentStore
  myContentStore::myContentStore()
  {
    NS_LOG_FUNCTION (this);
    m_disable_double_cache = 0;
    m_cache_ttl = Seconds(0);
  }

  myContentStore::~myContentStore()
  {
  }

  myContentStore::CS_Entry::CS_Entry (std::string name, Time cache,
      Time gen, Time expire, std::string str_data)
  {
    NS_LOG_FUNCTION (this);
    content_name = name;
    cache_time = cache;
    gen_time = gen;
    expire_time = expire;
    string_data = str_data;
  }

  Ptr<Packet> myContentStore::GetDataPacket (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( CSmap.find(name) == CSmap.end() ){
      NS_LOG_WARN ("CS entry was not found");
      return NULL;
    }
    Ptr<CS_Entry> entry = CSmap[name];

    myPacket myPkt (myPacket::TYPE_NAME_DATA);
    myPkt.AddData (name, entry->gen_time,
        entry->expire_time, entry->string_data);
    myPkt.AddCacheFlag();
    return myPkt.GetPacket();
  }

  void myContentStore::SetCacheTTL (Time ttl)
  {
    m_cache_ttl = ttl;
  }

  void myContentStore::AddEntry (Ptr<myPacket::Data> data_ptr)
  {
    NS_LOG_FUNCTION (this);

    if( data_ptr->flag && m_disable_double_cache ){
      return;
    }
    std::string name = data_ptr->content_name;

    if( CSmap.find(name) == CSmap.end() ){
      CSmap[name] = Create<CS_Entry> (name, Simulator::Now(),
          data_ptr->gen_time, data_ptr->expire_time, data_ptr->string_data);
    }else{
      if( CSmap[name]->gen_time < data_ptr->gen_time ){
        CSmap[name] = Create<CS_Entry> (name, Simulator::Now(),
            data_ptr->gen_time, data_ptr->expire_time, data_ptr->string_data);
      }
    }
  }

  bool myContentStore::CheckEntry (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( CSmap.find(name) == CSmap.end() ){
      return false;
    }
    return true;
  }

  void myContentStore::CheckExpire()
  {
    NS_LOG_FUNCTION (this);
    Time current_time = Simulator::Now();

    /// イテレータの無効化を防ぐ妙技
    std::map<std::string, Ptr<myContentStore::CS_Entry> >::iterator  itr = CSmap.begin();
    while( itr != CSmap.end() ){
      Ptr<CS_Entry> entry = itr->second;
      if( current_time > entry->expire_time ){
        CSmap.erase(itr++);
      }else{
        cache_count++;
        ++itr;
      }
    }// exit for loop
  }

  /// myPBR
  myPBR::myPBR()
  {
    NS_LOG_FUNCTION (this);
    m_D = 0;
    m_rho = 0;
    m_hop_limit = 0;
    m_node_id = 0;
    m_mode = 0;
    m_gamma = 0;
  }

  void myPBR::Init (uint32_t node, double D, double rho, uint16_t hop, Time interval, Time ttl)
  {
    NS_LOG_FUNCTION (this);
    m_D = D;
    m_rho = rho;
    m_hop_limit = hop;
    m_node_id = node;
    m_alpha = rho;
    m_gamma = - rho / ( 1 + D * ttl.GetSeconds() / ( 6 * interval.GetSeconds() ) ) ;
    m_update_interval = interval;
  }

  void myPBR::SetMode (uint16_t mode)
  {
    NS_LOG_FUNCTION (this);
    m_mode = mode;
  }

  myPBR::PBR_Entry::PBR_Entry (std::string name, Ipv4Address addr, uint16_t hop, uint32_t node, Time expire)
  {
    NS_LOG_FUNCTION (this);
    content_name = name;
    next_addr = addr;
    hop_count = hop;
    node_num = node;
    expire_time = expire;
    potential_self = INITIAL_POTENTIAL;
    potential_next = INITIAL_POTENTIAL;
    tmp_pbr = NULL;
    tmp_addr = Ipv4Address::GetZero();
  }

  Ipv4Address myPBR::GetNextAddr (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PBRmap.find(name) == PBRmap.end() ){
      NS_LOG_WARN ("Content Store entry was not found");
      return Ipv4Address::GetZero();
    }
    return PBRmap[name]->next_addr;
  }

  void myPBR::AddEntry (Ptr<myPacket::PBR> pbr, Ipv4Address addr)
  {
    NS_LOG_FUNCTION (this);

    if( pbr->node_num == m_node_id ){
      return;
    }

    std::string name = pbr->content_name;

    if( PBRmap.find(name) == PBRmap.end() ){
      Ptr<PBR_Entry> entry = Create<PBR_Entry> (name, addr, pbr->hop_count, pbr->node_num, pbr->expire_time);
      entry->cache_ttl = pbr->cache_ttl;
      entry->potential_next = pbr->potential_value;
      entry->potential_self = InitPotential(entry);

      entry->tmp_pbr = pbr;
      entry->tmp_addr = addr;
      PBRmap[name] = entry;
    }else{
      Ptr<PBR_Entry> entry = PBRmap[name];
      if( entry->next_addr == Ipv4Address::GetLoopback() ){
        return;
      }else if( entry->tmp_pbr == NULL ){
        entry->tmp_pbr = pbr;
        entry->tmp_addr = addr;
      }else if( entry->tmp_pbr->potential_value > pbr->potential_value ){
        entry->tmp_pbr = pbr;
        entry->tmp_addr = addr;
      }
    }
  }

  void myPBR::AddEntry (std::string name, Time expire, Time ttl)
  {
    NS_LOG_FUNCTION (this);

    Ipv4Address addr = Ipv4Address::GetLoopback();

    if( PBRmap.find(name) == PBRmap.end() ){
      Ptr<PBR_Entry> entry = Create<PBR_Entry> (name, addr, 0, m_node_id, expire);
      entry->cache_ttl = ttl;
      entry->potential_next = INITIAL_POTENTIAL;
      entry->potential_self = InitPotential(entry);
      entry->tmp_pbr = NULL;
      entry->tmp_addr = Ipv4Address::GetZero();
      PBRmap[name] = entry;
    }else{
      if( PBRmap[name]->expire_time < expire ){
        Ptr<PBR_Entry> entry = Create<PBR_Entry> (name, addr, 0, m_node_id, expire);
        entry->cache_ttl = ttl;
        entry->potential_next = INITIAL_POTENTIAL;
        entry->potential_self = InitPotential(entry);
        entry->tmp_pbr = NULL;
        entry->tmp_addr = Ipv4Address::GetZero();
        PBRmap[name] = entry;
      }
    }
  }

  void myPBR::UpdateEntry()
  {
    NS_LOG_FUNCTION (this);
    // イテレータの無効化を防ぐ妙技
    std::map<std::string, Ptr<myPBR::PBR_Entry> >::iterator  itr = PBRmap.begin();
    while( itr != PBRmap.end() ){
      Ptr<myPBR::PBR_Entry> pbr = itr->second;

      pbr_count++;
      //  NS_LOG_INFO("Node:\t" << m_node_id << "\tPotential =\t" << pbr->potential_self
      //      << "\tPotential_Next =\t" << pbr->potential_next << "\tCacheNode:\t" << pbr->node_num );
      if( pbr->next_addr == Ipv4Address::GetLoopback() ){
        pbr->node_num = m_node_id;
        pbr->hop_count = 0;
      }else if( pbr->tmp_pbr == NULL ){
        pbr->potential_next = INITIAL_POTENTIAL;
        pbr->next_addr = Ipv4Address::GetZero();
      }else{
        pbr->potential_next = pbr->tmp_pbr->potential_value;
        if( pbr->potential_self - pbr->potential_next > m_alpha )
        {
          pbr->next_addr = pbr->tmp_addr;
          pbr->hop_count = pbr->tmp_pbr->hop_count;
          pbr->node_num = pbr->tmp_pbr->node_num;
          pbr->expire_time = pbr->tmp_pbr->expire_time;
          pbr->cache_ttl = pbr->tmp_pbr->cache_ttl;
        }else{
          pbr->next_addr = Ipv4Address::GetZero();
        }
      }
      pbr->potential_self = UpdatePotential(pbr);
      pbr->tmp_pbr = NULL;
      if( m_mode && pbr->potential_self > 0 ){
        std::string name = itr->first;
        PBRmap.erase(itr++);
      }else{
        itr++;
      }
    }
  }

  bool myPBR::CheckEntry (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PBRmap.find(name) == PBRmap.end() ){
      return false;
    }else
      return true;
  }

  void myPBR::CheckExpire()
  {
    NS_LOG_FUNCTION (this);
    Time current_time = Simulator::Now();
    /// イテレータの無効化を防ぐ妙技
    std::map<std::string, Ptr<myPBR::PBR_Entry> >::iterator itr = PBRmap.begin();
    while( itr != PBRmap.end() ){
      if( current_time > itr->second->expire_time - m_update_interval ){
        PBRmap.erase(itr++);
      }else{
        ++itr;
      }
    }
  }

  bool myPBR::isOverHopLimit(Ptr<myPacket::PBR> pbr)
  {
    NS_LOG_FUNCTION (this);
    uint16_t hop = pbr->hop_count;

    if( m_mode ){
      if( hop > m_hop_limit + 2 ){
        return true;
      }
      return false;
    }else{
      if( hop > m_hop_limit ){
        return true;
      }
      return false;
    }
  }

  bool myPBR::isOverHopLimit(Ptr<PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);
    uint16_t hop = pbr->hop_count;

    if( m_mode ){
      if( hop + 1 > m_hop_limit + 2){
        return true;
      }
      return false;
    }else{
      if( hop + 1 > m_hop_limit ){
        return true;
      }
      return false;
    }
  }

  Ptr<Packet> myPBR::GetPBRPacket (std::string name)
  {
    NS_LOG_FUNCTION (this);
    if( PBRmap.find(name) == PBRmap.end() ){
      NS_LOG_WARN ("PBR entry was not found");
      return NULL;
    }

    myPacket myPkt (myPacket::TYPE_NAME_PBR);
    myPkt.AddPBR (name, PBRmap[name]->potential_self, PBRmap[name]->hop_count + 1,
        PBRmap[name]->node_num, PBRmap[name]->expire_time);
    myPkt.AddCacheTTL ( PBRmap[name]->cache_ttl );
    return myPkt.GetPacket();
  }

  double myPBR::UpdatePotential (Ptr<myPBR::PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);

    double v;

    if( m_mode ){
      v = PotentialFormula2 (pbr);
    }else{
      v = PotentialFormula1 (pbr);
    }
    return v;
  }

  double myPBR::PotentialFormula1 (Ptr<myPBR::PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);
    if( pbr->next_addr == Ipv4Address::GetLoopback() ){
      return 0.0;
    }

    double v = pbr->potential_self;
    double min_v = pbr->potential_next;

    if( min_v < v ){
      v = pbr->potential_self
        +m_D * (pbr->potential_next - pbr->potential_self) + m_rho;
    }else{
      v = pbr->potential_self + m_rho;
    }
    return v;
  }

  double myPBR::PotentialFormula2 (Ptr<myPBR::PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);
    if( pbr->next_addr == Ipv4Address::GetLoopback() ){
      return GetCacheAwarePotential (pbr) ;
    }

    double v = pbr->potential_self;
    double min_v = pbr->potential_next;

    if( min_v < v ){
      v = pbr->potential_self
        +m_D * (pbr->potential_next - pbr->potential_self) + m_rho;
    }else{
      v = pbr->potential_self + m_rho;
    }
    return v;
  }

  double myPBR::GetCacheAwarePotential (Ptr<myPBR::PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);
    Time current_time = Simulator::Now();
    Time expire_time = pbr->expire_time;

    double v = m_gamma *( (expire_time - current_time).GetMilliSeconds() / m_update_interval.GetMilliSeconds());

    return v;
  }

  double myPBR::InitPotential (Ptr<myPBR::PBR_Entry> pbr)
  {
    NS_LOG_FUNCTION (this);
    if( pbr->next_addr == Ipv4Address::GetLoopback() ){
      if( m_mode ){
        return GetCacheAwarePotential (pbr);
      }else{
        return 0.0;
      }
    }

    double v = pbr->potential_next + ( m_rho / m_D );
    return v;
  }

} // Namespace ns3
