#include "my-app-packet.h"

#include "ns3/log.h"
#include "ns3/simulator.h"
#include "ns3/packet.h"

NS_LOG_COMPONENT_DEFINE ("myAppPacket");

namespace ns3 {

  /// static const 変数初期化
  const char* myPacket::DELIMITER = "\t";
  const char* myPacket::TIME_UNIT = "ms";
  const char* myPacket::TYPE_NAME_INTEREST = "Interest";
  const char* myPacket::TYPE_NAME_DATA = "Data";
  const char* myPacket::TYPE_NAME_ROUTING = "Routing";
  const char* myPacket::TYPE_NAME_PBR = "PBR";
  const char* myPacket::TYPE_NAME_UNKNOWN = "UNKNOWN";
  const uint16_t myPacket::ORIGINAL_NODE_ID = 10000;

  /// コンストラクタ
  myPacket::myPacket (Ptr<Packet> packet)
  {
    NS_LOG_FUNCTION (this);

    m_packet = packet;
    m_Interest = NULL;
    m_Data = NULL;
    m_Routing = NULL;
    m_PBR = NULL;

    packet->RemoveAllPacketTags();
    packet->RemoveAllByteTags();

    uint32_t buf_size = packet->GetSize();
    uint8_t buf[buf_size];
    memset (buf, 0, buf_size);
    packet->CopyData (buf,buf_size);


    std::vector<std::string> field = split ((char*)buf);
    std::vector<std::string>::iterator itr = field.begin();
    m_type = *itr;

    NS_LOG_INFO (Simulator::Now() << "\t" << buf); // DEBUG
    if( m_type == TYPE_NAME_INTEREST ){
      m_Interest = Create<Interest> (itr);
    }else if( m_type == TYPE_NAME_DATA ){
      m_Data = Create<Data> (itr);
    }else if( m_type == TYPE_NAME_ROUTING ){
      m_Routing = Create<Routing> (itr);
    }else if( m_type == TYPE_NAME_PBR ){
      m_PBR = Create<PBR> (itr);
    }else{
      NS_LOG_WARN ("Unknown type name was found [" << m_type << "]");
      m_type = TYPE_NAME_UNKNOWN;
    }
  }

  myPacket::myPacket (std::string type_name)
  {
    NS_LOG_FUNCTION (this);

    m_type = type_name;

    m_packet = NULL;
    m_Interest = NULL;
    m_Data = NULL;
    m_Routing = NULL;
    m_PBR = NULL;
  }

