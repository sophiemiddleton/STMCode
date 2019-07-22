//Author: S Middleton
//Date: July 2019
//Purpose: Class to hold STM Data Packet as outputted by STM ROC FPGA
#include<vector>
using namespace std;


   typedef struct ChannelHeader{
      //Local Header:
      short int ChannelID;//2 bits
      char  Flags;
      char  Status;
   } ChannelHeader;

   typedef struct ChannelPacket{
      //Local Data Packets:
      short int ChannelID;//2 bits
      long unsigned CountLength; //nDigits
      vector<char> ErrFlags;
      vector<double> ADCList;//Assume 1 channel at the moment
   } ChannelPacket;

   typedef  struct DataSample{
      //Global Header:
      long int DataLength;//nSamples
      int DetID; //8 bits
      char TriggerType;//4 bits
      int FPGAVersion; //8bits
      long long unsigned int TriggerNumber; //64 bits
      long unsigned ResetTime1; //48 bits for all these
      long unsigned StopTime1;
      long unsigned ResetTime2;
      long unsigned StopTime2;
      long unsigned ResetTime3;
      long unsigned StopTime3;
      long unsigned ResetTime4;
      long unsigned StopTime4;
      long unsigned RequestPacketTime;
      double GlobalTimeStamp;
     vector<ChannelPacket> channel;//for current purpose....
    } DataSample;

class STMDataPacket{
    public:
      DataSample datasample;
      ChannelHeader channelheader;
      ChannelPacket channelpacket;
 
 };


