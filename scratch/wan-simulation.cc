#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/internet-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/applications-module.h"
#include "ns3/netanim-module.h"

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("WanCommunitySimulation");

int main (int argc, char *argv[])
{
    Time::SetResolution (Time::NS);
    LogComponentEnable ("UdpEchoClientApplication", LOG_LEVEL_INFO);
    LogComponentEnable ("UdpEchoServerApplication", LOG_LEVEL_INFO);

    // Create nodes
    NodeContainer router;
    router.Create (1);

    NodeContainer remoteNodes;
    remoteNodes.Create (4);

    // Internet stack
    InternetStackHelper stack;
    stack.Install (router);
    stack.Install (remoteNodes);

    // Point-to-Point WAN links
    PointToPointHelper p2p;
    p2p.SetDeviceAttribute ("DataRate", StringValue ("5Mbps"));
    p2p.SetChannelAttribute ("Delay", StringValue ("10ms"));

    Ipv4AddressHelper address;

    for (uint32_t i = 0; i < remoteNodes.GetN (); ++i)
    {
        NodeContainer pair (router.Get (0), remoteNodes.Get (i));
        NetDeviceContainer devices = p2p.Install (pair);

        std::ostringstream subnet;
        subnet << "10.1." << i+1 << ".0";

        address.SetBase (Ipv4Address (subnet.str ().c_str ()), "255.255.255.0");
        address.Assign (devices);
    }

    // UDP Echo Server on router
    UdpEchoServerHelper echoServer (9);
    ApplicationContainer serverApps = echoServer.Install (router.Get (0));
    serverApps.Start (Seconds (1.0));
    serverApps.Stop (Seconds (10.0));

    // UDP Echo Client on one remote node
    UdpEchoClientHelper echoClient (
        Ipv4Address ("10.1.1.1"), 9);
    echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
    echoClient.SetAttribute ("Interval", TimeValue (Seconds (1.0)));
    echoClient.SetAttribute ("PacketSize", UintegerValue (1024));

    ApplicationContainer clientApps = echoClient.Install (remoteNodes.Get (0));
    clientApps.Start (Seconds (2.0));
    clientApps.Stop (Seconds (10.0));

    // NetAnim
    AnimationInterface anim ("wan-simulation.xml");

    Simulator::Run ();
    Simulator::Destroy ();

    return 0;
}
