#include "WiFi.h"


/* -------------------------------------------------------------------------- */
CWifi::CWifi() : _timeout(50000){
}
/* -------------------------------------------------------------------------- */

/* -------------------------------------------------------------------------- */
const char* CWifi::firmwareVersion() {
/* -------------------------------------------------------------------------- */
   return WIFI_FIRMWARE_LATEST_VERSION;
}


/* -------------------------------------------------------------------------- */
int CWifi::begin(const char* ssid) {
/* -------------------------------------------------------------------------- */

   return 0;
}

/* -------------------------------------------------------------------------- */
int CWifi::begin(const char* ssid, const char *passphrase) {
/* -------------------------------------------------------------------------- */
   string res = "";
   modem.begin();
   modem.write(string(PROMPT(_MODE)),res, "%s%d\r\n" , CMD_WRITE(_MODE), 1);
   

   if(!modem.write(string(PROMPT(_BEGINSTA)),res, "%s%s,%s\r\n" , CMD_WRITE(_BEGINSTA), ssid, passphrase)) {
      return WL_CONNECT_FAILED;
   }

   unsigned long start_time = millis();
   while(millis() - start_time < 10000){
      if(modem.write(string(PROMPT(_GETSTATUS)),res,CMD(_GETSTATUS))) {
         if(atoi(res.c_str()) == WL_CONNECTED) {
            return WL_CONNECTED;
         }
      }      
   }

  return WL_CONNECT_FAILED;
}

/* passphrase is needed so a default one will be set */
/* -------------------------------------------------------------------------- */
uint8_t CWifi::beginAP(const char *ssid) {
/* -------------------------------------------------------------------------- */   
  return 0;
}

/* -------------------------------------------------------------------------- */
uint8_t CWifi::beginAP(const char *ssid, uint8_t channel) {
/* -------------------------------------------------------------------------- */   
  return 0;
}

/* -------------------------------------------------------------------------- */
uint8_t CWifi::beginAP(const char *ssid, const char* passphrase) {
/* -------------------------------------------------------------------------- */   
  return 0;
}

/* -------------------------------------------------------------------------- */
uint8_t CWifi::beginAP(const char *ssid, const char* passphrase, uint8_t channel) {
/* -------------------------------------------------------------------------- */   
  return 0;
}



/* -------------------------------------------------------------------------- */
void CWifi::config(IPAddress local_ip) {
/* -------------------------------------------------------------------------- */
}

/* -------------------------------------------------------------------------- */
void CWifi::_config(IPAddress local_ip, IPAddress gateway, IPAddress subnet) {
/* -------------------------------------------------------------------------- */    
}