  bool myPacket::AddInterest (std::string name){
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_INTEREST ){
      m_Interest = Create<Interest> (name);
      return true;
    }
    NS_LOG_WARN ("Can't use \"AddInterest()\". This packet type is " << m_type);
    return false;
  }

  void myPacket::AddPBRFlag ()
  {
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_INTEREST ){
      m_Interest->flag = 1;
      return;
    }
      NS_LOG_WARN ("Can't use \"AddCacheFlag()\". This packet type is " << m_type);
  }

  bool myPacket::AddData (std::string name, Time gen, Time expire, std::string data_str){
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_DATA ){
      m_Data = Create<Data> (name, gen, expire, data_str);
      return true;
    }
    NS_LOG_WARN ("Can't use \"AddData()\". This packet type is " << m_type);
    return false;
  }

  bool myPacket::AddData (Ptr<Data> data_ptr)
  {
    NS_LOG_FUNCTION (this);
    m_Data = data_ptr;
  }

  void myPacket::AddCacheFlag ()
  {
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_DATA ){
      m_Data->flag = 1;
      return;
    }
      NS_LOG_WARN ("Can't use \"AddCacheFlag()\". This packet type is " << m_type);
  }

  bool myPacket::AddRouting (std::string name, int32_t serial){
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_ROUTING ){
      m_Routing = Create<Routing> (name, serial);
      return true;
    }
    NS_LOG_WARN ("Can't use \"AddRouting()\". This packet type is " << m_type);
    return false;
  }

  void myPacket::AddCacheTTL (Time ttl)
  {
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_PBR ){
      m_PBR->cache_ttl = ttl;
      return;
    }
      NS_LOG_WARN ("Can't use \"AddCacheTTL()\". This packet type is " << m_type);
  }

  bool myPacket::AddPBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire)
  {
    NS_LOG_FUNCTION (this);
    if( m_type == TYPE_NAME_PBR ){
      m_PBR = Create<PBR> (name, potential, hop, node, expire);
      return true;
    }
    NS_LOG_WARN ("Can't use \"AddPBR()\". This packet type is " << m_type);
    return false;
  }


  Ptr<myPacket::Interest> myPacket::GetInterest()
  {
    return m_Interest;
  }

  Ptr<myPacket::Data> myPacket::GetData()
  {

    return m_Data;
  }

  Ptr<myPacket::Routing> myPacket::GetRouting()
  {
    return m_Routing;
  }

  Ptr<myPacket::PBR> myPacket::GetPBR()
  {
    return m_PBR;
  }

  //// コピーコンストラクタ
  //myPacket::myPacket (const myPacket &o)
  //{
  //}

  /// パケット作成
  Ptr<Packet> myPacket::GetPacket()
  {
    NS_LOG_FUNCTION (this);

    if(m_packet == NULL){

      std::string packet_str;
      packet_str = m_type;
      packet_str.append (DELIMITER);

      if( m_type == TYPE_NAME_INTEREST ){
        packet_str += m_Interest->toString();
      }else if( m_type == TYPE_NAME_DATA ){
        packet_str += m_Data->toString();
      }else if( m_type == TYPE_NAME_ROUTING ){
        packet_str += m_Routing->toString();
      }else if( m_type == TYPE_NAME_PBR ){
        packet_str += m_PBR->toString();
      }else{
        NS_LOG_WARN ("Can't execute \"GetPacket()\". Wrong type name [" << m_type << "].");
        return NULL;
      }

      uint32_t buf_size = packet_str.length() + 1;
      uint8_t buf[buf_size];

      memcpy (buf, packet_str.c_str(), buf_size);

      m_packet = Create<Packet> (buf, buf_size);
    }
    return m_packet;
  }

  std::string myPacket::GetType()
  {
    NS_LOG_FUNCTION (this);
    return m_type;
  }

  /// split関数
  std::vector<std::string> myPacket::split (const std::string &str)
  {
    NS_LOG_FUNCTION (this);
    std::vector<std::string> res;
    size_t current = 0, found;
    while( (found = str.find_first_of (DELIMITER, current)) != std::string::npos ){
      res.push_back (std::string (str, current, found - current));
      current = found + 1;
    }
    res.push_back (std::string (str, current, str.size() - current));
    return res;
  }


  /**
   * myPacketのインナークラス
   * パケットの中身を定義
   * 構造体のように使いたい為，
   * 機能は最小限にして，必要な処理はアウタークラスで行う
   */
  myPacket::Interest::Interest (std::vector<std::string>::iterator itr)
  {
    NS_LOG_FUNCTION (this);
    /// 第1フィールド
    content_name = *(++itr);
    /// 第2フィールド
    std::istringstream flag_is(*(++itr));
    flag_is >> flag;
  }

  myPacket::Interest::Interest (std::string name)
  {
    NS_LOG_FUNCTION (this);
    /// 第1フィールド
    content_name = name;
    /// 第2フィールド
    flag = 0;
  }

  std::string myPacket::Interest::toString()
  {
    NS_LOG_FUNCTION (this);
    std::string packet_str;
    /// 第1フィールド
    packet_str = content_name;
    packet_str.append(DELIMITER);
    // 第2フィールド
    std::ostringstream flag_os;
    flag_os << flag;
    packet_str += flag_os.str();

    return packet_str;
  }

  myPacket::Data::Data (std::vector<std::string>::iterator itr)
  {
    NS_LOG_FUNCTION (this);
    /// 第1フィールド
    content_name = *(++itr);
    /// 第2フィールド
    gen_time = Time ((++itr)->append(TIME_UNIT));
    /// 第3フィールド
    expire_time = Time ((++itr)->append(TIME_UNIT));
    /// 第4フィールド
    string_data = *(++itr);
    /// 第5フィールド
    std::istringstream flag_is(*(++itr));
    flag_is >> flag;
  }

  myPacket::Data::Data (std::string name, Time gen, Time expire, std::string data_str)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = name;
    // 第2フィールド
    gen_time = gen;
    // 第3フィールド
    expire_time = expire;
    // 第4フィールド
    string_data = data_str;
    // 第5フィールド
    flag = 0;
  }

  std::string myPacket::Data::toString()
  {
    NS_LOG_FUNCTION (this);
    std::string packet_str;
    // 第1フィールド
    packet_str = content_name;
    packet_str.append(DELIMITER);
    // 第2フィールド
    std::ostringstream gen_os;
    gen_os << gen_time.GetMilliSeconds();
    packet_str += gen_os.str();
    packet_str.append(DELIMITER);
    // 第3フィールド
    std::ostringstream expire_os;
    expire_os << expire_time.GetMilliSeconds();
    packet_str += expire_os.str();
    packet_str.append(DELIMITER);
    // 第4フィールド
    packet_str += string_data;
    packet_str.append(DELIMITER);
    // 第5フィールド
    std::ostringstream flag_os;
    flag_os << flag;
    packet_str += flag_os.str();

    return packet_str;
  }

  myPacket::Routing::Routing (std::vector<std::string>::iterator itr)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = *(++itr);
    // 第2フィールド
    std::istringstream serial_is(*(++itr));
    serial_is >> serial_num;
  }

  myPacket::Routing::Routing (std::string name, int32_t serial)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = name;
    // 第2フィールド
    serial_num = serial;
  }

  std::string myPacket::Routing::toString()
  {
    NS_LOG_FUNCTION (this);
    std::string packet_str;
    // 第1フィールド
    packet_str = content_name;
    packet_str.append(DELIMITER);
    // 第2フィールド
    std::ostringstream serial_str;
    serial_str << serial_num;
    packet_str += serial_str.str();

    return packet_str;
  }

  myPacket::PBR::PBR (std::vector<std::string>::iterator itr)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = *(++itr);
    // 第2フィールド
    std::istringstream potential_is (*(++itr));
    potential_is >> potential_value;
    // 第3フィールド
    std::istringstream hop_is (*(++itr));
    hop_is >> hop_count;
    // 第4フィールド
    std::istringstream node_is (*(++itr));
    node_is >> node_num;
    // 第5フィールド
    expire_time = Time ((++itr)->append(TIME_UNIT));
    // 第5フィールド
    cache_ttl = Time ((++itr)->append(TIME_UNIT));
  }

  myPacket::PBR::PBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = name;
    // 第2フィールド
    potential_value = potential;
    // 第3フィールド
    hop_count = hop;
    // 第4フィールド
    node_num = node;
    // 第5フィールド
    expire_time = expire;
  }

  myPacket::PBR::PBR (std::string name, double potential, uint16_t hop, uint32_t node, Time expire, Time ttl)
  {
    NS_LOG_FUNCTION (this);
    // 第1フィールド
    content_name = name;
    // 第2フィールド
    potential_value = potential;
    // 第3フィールド
    hop_count = hop;
    // 第4フィールド
    node_num = node;
    // 第5フィールド
    expire_time = expire;
    // 第6フィールド
    cache_ttl = ttl;
  }

  std::string myPacket::PBR::toString()
  {
    NS_LOG_FUNCTION (this);
    std::string packet_str;
    // 第1フィールド
    packet_str = content_name;
    packet_str.append(DELIMITER);
    // 第2フィールド
    std::ostringstream potential_os;
    potential_os << potential_value;
    packet_str += potential_os.str();
    packet_str.append(DELIMITER);
    // 第3フィールド
    std::ostringstream hop_os;
    hop_os << hop_count;
    packet_str += hop_os.str();
    packet_str.append(DELIMITER);
    // 第4フィールド
    std::ostringstream node_os;
    node_os << node_num;
    packet_str += node_os.str();
    packet_str.append(DELIMITER);
    // 第5フィールド
    std::ostringstream expire_os;
    expire_os << expire_time.GetMilliSeconds();
    packet_str += expire_os.str();
    packet_str.append(DELIMITER);
    // 第6フィールド
    std::ostringstream ttl_os;
    ttl_os << cache_ttl.GetMilliSeconds();
    packet_str += ttl_os.str();

    return packet_str;
  }

} // namespace ns3
