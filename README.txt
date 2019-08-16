This code can be compiled with:

g++-9 -std=c++14 ModuleDevelopment.cpp -I/usr/local/Cellar/tbb/2019_U8/include/  -L/usr/local/Cellar/tbb/2019_U8/lib/ -I /usr/local/Cellar/boost/1.70.0/include/ -L/usr/local/Cellar/boost/1.70.0/lib/ -ltbb -lboost_system  -pthread 

For the most recent C++ libraries. However, its possible to use in C++14 I think.

Some details about the classes here:

1) STMDataPacket - input class for proposed STMData Packet from the FPGA

2) STMDataPacketInput - turns the CSV/TXT file into the STMDataPacket Format

3) STMDataProduct - the output of the analysis, will be passed ot offlinf

4) ModuleDevelopement - The algorithim

5) Other threader- infrastrucutre for multithreading for time efficiency
