#pragma once

#include <map>

#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"

#include "Config.h"
#include "SerialManager.h"

/*
   GROUP IDS
  orange = 1
  green = 2
  aqua =  3
  blue = 4
  purple = 5
*/

class RFManager :
  public CommandProvider
{
  public:
    typedef struct {
	uint8_t data[21];
    } tx_payload_t;

    typedef struct {
	int8_t rssi;
	uint8_t data[sizeof(tx_payload_t)];
    } rx_payload_t;

    static constexpr int TX_QUEUE_LEN = 16;
    static constexpr int GROUP_SLOTS = 64;

    RFManager()
	: CommandProvider("RF")
	, radio(4, 33)
	, tx_queue{xQueueCreate(TX_QUEUE_LEN, sizeof(tx_payload_t))}
    {}
    virtual ~RFManager() {}

    RF24 radio; //CE and CS pins for SPI bus on NRF24+

    uint8_t address[3] = { 0x01, 0x07, 0xf1 };

    QueueHandle_t tx_queue;

    bool isListening = false;
    tx_payload_t currentPacket;
    int remainingRepeats = 0;

    volatile bool doClearGroups = false;
    std::map<uint16_t, rx_payload_t> groups = {};


    void init()
    {
	setRFDataCallback(&RFManager::onRFDataDefaultCallback);
	setupRadio();

	DBG("RF Manager is init.");
    }

    void update()
    {
	sendNextPacket();
	receivePackets();
    }


    void setupRadio()
    {
      radio.begin();
      radio.setAutoAck(false);
      radio.setDataRate(RF24_250KBPS);
      radio.setChannel(2);
      radio.setAddressWidth(3);
      radio.setPayloadSize(sizeof(tx_payload_t));
      radio.setCRCLength(RF24_CRC_16);

      radio.stopListening();
      radio.openReadingPipe(1, address);
      radio.openWritingPipe(address);
      radio.startListening();
      isListening = true;

#if SERIAL_DEBUG
      radio.printDetails();
#endif

    }

    void control(const std::string& data) {
	doClearGroups = true;
    }

    void queuePacket(const std::string& data) {
	log_v("queuing packet");
	xQueueSendToBack(tx_queue, data.data(), 0);
    }

    bool haveQueuedPackets() {
	return uxQueueMessagesWaiting(tx_queue) > 0;
    }

    void dequeuePacket() {
	log_v("dequeuing packet");
	xQueueReceive(tx_queue, &currentPacket, portMAX_DELAY);
    }

    void sendNextPacket() {
	if (remainingRepeats == 0) {
	    if (!haveQueuedPackets()) {
		if (isListening)
		    return;
		log_v("waiting for tx standby");
		if (radio.txStandBy()) {
		    log_v("switching to listening");
		    radio.startListening();
		    isListening = true;
		    return;
		}
	    } else {
		dequeuePacket();
		remainingRepeats = 3;
		log_v("dequeued packet");
	    }
	}

	if (!radio.txFifoFull()) {
	    log_v("fifo has space, sending packet, %d remaining", remainingRepeats);
	    if (isListening) {
		radio.stopListening();
		isListening = false;
	    }
	    if (radio.writeFast(currentPacket.data, sizeof(currentPacket))) {
		// success
		remainingRepeats--;
		log_v("transmit submitted");
	    }
	}
    }

    void receivePackets() {
        while (radio.available()) {
	    rx_payload_t receivingPacket;
	    radio.read(&receivingPacket.data, sizeof(receivingPacket.data));
	    receivingPacket.rssi = 127;

	    uint16_t group;
	    memcpy(&group, &receivingPacket.data, sizeof(group));

	    if (doClearGroups)
		groups.clear();
	    if (groups.find(group) != groups.end()) {
		const auto& pkt = groups[group];
		if (memcmp(pkt.data, receivingPacket.data, sizeof(receivingPacket.data)) == 0) {
		    continue;
		}
		groups[group] = receivingPacket;
	    } else if (groups.size() < GROUP_SLOTS) {
		log_v("new group %04x", group);
		groups[group] = receivingPacket;
	    }

	    log_v("new packet from group %04x", group);

	    auto data_str = std::string((char*)&receivingPacket, sizeof(receivingPacket));
	    onRFData(data_str);
        }
    }


    // COMMAND FUNCTIONS



    //DATA SYNC
    typedef void(*RFEvent)(const std::string&);
    RFEvent onRFData;
    void setRFDataCallback (RFEvent func) {
      onRFData = func;
    }
    static void onRFDataDefaultCallback(const std::string&) {}
};
