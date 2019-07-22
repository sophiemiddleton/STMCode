//Author: S Middleton
//Date : 07 Jun2019
//Purpose : This file is meant to create "fake data packets" from a sample CSV file for validation of the algorithms
//Note : to run you need to invoke c++11, to compile type: 
// "g++ -std=c++11 CVSReader.cc" 
//  then "./a.out" as usual.

#include <iostream>
#include <fstream>
#include <vector>
#include <iterator>
#include <string>
#include <algorithm>
#include <boost/algorithm/string.hpp>

#include "STMDataPacket.hpp"
using namespace std;
using namespace boost;

class CSVReaderUpdatedStructure
{
  std::string FileName;
  std::string LineDelimeter;
 
public:
  CSVReaderUpdatedStructure(std::string filename, std::string delm = ";") : FileName(filename), LineDelimeter(delm)
	{ }
 
  std::vector<STMDataPacket> GetData();
};

std::vector<STMDataPacket> CSVReaderUpdatedStructure::GetData()
{
  
  std::ifstream file(FileName);
  std::vector<std::vector<std::string> > DataList;
  std::vector<STMDataPacket> Data;
  std::string line = "";
  int i = 0;
  while (getline(file, line))
	{
	 
	  i+=1;
	  if(i>1){ //Skips first row as this is just titles  
	    std::vector<std::string> vec;
	    boost::algorithm::split(vec, line, boost::is_any_of(LineDelimeter));
	    DataList.push_back(vec);
	}
	}
       for(unsigned i=0; i < DataList.size(); i++){
	 STMDataPacket d;
	 // STMDataPacket::DataSample d.DataSample;
         d.datasample.GlobalTimeStamp = (atof (DataList[i][0].c_str()));
	 d.datasample.RequestPacketTime = (atof (DataList[i][0].c_str()));
	 for(unsigned ch=0; ch<1;ch++){//loop 4 channels - here assume just 1
	   //STMDataPacket::ChannelHeader ;
	   // STMDataPacket::ChannelPacket d.ChannelPacket;
           d.channelheader.ChannelID = ch;
           d.channelpacket.ChannelID = ch;
           d.channelpacket.CountLength = (DataList[i].size());
           d.channelheader.Status = true;
	   for(unsigned j=2; j< (DataList[i]).size();j++){
	     float  isample= atof (DataList[i][j].c_str()); 
	     d.channelpacket.ADCList.push_back(isample);	   
	    }
	   d.datasample.channel.push_back(d.channelpacket);
         
	 }
        
	 Data.push_back(d);
       }
   
       DataList.clear();
       file.close();
       return Data;
}

int  read()
{
  std::cout<<"In Main...Extracting Data..."<<std::endl;
  CSVReaderUpdatedStructure reader("DT5725-2-14-1694_Data_run_co60_waveforms_hpge_06042019.csv");
  std::vector<STMDataPacket> DataList = reader.GetData();
  return 0;
}
