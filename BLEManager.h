#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#define FLOWBRIDGE_SERVICE_UUID "49550001-AAD5-59BD-934C-023D807E01D5"
#define FLOWBRIDGE_RX_CHAR_UUID "49550002-AAD5-59BD-934C-023D807E01D5"
#define FLOWBRIDGE_TX_CHAR_UUID "49550003-AAD5-59BD-934C-023D807E01D5"
#define FLOWBRIDGE_CONTROL_CHAR_UUID "49550004-AAD5-59BD-934C-023D807E01D5"
#define FLOWBRIDGE_NAME_CHAR_UUID "49550005-AAD5-59BD-934C-023D807E01D5"


class BLEManager
{
  public:

    class ServerCallback: public BLEServerCallbacks {
      public:
        ServerCallback(BLEManager * manager) : manager(manager) {}
        BLEManager * manager;

        void onConnect(BLEServer* pServer) {
          manager->deviceConnected = true;
          log_i("BLE is connected.");
        };

        void onDisconnect(BLEServer* pServer) {
          manager->deviceConnected = false;
          log_i("BLE is disconnected.");
        }
    };

    struct TxCallback: public BLECharacteristicCallbacks {
	BLEManager* manager;

	TxCallback(BLEManager* manager_)
	    : manager{manager_}
	{}

        void onWrite(BLECharacteristic *pCharacteristic) {
	    auto&& val = pCharacteristic->getValue();
	    if (manager->txCallback)
		manager->txCallback(val);
        }
    };

    struct CtrlCallback: public BLECharacteristicCallbacks {
	BLEManager* manager;

	CtrlCallback(BLEManager* manager_)
	    : manager{manager_}
	{}

        void onWrite(BLECharacteristic *pCharacteristic) {
	    auto&& val = pCharacteristic->getValue();
	    if (manager->ctrlCallback)
		manager->ctrlCallback(val);
        }
    };

    class NameCallback: public BLECharacteristicCallbacks {
        void onWrite(BLECharacteristic *pCharacteristic) {
	    String name = String(pCharacteristic->getValue().c_str());
	    Config::instance->setDeviceName(name);
        }
    };


    BLEManager() {}

    BLEServer *pServer = NULL;
    BLECharacteristic * pTxCharacteristic;
    BLECharacteristic * pRxCharacteristic;
    BLECharacteristic * pCtrlCharacteristic;
    BLECharacteristic * pNameCharacteristic;

    using valueCallbackType = void (*)(const std::string&);

    valueCallbackType txCallback;
    valueCallbackType ctrlCallback;

    volatile bool deviceConnected = false;
    bool oldDeviceConnected = false;

    bool isActivated;

    void init()
    {
      // Create the BLE Device

      isActivated = Config::instance->getBLEMode();

      if(!isActivated)
      {
        DBG("BLE not activated in preferences, not initializing");
        return;
      }

      String bleName = Config::instance->getDeviceName();
      BLEDevice::init(bleName.c_str());

      // Create the BLE Server
      pServer = BLEDevice::createServer();
      pServer->setCallbacks(new ServerCallback(this));

      // Create the BLE Service
      BLEService *pService = pServer->createService(FLOWBRIDGE_SERVICE_UUID);

      pTxCharacteristic = pService->createCharacteristic(
	  FLOWBRIDGE_TX_CHAR_UUID,
	  BLECharacteristic::PROPERTY_WRITE
	  );
      pTxCharacteristic->setCallbacks(new TxCallback(this));

      pRxCharacteristic = pService->createCharacteristic(
          FLOWBRIDGE_RX_CHAR_UUID,
          BLECharacteristic::PROPERTY_NOTIFY
	  );
      pRxCharacteristic->addDescriptor(new BLE2902());

      pCtrlCharacteristic = pService->createCharacteristic(
	  FLOWBRIDGE_CONTROL_CHAR_UUID,
	  BLECharacteristic::PROPERTY_WRITE
	  );
      pCtrlCharacteristic->setCallbacks(new CtrlCallback(this));

      pNameCharacteristic = pService->createCharacteristic(
	  FLOWBRIDGE_NAME_CHAR_UUID,
	  BLECharacteristic::PROPERTY_WRITE
	  );
      pNameCharacteristic->setCallbacks(new NameCallback());

      // Start the service
      pService->start();

      // Start advertising
      pServer->getAdvertising()->addServiceUUID(FLOWBRIDGE_SERVICE_UUID);
      pServer->getAdvertising()->start();
      DBG("BLE is init with name "+bleName);
    }

    void update()
    {
      if(!isActivated) return;

      if (!deviceConnected && oldDeviceConnected) {
        delay(500); // give the bluetooth stack the chance to get things ready
        pServer->startAdvertising(); // restart advertising
        Serial.println("start advertising");
        oldDeviceConnected = deviceConnected;
      }
      // connecting
      if (deviceConnected && !oldDeviceConnected) {
        // do stuff here on connecting
        oldDeviceConnected = deviceConnected;
      }
    }

    void sendRxData(const std::string& data) {
	if (!isActivated)
	    return;

	pRxCharacteristic->setValue(data);
	pRxCharacteristic->notify();
    }
};
