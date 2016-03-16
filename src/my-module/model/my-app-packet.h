#ifndef my_APP_PACKET_H
#define my_APP_PACKET_H

#include "ns3/ptr.h"
#include "ns3/nstime.h"

#include <vector>

namespace ns3 {

  class Packet;

  class myPacket : public SimpleRefCount<myPacket>
  {
    /// split関数の区切り文字
    static const char* DELIMITER;

    public:
    /// 時間の単位"ms"
    static const char* TIME_UNIT;
    /// パケットのタイプの名前
    static const char* TYPE_NAME_INTEREST;
    static const char* TYPE_NAME_DATA;
    static const char* TYPE_NAME_ROUTING;
    static const char* TYPE_NAME_PBR;
    static const char* TYPE_NAME_UNKNOWN;
    static const uint16_t ORIGINAL_NODE_ID;

    /**
     * myPacketのインナークラス
     * パケットの中身を定義
     * 構造体のように使いたい為，
     * 機能は最小限にして，必要な処理はアウタークラスで行う
     */
    class Interest : public SimpleRefCount<Interest>
    {
      public:
        std::string content_name;
        uint16_t flag;
        Interest (std::vector<std::string>::iterator itr);
        Interest (std::string name);
        std::string toString();
    };

    class Data : public SimpleRefCount<Data>
    {
      public:
        std::string content_name;
        Time gen_time;
        Time expire_time;
        std::string string_data;
        uint16_t flag;
        Data (std::vector<std::string>::iterator itr);
        Data (std::string name, Time gen, Time expire, std::string data_str);
        std::string toString();
    };

    class Routing : public SimpleRefCount<Routing>
    {
      public:
        std::string content_name;
        int32_t serial_num;
        Routing (std::vector<std::string>::iterator itr);
        Routing (std::string name, int32_t serial);
        std::string toString();
    };

    class PBR : public SimpleRefCount<PBR>
    {
      public:
        std::string content_name;
        double potential_value;
        uint16_t hop_count;
        uint32_t node_num;
        Time expire_time;
        Time cache_ttl;
        PBR (std::vector<std::string>::iterator itr);
        PBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire);
        PBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire, Time ttl);
        std::string toString();
    };

    /// コンストラクタ
    myPacket (Ptr<Packet> packet);
    myPacket (std::string type_name);
    /// コピーコンストラクタ
    //myPacket (const myPacket &o);

    std::string GetType();
    Ptr<Interest> GetInterest();
    Ptr<Data> GetData();
    Ptr<Routing> GetRouting();
    Ptr<PBR> GetPBR();

    bool AddInterest (std::string name);
    bool AddData (std::string name, Time gen, Time expire, std::string data_str);
    bool AddData (Ptr<Data> data_ptr);
    void AddCacheFlag ();
    void AddPBRFlag ();
    bool AddRouting (std::string name, int32_t serial);
    void AddCacheTTL (Time ttl);
    bool AddPBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire);
    bool AddPBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire, Time ttl);
    Ptr<Packet> GetPacket();

    private:
    /// パケットのタイプ
    std::string m_type;
    /// split関数
    std::vector<std::string> split (const std::string &str);

    Ptr<Packet> m_packet;

    Ptr<Interest> m_Interest;
    Ptr<Data> m_Data;
    Ptr<Routing> m_Routing;
    Ptr<PBR> m_PBR;
  };

} // namespace ns3

#endif /* my_APP_PACKET_H  */
