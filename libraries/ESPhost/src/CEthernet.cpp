#include "CEthernet.h"

/*
 * The old implementation of the begin set a default mac address:
 * this does not make any sense.
 * Default mac address is in the hardware, when lwip start that mac
 * address is passed to lwip
 * If mac address needs to be changed then call the appropriate function
 * of lwIpIf before to get the interface 
 */

/* -------------------------------------------------------------------------- */
int CEthernet::begin(unsigned long timeout, unsigned long responseTimeout) {
/* -------------------------------------------------------------------------- */  
  int rv = 0;

  ni = CLwipIf::getInstance().get(NI_ETHERNET);
  if(ni != nullptr) {
    rv = (int)ni->DhcpStart();
  }
  
  return rv;
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(IPAddress local_ip) {
/* -------------------------------------------------------------------------- */
  IPAddress subnet(255, 255, 255, 0);
  begin(local_ip, subnet);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(IPAddress local_ip, IPAddress subnet) {
/* -------------------------------------------------------------------------- */  
  // Assume the gateway will be the machine on the same network as the local IP
  // but with last octet being '1'
  IPAddress gateway = local_ip;
  gateway[3] = 1;
  begin(local_ip, subnet, gateway);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(IPAddress local_ip, IPAddress subnet, IPAddress gateway) {
/* -------------------------------------------------------------------------- */  
  // Assume the DNS server will be the same machine than gateway
  begin(local_ip, subnet, gateway, gateway);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(IPAddress local_ip, IPAddress subnet, IPAddress gateway, IPAddress dns_server) {
/* -------------------------------------------------------------------------- */  
  
  ni = CLwipIf::getInstance().get(NI_ETHERNET,  local_ip.raw_address(), gateway.raw_address(), subnet.raw_address());
  if(ni != nullptr) {
    /* If there is a local DHCP informs it of our manual IP configuration to prevent IP conflict */
    ni->DhcpNotUsed();
  }
  CLwipIf::getInstance().addDns(dns_server);
}

/* -------------------------------------------------------------------------- */
int CEthernet::begin(uint8_t *mac_address, unsigned long timeout, unsigned long responseTimeout) {
/* -------------------------------------------------------------------------- */  
  int ret = (int)CLwipIf::getInstance().setMacAddress(NI_ETHERNET, mac_address);
  begin(timeout, responseTimeout);
  return ret;
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(uint8_t *mac_address, IPAddress local_ip) {
/* -------------------------------------------------------------------------- */
  // Assume the DNS server will be the machine on the same network as the local IP
  // but with last octet being '1'
  IPAddress dns_server = local_ip;
  dns_server[3] = 1;
  begin(mac_address, local_ip, dns_server);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server) {
/* -------------------------------------------------------------------------- */  
  // Assume the gateway will be the machine on the same network as the local IP
  // but with last octet being '1'
  IPAddress gateway = local_ip;
  gateway[3] = 1;
  begin(mac_address, local_ip, dns_server, gateway);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(uint8_t *mac_address, IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
/* -------------------------------------------------------------------------- */  
  IPAddress subnet(255, 255, 255, 0);
  begin(mac_address, local_ip, dns_server, gateway, subnet);
}

/* -------------------------------------------------------------------------- */
void CEthernet::begin(uint8_t *mac, IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
/* -------------------------------------------------------------------------- */  
  CLwipIf::getInstance().setMacAddress(NI_ETHERNET, mac_address);
  begin(local_ip, subnet, gateway, dns_server);
}

/* -------------------------------------------------------------------------- */
EthernetLinkStatus CEthernet::linkStatus() {
/* -------------------------------------------------------------------------- */  
  //return (!is_eth0_initialized()) ? Unknown : (is_eth0_link_up() ? LinkON : LinkOFF);
}

/* -------------------------------------------------------------------------- */
int CEthernet::maintain() {
/* -------------------------------------------------------------------------- */  
  int rc = DHCP_CHECK_NONE;

  if (ni != NULL) {
    //we have a pointer to dhcp, use it
    rc = ni->checkLease();
    switch (rc) {
      case DHCP_CHECK_NONE:
        //nothing done
        break;
      case DHCP_CHECK_RENEW_OK:
      case DHCP_CHECK_REBIND_OK:
        //_dnsServerAddress = _dhcp->getDnsServerIp();
        break;
      default:
        //this is actually a error, it will retry though
        break;
    }
  }
  return rc;
}

/*
 * This function updates the LwIP stack and can be called to be sure to update
 * the stack (e.g. in case of a long loop).
 */
void CEthernet::schedule(void) {
  if (ni != NULL) {
    ni->task();
  }
}



uint8_t *CEthernet::MACAddress(void) {
  //return mac_address;
}

IPAddress CEthernet::localIP() {
  if(ni != nullptr) {
      return IPAddress(ni->getIpAdd());   
   }
   return IPAddress((uint32_t)0);
}

IPAddress CEthernet::subnetMask() {
  if(ni != nullptr) {
      return IPAddress(ni->getNmAdd());   
   }
   return IPAddress((uint32_t)0);
}

IPAddress CEthernet::gatewayIP() {
  if(ni != nullptr) {
      return IPAddress(ni->getGwAdd());   
   }
   return IPAddress((uint32_t)0);
}

IPAddress CEthernet::dnsServerIP() {
  return CLwipIf::getInstance().getDns();
}

CEthernet Ethernet;