/* -------------------------------------------------------------------------- */
void CWifi::config(IPAddress local_ip, IPAddress dns_server) {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway) {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::config(IPAddress local_ip, IPAddress dns_server, IPAddress gateway, IPAddress subnet) {
/* -------------------------------------------------------------------------- */
}

/* -------------------------------------------------------------------------- */
void CWifi::setDNS(IPAddress dns_server1) {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::setDNS(IPAddress dns_server1, IPAddress dns_server2) {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::setHostname(const char* name) {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
int CWifi::disconnect() {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::end(void) {
/* -------------------------------------------------------------------------- */   

}

/* -------------------------------------------------------------------------- */
uint8_t* CWifi::macAddress(uint8_t* mac) {
/* -------------------------------------------------------------------------- */   
  return 0;
}

static bool macStr2macArray(uint8_t *mac_out, const char *mac_in) {
   if(mac_in[2] != ':' || 
      mac_in[5] != ':' ||
      mac_in[8] != ':' ||
      mac_in[11] != ':' ||
      mac_in[14] != ':') {
      return false;
   }

   for(int i = 0; i < 6; i++) {
      std::string str_num(mac_in+(i*3),2);
      *(mac_out+i) = std::stoul(str_num,nullptr,16);
   }

   return true;
}


/* -------------------------------------------------------------------------- */
int8_t CWifi::scanNetworks() {
/* -------------------------------------------------------------------------- */
   modem.begin();
   access_points.clear();
   string res;
   
   vector<string> aps;
   if(modem.write(string(PROMPT(_WIFISCAN)),res,CMD(_WIFISCAN))) {

      split(aps, res, string("\r\n"));
      for(uint16_t i = 0; i < aps.size(); i++) {
         CAccessPoint ap;
         vector<string> tokens;
         split(tokens, aps[i], string("|"));
         if(tokens.size() >= 5) {
            ap.ssid            = tokens[0];
            ap.bssid           = tokens[1];
            macStr2macArray(ap.uint_bssid, ap.bssid.c_str());
            ap.rssi            = tokens[2];
            ap.channel         = tokens[3];
            ap.encryption_mode = tokens[4];
            access_points.push_back(ap);
         }
      } 
   }

   return (int8_t)access_points.size();
}
 
/* -------------------------------------------------------------------------- */   
IPAddress CWifi::localIP() {
/* -------------------------------------------------------------------------- */   
  return IPAddress(0,0,0,0);
}

/* -------------------------------------------------------------------------- */
IPAddress CWifi::subnetMask() {
/* -------------------------------------------------------------------------- */   
  return IPAddress(0,0,0,0);
}

/* -------------------------------------------------------------------------- */
IPAddress CWifi::gatewayIP() {
/* -------------------------------------------------------------------------- */   
  return IPAddress(0,0,0,0);
}

/* -------------------------------------------------------------------------- */
const char* CWifi::SSID(uint8_t networkItem) {
/* -------------------------------------------------------------------------- */
  if(networkItem < access_points.size()) {
      return access_points[networkItem].ssid.c_str();   
  }
  return nullptr;
}
/* -------------------------------------------------------------------------- */ 

/* -------------------------------------------------------------------------- */
int32_t CWifi::RSSI(uint8_t networkItem) {
  if(networkItem < access_points.size()) {
      return atoi(access_points[networkItem].rssi.c_str());   
  }
  return -1000;
}
/* -------------------------------------------------------------------------- */ 

static uint8_t Encr2wl_enc(string e) {
   if (e == string("open")) {
      return ENC_TYPE_NONE;
   } else if (e == string("WEP")) {
      return ENC_TYPE_WEP;
   } else if (e == string("WPA")) {
      return ENC_TYPE_WPA;
   } else if (e == string("WPA2")) {
      return ENC_TYPE_WPA2;
   } else if (e == string("WPA+WPA2")) {
      return ENC_TYPE_WPA2;
   } else if (e == string("WPA2-EAP")) {
      return ENC_TYPE_WPA2_ENTERPRISE;
   } else if (e == string("WPA2+WPA3")) {
      return ENC_TYPE_WPA3;
   } else if (e == string("WPA3")) {
      return ENC_TYPE_WPA3;
   } else {
      return ENC_TYPE_UNKNOWN;
   }
 }




/* -------------------------------------------------------------------------- */
uint8_t CWifi::encryptionType(uint8_t networkItem) {
  if(networkItem < access_points.size()) {
      return Encr2wl_enc(access_points[networkItem].encryption_mode);   
  }
  return 0;
}
/* -------------------------------------------------------------------------- */ 





/* -------------------------------------------------------------------------- */
uint8_t* CWifi::BSSID(uint8_t networkItem, uint8_t* bssid) {
  if(networkItem < access_points.size()) {
      for(int i = 0; i < 6; i++) {
         *(bssid + i) = access_points[networkItem].uint_bssid[i];
      }
      return bssid;   
   }
  return nullptr;
}
/* -------------------------------------------------------------------------- */ 

/* -------------------------------------------------------------------------- */
uint8_t CWifi::channel(uint8_t networkItem) { 
   if(networkItem < access_points.size()) {
      return atoi(access_points[networkItem].channel.c_str());   
   }
   return 0;
}
/* -------------------------------------------------------------------------- */ 

/* -------------------------------------------------------------------------- */ 
const char* CWifi::SSID() {
/* -------------------------------------------------------------------------- */    
  return ""; 
}

/* -------------------------------------------------------------------------- */ 
uint8_t* CWifi::BSSID(uint8_t* bssid) {
/* -------------------------------------------------------------------------- */    
  return nullptr;
}

/* -------------------------------------------------------------------------- */ 
int32_t CWifi::RSSI() {
/* -------------------------------------------------------------------------- */    
  return 0;
}

/* -------------------------------------------------------------------------- */ 
uint8_t CWifi::encryptionType() {
/* -------------------------------------------------------------------------- */    
  return 0;
}

/* -------------------------------------------------------------------------- */
uint8_t CWifi::status() {
/* -------------------------------------------------------------------------- */   
   string res = "";
   if(modem.write(string(PROMPT(_GETSTATUS)), res, CMD_READ(_GETSTATUS))) {
      return atoi(res.c_str());
   }
   return 0;
}

/* -------------------------------------------------------------------------- */
int CWifi::hostByName(const char* aHostname, IPAddress& aResult) {
/* -------------------------------------------------------------------------- */   
  return 0;
}

/* -------------------------------------------------------------------------- */
void CWifi::lowPowerMode() {
/* -------------------------------------------------------------------------- */   
}

/* -------------------------------------------------------------------------- */
void CWifi::noLowPowerMode() {
/* -------------------------------------------------------------------------- */   
}

uint8_t CWifi::reasonCode() {
  return 0;
}

unsigned long CWifi::getTime() {
  return 0;
}



void CWifi::setTimeout(unsigned long timeout) {
   (void)(timeout);  
}


CWifi WiFi;

