#include "RF24.h"
#include "CommandProvider.h"

#pragma pack(push, 1) // prevents memory alignment from disrupting the layout and size of the network packet
struct SyncPacket {
  uint16_t groupID;
  uint32_t serial;
  uint8_t lfo[4];
  uint8_t global_hue;
  uint8_t global_sat;
  uint8_t global_val;
  uint8_t global_speed;
  uint8_t global_density;
  bool lfo_active : 1;
  bool hue_active : 1;
  bool sat_active : 1;
  bool val_active : 1;
  bool speed_active : 1;
  bool density_active : 1;
  bool reserved[2];
  uint8_t page;
  uint8_t mode;
  bool adjust_active : 1;
  bool wakeup : 1;
  bool poweroff : 1;
  bool force_reload : 1;
  bool save : 1;
  bool _delete : 1;
  bool alternate : 1;
};
#pragma pack(pop)

void print_bytes( void *ptr, size_t size )
{
    //char *buf = (char*) ptr;
    unsigned char *p = (unsigned char*)ptr ;

    for( size_t i = 0; i < size; i++ )
    {
        printf( "%02hhX ", p[i] ) ;
    }
    printf( "\n" ) ;

    for( size_t i = 0; i < size; i++ )
    {
        for( short j = 7; j >= 0; j-- )
        {
            printf( "%d", ( p[i] >> j ) & 1 ) ;
        }
        printf(" ");
    }
    printf("\n");
}

class RFGroup
{
  public:
    RFGroup() : isDirty(true) {}

    ~RFGroup() {}

    int dirtyCount = 5;
    bool isDirty;

    void setup(int gid, RF24 * r)
    {
      groupID = gid;
      radio = r;
      packet.padding = 1;
      packet.groupID = ((gid & 0xff) << 8) | ((gid >> 8) & 0xff);
    }

    RF24 * radio = nullptr;
    int groupID = -1;
    SyncPacket packet;

    void sendPacket(bool force = false)
    {
      if(groupID <= 0) return;
      if(dirtyCount == 0 && !force) return;

      radio->write(&packet, sizeof(SyncPacket));

      if(!force) dirtyCount = max(dirtyCount -1, 0);
    }

    void setData(CommandProvider::PatternData data, bool doNotUpdateIfSame = false)
    {
      if(doNotUpdateIfSame)
      {

        if(packet.page == data.page
        && packet.mode == data.mode
 /*       && packet.lfo_active == data.actives & 1//true;
        && packet.hue_active == (data.actives >> 1) & 1//true;
        && packet.sat_active == (data.actives >> 2) & 1//true;
        && packet.val_active == (data.actives >> 3) & 1//true;
        && packet.speed_active == (data.actives >> 4) & 1//true;
        && packet.density_active == (data.actives >> 5) & 1//true;
        && packet.lfo[0] == data.lfo1
        && packet.lfo[1] == data.lfo2
        && packet.lfo[2] == data.lfo3
        && packet.lfo[3] == data.lfo4*/
        && packet.global_val == data.brightness
        && packet.global_hue == data.hueOffset
        && packet.global_sat == data.saturation
        && packet.global_speed == data.speed
        && packet.global_density == data.density
        )
        {
          //DBG(String(groupID) +  " : same packet");
          return;
        }else
        {
        }
      }

      packet.padding++;
      dirtyCount = 5;

      packet.page = data.page;
      packet.mode = data.mode;
      packet.wakeup = false;
      packet.poweroff = false;
//
//      packet.lfo_active = (data.actives >> 5) & 1;
//      packet.hue_active = data.actives & 1;
//      packet.sat_active = (data.actives >> 1) & 1;
//      packet.val_active = (data.actives >> 2) & 1;
//      packet.speed_active = (data.actives >> 3) & 1;
//      packet.density_active = (data.actives >> 4) & 1;


      packet.lfo_active = data.actives & 1;//true;
      packet.hue_active = (data.actives >> 1) & 1;//true;
      packet.sat_active = (data.actives >> 2) & 1;//true;
      packet.val_active = (data.actives >> 3) & 1;//true;
      packet.speed_active = (data.actives >> 4) & 1;//true;
      packet.density_active = (data.actives >> 5) & 1;//true;

      packet.lfo[0] = data.lfo1;
      packet.lfo[1] = data.lfo2;
      packet.lfo[2] = data.lfo3;
      packet.lfo[3] = data.lfo4;
      packet.global_val = data.brightness;
      packet.global_hue = data.hueOffset;
      packet.global_sat = data.saturation;
      packet.global_speed = data.speed;
      packet.global_density = data.density;
/*
      DBG("Set Pattern, groupID = "+String(groupID)+", padding = "+packet.padding
      +", LFO : "+String(packet.lfo_active)+" > lfo0 : "+String(packet.lfo[0])
      +", Hue active : "+String(packet.hue_active)+" > hue : "+String(packet.global_hue)+", sat : "+String(packet.global_sat)+", val : "+String(packet.global_val)
      +", speed : "+String(packet.global_speed)+", density : "+String(packet.global_density)
      );
*/
    }

     bool updateFromPacket(SyncPacket receivingPacket)
     {
      if(packet.padding != receivingPacket.padding)
      {
        //DBG("Received packet with groupID : " + String(receivingPacket.groupID) + ", padding " + String(receivingPacket.padding));
         packet.padding = max(packet.padding,receivingPacket.padding);
         return true;
      }


      return false;

      //packet.page = receivingPacket.page;
      //packet.mode = receivingPacket.mode;

      //DBG("LFO Active : "+String(receivingPacket.lfo_active));
      //DBG("Global Active : "+String(receivingPacket.global_active));
      //DBG("Hue : "+String(receivingPacket.global_hue));
      //DBG("Sat : "+String(receivingPacket.global_sat));
      //DBG("Val : "+String(receivingPacket.global_val));
      //DBG("LFO 0 : "+String(receivingPacket.lfo[0]));

/*
      packet.lfo_active = true;
      packet.global_active = true;
      packet.lfo[0] = data.lfo1;
      packet.lfo[1] = data.lfo2;
      packet.lfo[2] = data.lfo3;
      packet.lfo[3] = data.lfo4;
      packet.global_intensity = data.brightness;
      packet.global_hue = data.hueOffset;
      packet.global_sat = data.saturation;
      packet.global_palette = 0;
      packet.global_speed = data.speed;
      packet.global_density = data.density;
      */
     }

     void wakeUp()
     {
        packet.padding++;
        packet.wakeup = true;
        dirtyCount = 10;
     }

     void powerOff()
     {
        packet.padding++;
        packet.poweroff = true;
        dirtyCount = 10;
     }
};
