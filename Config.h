#pragma once
#include "Preferences.h"

class Config
{
public:
  static Config * instance;

  enum WifiBLEMode { WIFI_ONLY = 0, BLE_ONLY = 1, BOTH = 2 };
  
  Preferences preferences;
  
  Config()
  {
    instance = this;
  }

  ~Config(){}

  void init() {
    preferences.begin("bridge", false);
  }

  // std::string[] getWifiCredentials() {
  //   String credentials = preferences.getString("wifiCredentials");
  // }
  // void putWifiCredentials() {
  //   return preferences.getULong(String("privateGroup"+String(privateGroupIndex)).c_str(), 0);
  // }
  uint32_t getRFNetworkId(int privateGroupIndex) { return preferences.getULong(String("privateGroup"+String(privateGroupIndex)).c_str(), 0); }
  void setRFNetworkId(int privateGroupIndex, uint32_t groupID) { preferences.putULong(String("privateGroup"+String(privateGroupIndex)).c_str(), groupID); }

  int getNumPrivateGroups() { return preferences.getInt("numGroups", 0); }
  void setNumPrivateGroups(int num) { preferences.putInt("numGroups", num); }

  String getDeviceName() { return preferences.getString("deviceName", "FlowConnect " + String((int)ESP.getEfuseMac()).substring(0,10)); }
  void setDeviceName(String deviceName) { preferences.putString("deviceName", deviceName); }

  bool getWifiMode() { int mode = preferences.getInt("wifiBLEMode",2); return mode == WIFI_ONLY || mode == BOTH; } //0 is wifi only, 1 is BLE only, 2 is both
  bool getBLEMode() { int mode = preferences.getInt("wifiBLEMode",2); return mode == BLE_ONLY || mode == BOTH; }
  void setWifiBLEMode(int mode) { preferences.putInt("wifiBLEMode", mode); }
 
  String getMostRecentWifiSSID() { return preferences.getString("ssid",""); }
  void setWifiSSID(String ssid) { preferences.putString("ssid",ssid); }
  
  void setWifiPassword(String ssid, String pass) { preferences.putString(String("PASS:"+ssid).c_str(), pass); }
  String getWifiPassword(String ssid) {
		Serial.println("GET WIFI NETWORK FOR " + ssid);
		return preferences.getString(String("PASS:"+ssid).c_str(), "");
	}
  // String getWifiPassword() { return preferences.getString("pass","flowarts"); }
  // void setWifiPassword(String pass) { preferences.putString("pass", pass); }

  int getButtonStateVal(int id) { return preferences.getInt(String("buttonValue"+String(id)).c_str(),0); }
  void setButtonStateVal(int id, int val) { preferences.putInt(String("buttonValue"+String(id)).c_str(), val); }
};

Config * Config::instance = nullptr;
