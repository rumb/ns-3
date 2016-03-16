#ifndef my_APP_HELPER_H
#define my_APP_HELPER_H

#include "ns3/application-container.h"
#include "ns3/node-container.h"
#include "ns3/object-factory.h"
#include "ns3/ipv4-address.h"

namespace ns3 {

  class myRouterAppHelper
  {
    public:
      myRouterAppHelper (uint16_t port);
      void SetAttribute (std::string name, const AttributeValue &value);
      ApplicationContainer Install (Ptr<Node> node) const;
      ApplicationContainer Install (std::string nodeName) const;
      ApplicationContainer Install (NodeContainer c) const;

    private:
      Ptr<Application> InstallPriv (Ptr<Node> node) const;
      ObjectFactory m_factory; //!< Object factory.
  };

  class myEndNodeAppHelper
  {
    public:
      myEndNodeAppHelper (uint16_t port);
      myEndNodeAppHelper (Ipv4Address ip, uint16_t port);
      void SetAttribute (std::string name, const AttributeValue &value);
      ApplicationContainer Install (Ptr<Node> node) const;
      ApplicationContainer Install (std::string nodeName) const;
      ApplicationContainer Install (NodeContainer c) const;

    private:
      Ptr<Application> InstallPriv (Ptr<Node> node) const;
      ObjectFactory m_factory; //!< Object factory.
  };

} // namespace ns3

#endif /* my_APP_HELPER_H */
