#include "my-app-helper.h"
#include "ns3/my-router-app.h"
#include "ns3/my-endnode-app.h"
#include "ns3/uinteger.h"
#include "ns3/names.h"

namespace ns3 {

  myRouterAppHelper::myRouterAppHelper (uint16_t port)
  {
    m_factory.SetTypeId (myRouterApp::GetTypeId ());
    SetAttribute ("Port", UintegerValue (port));
  }

  void myRouterAppHelper::SetAttribute (
      std::string name,
      const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }

  ApplicationContainer myRouterAppHelper::Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer myRouterAppHelper::Install (std::string nodeName) const
  {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer myRouterAppHelper::Install (NodeContainer c) const
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

    return apps;
  }

  Ptr<Application> myRouterAppHelper::InstallPriv (Ptr<Node> node) const
  {
    Ptr<Application> app = m_factory.Create<myRouterApp> ();
    node->AddApplication (app);

    return app;
  }

  myEndNodeAppHelper::myEndNodeAppHelper (Ipv4Address address, uint16_t port)
  {
    m_factory.SetTypeId (myEndNodeApp::GetTypeId ());
    SetAttribute ("RemoteAddress", Ipv4AddressValue (address));
    SetAttribute ("RemotePort", UintegerValue (port));
  }

  myEndNodeAppHelper::myEndNodeAppHelper (uint16_t port)
  {
    m_factory.SetTypeId (myEndNodeApp::GetTypeId ());
    SetAttribute ("RemotePort", UintegerValue (port));
  }

  void  myEndNodeAppHelper::SetAttribute (
      std::string name,
      const AttributeValue &value)
  {
    m_factory.Set (name, value);
  }

  ApplicationContainer myEndNodeAppHelper::Install (Ptr<Node> node) const
  {
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer myEndNodeAppHelper::Install (std::string nodeName) const
  {
    Ptr<Node> node = Names::Find<Node> (nodeName);
    return ApplicationContainer (InstallPriv (node));
  }

  ApplicationContainer myEndNodeAppHelper::Install (NodeContainer c) const
  {
    ApplicationContainer apps;
    for (NodeContainer::Iterator i = c.Begin (); i != c.End (); ++i)
    {
      apps.Add (InstallPriv (*i));
    }

    return apps;
  }

  Ptr<Application> myEndNodeAppHelper::InstallPriv (Ptr<Node> node) const
  {
    Ptr<Application> app = m_factory.Create<myEndNodeApp> ();
    node->AddApplication (app);

    return app;
  }

} // namespace ns3
