#ifndef my_ROUTER_MODULE_H
#define my_ROUTER_MODULE_H

#include "ns3/ptr.h"
#include "ns3/ipv4.h"
#include "ns3/nstime.h"
#include <map>
#include <vector>

#include "ns3/my-app-packet.h"

namespace ns3 {

  class Packet;

  class myFIB
  {
    public:
    class FIB_Entry : public SimpleRefCount<FIB_Entry>
    {
      std::string content_name;
      public:
      Ipv4Address next_addr;
      uint32_t serial_num;
      FIB_Entry (std::string name, Ipv4Address addr, uint32_t serial);
    };

    myFIB();
    std::map<std::string, Ptr<FIB_Entry> > FIBmap;
    Ipv4Address GetNextAddr (std::string name);
    void AddEntry (std::string name, Ipv4Address addr, uint32_t serial);
    Ptr<Packet> GetRoutingPacket (std::string name);
  };

  class myPIT
  {
    public:
    class PIT_Entry : public SimpleRefCount<PIT_Entry>
    {
      std::string content_name;
      public:
      Ipv4Address forwarded_addr;
      Time forwarded_time;
      std::vector<Ipv4Address> next_addr;
      PIT_Entry(std::string name, Ipv4Address addr);
    };

    myPIT();
    std::map<std::string, Ptr<PIT_Entry> > PITmap;
    std::vector<Ipv4Address>* GetNextAddrs (std::string name);
    void ClearEntry (std::string name);
    void SetForwardedAddr (std::string name, Ipv4Address addr);
    void AddEntry (std::string name, Ipv4Address addr);
    bool CheckEntry (std::string name);
    bool CheckExpire (std::string name);
    bool CheckForwardedAddr (std::string name, Ipv4Address addr);
  };

  class myContentStore
  {
    public:
    class CS_Entry : public SimpleRefCount<CS_Entry>
    {
      std::string content_name;
      public:
      Time cache_time;
      Time gen_time;
      Time expire_time;
      std::string string_data;
      CS_Entry(std::string name, Time cache, Time gen, Time expire, std::string str_data);
    };

    static uint64_t cache_count;
    Time m_cache_ttl;
    myContentStore();
    ~myContentStore();
    uint16_t m_disable_double_cache;
    std::map<std::string, Ptr<CS_Entry> > CSmap;
    Ptr<Packet> GetDataPacket (std::string name);
    void AddEntry (Ptr<myPacket::Data> data_ptr);
    void SetCacheTTL (Time ttl);
    bool CheckEntry (std::string name);
    void CheckExpire();
  };

  class myPBR
  {
    public:
    class PBR_Entry : public SimpleRefCount<PBR_Entry>
    {
      std::string content_name;
      public:
      double potential_self;
      double potential_next;
      Ipv4Address next_addr;
      uint16_t hop_count;
      uint32_t node_num;
      Time expire_time;
      Time cache_ttl;

      Ptr<myPacket::PBR> tmp_pbr;
      Ipv4Address tmp_addr;

      PBR_Entry(std::string name, Ipv4Address addr, uint16_t hop, uint32_t node, Time expire);
    };

    static const double INITIAL_POTENTIAL;
    static uint64_t pbr_count;
    myPBR();
    std::map<std::string, Ptr<PBR_Entry> > PBRmap;
    void Init (uint32_t node, double D, double rho, uint16_t hop, Time update_interval, Time ttl);
    void SetMode (uint16_t mode);
    Ipv4Address GetNextAddr (std::string name);
    void AddEntry (Ptr<myPacket::PBR> pbr_ptr, Ipv4Address addr);
    void AddEntry (std::string name, Time expire, Time ttl);
    bool CheckEntry (std::string name);
    void CheckExpire();
    bool isOverHopLimit (Ptr<myPacket::PBR> pbr);
    bool isOverHopLimit (Ptr<PBR_Entry> pbr);
    void UpdateEntry();
    double UpdatePotential (Ptr<PBR_Entry> pbr_ptr);
    double PotentialFormula1 (Ptr<PBR_Entry> pbr_ptr);
    double PotentialFormula2 (Ptr<PBR_Entry> pbr_ptr);
    double GetCacheAwarePotential (Ptr<PBR_Entry> pbr_ptr);
    double InitPotential (Ptr<PBR_Entry> pbr_ptr);
    Ptr<Packet> GetPBRPacket (std::string name);

    private:
    /// PBRの変数
    double m_D;
    double m_rho;
    uint16_t m_hop_limit;
    double m_alpha;
    double m_gamma;
    /// 自分のノードのID
    uint32_t m_node_id;
    uint16_t m_mode;
    Time m_update_interval;
  };

} // namespace ns3

#endif
