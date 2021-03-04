#pragma once

#include <WiFi.h>
#include <WiFiMulti.h>
#include <WiFiUdp.h>

#include "Config.h"
#include "SerialManager.h"

#define CONNECT_TIMEOUT 5000
#define CONNECT_TRYTIME 500

class WifiManager
{
public:
  WifiManager(){  
      setCallbackConnectionUpdate(&WifiManager::connectionUpdateDefaultCallback);
  }
  
  ~WifiManager(){}

  bool isLocal = false;
  bool isConnected = false;
  bool isConnecting = false;
  bool hasScannedForNetworks = false;
  
  String ssid;
  String pass;
  
  long timeAtStartConnect;
  long timeAtLastConnect;

  bool isActivated;
  
  void init()
  {
    isActivated = Config::instance->getWifiMode();

    if(isConnected)
    {
      DBG("Disconnecting first...");
      WiFi.disconnect();
      delay(100);
    }
    
    if(!isActivated)
    {
      DBG("Wifi is not activated, not initializing");
      return;
    }

    
    digitalWrite(13, HIGH);
  }

  void attemptToConnect(String ssid) {
    String password = Config::instance->getWifiPassword(ssid);

    if (password.length() > 0) {
      DBG("Connecting to Wifi "+ssid+" with password "+pass);
      if (isConnecting)
        networksToTryLater.push_back(ssid);
      else {
        isLocal = false;
        isConnecting = true;
        setConnected(false);
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid.c_str(), password.c_str());
        timeAtStartConnect = millis();
        timeAtLastConnect = millis();
      }
    }
  }

  std::vector<String> networksToTryLater;

  void update()
  {
    if(!isActivated) return;
    if(isLocal || isConnected) return;

		// DBG("Not connected to wifi yet...");
    if(millis() > timeAtLastConnect + CONNECT_TRYTIME) {      
      if(WiFi.status() == WL_CONNECTED) {  
         digitalWrite(13, LOW);

				DBG("Grabbing local IP address:");
				String localIp = WiFi.localIP().toString();
        DBG("WiFi Connected, local IP : "+localIp);

        isLocal = false;
        isConnecting = false;
        setConnected(true);
    
         return;
      }
      timeAtLastConnect = millis();

      if (networksToTryLater.size() > 0) {
        std::vector<String> networksToTry = networksToTryLater;
        networksToTryLater = {};
        for (int i = 0; i < networksToTry.size(); ++i)
          attemptToConnect(networksToTry[i]);
      } else if (!hasScannedForNetworks) {
        attemptToConnect(Config::instance->getMostRecentWifiSSID());

        int foundNetworks = WiFi.scanNetworks();
        hasScannedForNetworks = true;
        if (foundNetworks > 0) {
          Serial.print(foundNetworks);
          Serial.println(" networks found");
          for (int i = 0; i < foundNetworks; ++i)
            attemptToConnect(WiFi.SSID(i));
        }
      }
    }
        
    if(millis() > timeAtStartConnect + CONNECT_TIMEOUT)
    {
      // DBG("Could not connect to "+ssid);
      setConnected(false);
      for(int i=0;i<5;i++)
      {
        digitalWrite(13, HIGH);
        delay(50);
        digitalWrite(13, LOW);
        delay(50);
      }
      
      setupLocal();
    }
  }

  void setupLocal()
  {
    String softAPName = "FlowConnect "+Config::instance->getDeviceName();
    String softAPPass = "findyourflow";
    WiFi.softAP(softAPName.c_str(), softAPPass.c_str());
    Serial.println("Local IP : "+String(WiFi.softAPIP()[0])+
    "."+String(WiFi.softAPIP()[1])+
    "."+String(WiFi.softAPIP()[2])+
    "."+String(WiFi.softAPIP()[3]));

    isLocal = true;
    isConnecting = false;
    setConnected(true);
    
    DBG("AP WiFi is init with name "+softAPName+" , pass : "+softAPPass);
  }

  void setConnected(bool value)
  {
    isConnected = value;
    onConnectionUpdate();
  }


  typedef void(*onConnectionUpdateEvent)();
    void (*onConnectionUpdate) ();

    void setCallbackConnectionUpdate (onConnectionUpdateEvent func) {
      onConnectionUpdate = func;
    }

    static void connectionUpdateDefaultCallback()
    {
      //nothing
    }
};
