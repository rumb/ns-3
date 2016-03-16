#include <string>
#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/mobility-module.h"
#include "ns3/applications-module.h"

#include "ns3/config.h"
#include "ns3/my-app-helper.h"
#include "ns3/my-router-module.h"
#include "ns3/my-endnode-app.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("mySimulation");

int main (int argc, char *argv[], char *envp[])
{
  double simulation_time = 300.0; // 単位は分

  // シミュレーションの時間の最小単位設定
  Time::SetResolution (Time::MS);
  // ログ有効化
  LogComponentEnable ("myEndNodeApp", LOG_LEVEL_INFO);
  //LogComponentEnable ("myRouterApp", LOG_LEVEL_INFO);
  //LogComponentEnable ("myAppPacket", LOG_LEVEL_INFO);
  //LogComponentEnable ("myRouterModule", LOG_LEVEL_INFO);
  LogComponentEnable ("mySimulation", LOG_LEVEL_INFO);
  // DEBUG用の関数ログ
  //LogComponentEnable ("myRouterApp", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("myEndNodeApp", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("myRouterModule", LOG_LEVEL_FUNCTION);
  //LogComponentEnable ("myAppPacket", LOG_LEVEL_FUNCTION);

  uint64_t run = 1;
  double cache_ttl = 1;
  double pbr_interval = 1000; // sec
  double request_interval = 1; // min
  double d = 0.2;
  double rho = 0.02;
  double update_interval = 1000; // sec
  uint16_t hop_limit = 0;
  double cache_prob = 0.2;
  uint16_t pbr_mode = 0;
  uint16_t cache_mode = 0;

  // コマンドライン引数指定
  CommandLine cmd;
  cmd.AddValue ("Run", "advancing the substream state", run);
  cmd.AddValue ("CacheTTL", "no introduction", cache_ttl);
  cmd.AddValue ("PBRInterval", "no introduction", pbr_interval);
  cmd.AddValue ("RequestInterval", "no introduction", request_interval);
  cmd.AddValue ("D", "no introduction",  d);
  cmd.AddValue ("Rho", "no introduction", rho);
  cmd.AddValue ("UpdateInterval", "no introduction", update_interval);
  cmd.AddValue ("HopLimit", "no introduction", hop_limit);
  cmd.AddValue ("CacheProbability", "no introduction", cache_prob);
  cmd.AddValue ("PBRMode", "no introduction", pbr_mode);
  cmd.AddValue ("CacheAwareMode", "no introduction", cache_mode);
  cmd.Parse ( argc,argv );

  // 乱数生成
  RngSeedManager::SetSeed (1);  // Changes seed from default of 1 to 3
  RngSeedManager::SetRun (run);  // Changes run number from default of 1 to 7
  Ptr<UniformRandomVariable>  UniRand = CreateObject<UniformRandomVariable> ();
  UniRand->SetAttribute ("Min", DoubleValue (0.0));
  UniRand->SetAttribute ("Max", DoubleValue (1.0));

  int row_num = 10;
  int col_num = 1;

  // トポロジーの生成
  PointToPointHelper p2p;
  p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
  PointToPointGridHelper grid (row_num, col_num, p2p);
  grid.BoundingBox (100, 100, 200, 200);

  // インターネットのスタック定義
  InternetStackHelper stack;
  grid.InstallStack (stack);

  // ipアドレス付与
  Ipv4AddressHelper address_row;
  address_row.SetBase ("10.0.0.0", "255.255.255.252");
  Ipv4AddressHelper address_col;
  address_col.SetBase ("10.1.0.0", "255.255.255.252");
  grid.AssignIpv4Addresses (address_row, address_col);

  //ディレイ設定
  Config::Set ("/ChannelList/*/$ns3::PointToPointChannel/Delay", TimeValue( MilliSeconds(5) ) );

  // アプリケーション生成
  myRouterAppHelper myRouter (9);
  myRouter.SetAttribute ("RoutingInterval", TimeValue (Minutes(30.0)));
  myRouter.SetAttribute ("PBRInterval", TimeValue (Seconds(pbr_interval)));
  myRouter.SetAttribute ("HopLimit", UintegerValue (hop_limit));
  myRouter.SetAttribute ("D", DoubleValue (d));
  myRouter.SetAttribute ("Rho", DoubleValue (rho));
  myRouter.SetAttribute ("CacheTTL", TimeValue (Minutes(cache_ttl)));
  myRouter.SetAttribute ("CacheAwareMode", UintegerValue (cache_mode));

  myEndNodeAppHelper myServer (9);
  myServer.SetAttribute ("Role", UintegerValue (1));
  myServer.SetAttribute ("PBRMode", UintegerValue (pbr_mode));
  myServer.SetAttribute ("RoutingInterval", TimeValue (Minutes(60)));
  myServer.SetAttribute ("PBRInterval", TimeValue (Seconds(pbr_interval)));
  myServer.SetAttribute ("UpdateInterval", TimeValue (Seconds (update_interval)));
  myServer.SetAttribute ("CacheTTL", TimeValue (Minutes(cache_ttl)));
  myServer.SetAttribute ("OriginalContentPotential", DoubleValue (cache_prob));
  myServer.SetAttribute ("MaxPackets", UintegerValue (100000));

  myEndNodeAppHelper myClient (9);
  myClient.SetAttribute ("Role", UintegerValue (0));
  myClient.SetAttribute ("RequestInterval", TimeValue (Minutes (request_interval)));
  myClient.SetAttribute ("MaxPackets", UintegerValue (100000));

  // アプリケーションインストール
  ApplicationContainer myServerApps;
  ApplicationContainer myRouterApps;
  ApplicationContainer myClientApps;

  double n = col_num * row_num;
  int flag = 1;

  // アプリケーションインストール
  for(int j = 0; j < row_num; j++){
    for(int i = 0; i < col_num; i++){

      if( flag && (UniRand->GetValue() < (1.0/n)) ){
        myRouter.SetAttribute ("CacheProbability", DoubleValue (0));
        myRouterApps.Add (myRouter.Install (grid.GetNode(j,i)) );
        myServerApps = myServer.Install (grid.GetNode(j,i));
        flag = 0;
        NS_LOG_INFO ("Server Node  \tAS:" << j << "\tNode:" << i);

      }else if( UniRand->GetValue() < 0.2 ){
        myRouter.SetAttribute ("CacheProbability", DoubleValue (cache_prob));
        myRouterApps.Add (myRouter.Install (grid.GetNode(j,i)));
        myClient.SetAttribute ("RequestInterval", TimeValue (Minutes(request_interval)));
        myClientApps.Add (myClient.Install (grid.GetNode (j,i)));
        NS_LOG_INFO ("Client Node (20par)  \tAS:" << j << "\tNode:" << i);

      }else{
        myRouter.SetAttribute ("CacheProbability", DoubleValue (cache_prob));
        myRouterApps.Add (myRouter.Install (grid.GetNode(j,i)));
        myClient.SetAttribute ("RequestInterval", TimeValue (Minutes (16.0 * request_interval)));
        myClientApps.Add (myClient.Install (grid.GetNode (j,i)));
        NS_LOG_INFO ("Client Node (80par)  \tAS:" << j << "\tNode:" << i);

      }
      n--;
    }
  }

  // アプリケーション起動時間指定
  myRouterApps.Start (Seconds (0.1));
  myClientApps.Start (Seconds (10.0));
  myServerApps.Start (Seconds (0.1));

  myRouterApps.Stop (Minutes (simulation_time));
  myClientApps.Stop (Minutes (simulation_time));
  myServerApps.Stop (Minutes (simulation_time));

  // シミュレーション実行
  Simulator::Stop (Minutes(simulation_time));
  Simulator::Run ();
  NS_LOG_INFO("[Packets]\t" << myContentStore::cache_count);
  NS_LOG_INFO("[PBR]\t" << myPBR::pbr_count);
  NS_LOG_INFO("[Send]\t" << myEndNodeApp::send_count);
  Simulator::Destroy ();

  return 0;
}
