//Author: SMiddleton
//Date July 2019
//Purpose Test Script for algorithim in c++ - must then add to the art module in DMW.cc, use c++14
#include <iostream>
#include <functional>
#include <future>
#include <fstream>
#include <vector>
#include <chrono> 
#include <algorithm>
#include <math.h>
#include<thread>
#include<mutex>
#include <ctime>
#include "STMDataPacketInput.cc"
#include "STMDataProduct.hpp"
#include "OtherThreader.hpp"

using namespace std;


//Globals - in art upgrade these may become FCL parameters:
unsigned char l = 100;
unsigned char m= 200; 
STMDataPacket Sample;
float tau;
float factor;
bool writefile = false;
unsigned nSamples;
ofstream   outputTfile;
unsigned SampleCounter =0;
std::vector<STMDataProduct>  pulses;
bool time_info = false;
unsigned i=0;//for debug counter

//System Details - from /proc/cpuinfo or lscpu can remain hardcoded once we know system (?)
unsigned N_cores_per_socket = 14;
unsigned N_sockets = 2;
unsigned N_threads_per_core = 2;
unsigned N_CPUs = 56;
unsigned N_Management_Threads =1; //used to oversee threading
unsigned MAX_THREADS = N_cores_per_socket*N_threads_per_core*N_sockets;
unsigned THREAD_COUNT = 1500;//MAX_THREADS;

//Tester function for threading:
void printer (int id) {
  if (id%100){
  cout << "sample " << id << '\n';
}
}

//Basic Tau Finder:
float UpdateTau(vector<STMDataPacket> DataList, int nSamples, int start, int end){
 float tau =0;
 for(unsigned i=1; i< nSamples-1; i++){   
    float dsigma =  log(DataList[i].datasample.channel[0].ADCList[end])- log(DataList[i].datasample.channel[0].ADCList[start]); 
    tau += (1/((dsigma)/(end-start)));
}
 return abs(tau)/nSamples;
}


//Algortithm
void GetDMWPulses(int s, STMDataPacket DataList,  bool time_info, bool writefile, float factor){
   
    unsigned int nDigits = DataList.datasample.channel[0].ADCList.size();
    STMDataProduct pulse;
    auto start = chrono::high_resolution_clock::now();
    vector<float>  hist,A, MWD, T;
   
    hist.assign(nDigits,0);
    A.assign(nDigits,0);
    MWD.assign(nDigits,0);
    T.assign(nDigits,0);
    
    for(unsigned j=1;j<nDigits-1; j++){    	 
      
           hist[j] =(DataList.datasample.channel[0].ADCList[j]);
	   if(time_info and s == 0 and  j==nDigits-2 ){
	      auto stop2 = chrono::high_resolution_clock::now(); 	
	      auto duration2 = chrono::duration_cast<chrono::microseconds>(stop2 - start);
	      cout<<"Hist "<<duration2.count()<<endl;
	   }   
    } 
     for(unsigned n=1;n<nDigits-1; n++){   
          A[n]= ((hist[n]-factor*hist[n-1]+A[n-1])); 
          if(time_info  and  s ==0 and  n==nDigits-2){
	       auto stop3 = chrono::high_resolution_clock::now(); 	
               auto duration3 = chrono::duration_cast<chrono::microseconds>(stop3 - start);
	       cout<<"A "<<duration3.count()<<endl;
         }
     }//end j loop
     for(unsigned k = m+2; k < nDigits-2; k++){
	  MWD[k]=((A[k]-A[k-m]));  
          if(time_info and  s==0 and k==nDigits-3){
             auto stop4 = chrono::high_resolution_clock::now(); 	
             auto duration4 = chrono::duration_cast<chrono::microseconds>(stop4 - start); 
             cout<<"MWD "<<duration4.count()<<endl;
       }
     }
     for(unsigned h = m+l+2; h< nDigits-2; h++){ 
	   for (unsigned p = h-l-1; p <h-2 ; p++){ 
	        T[h] += ((MWD[p]))/l; 	           
   }
	   if(writefile){outputTfile<<" "<<T[h]<<" "<<"\n";}
     }
     if(time_info and  s==0){
         auto stop5 = chrono::high_resolution_clock::now(); 	
         auto duration5 = chrono::duration_cast<chrono::microseconds>(stop5 - start);
	 cout<<"T "<<duration5.count()<<endl;;   
     } 
   
   float maximum_height = *std::max_element(T.begin(), T.end());
   int maxElementIndex = std::max_element(T.begin(),T.end()) - T.begin();
   float peak_time = DataList.datasample.channel[0].ADCList.at(maxElementIndex);
   pulse.SetPulseTime(peak_time) ;
   pulse.SetPulseHeight(maximum_height);
   pulses.push_back(pulse);
   if(time_info  and s==0){
	   auto stop6 = chrono::high_resolution_clock::now(); 	
	   auto duration6 = chrono::duration_cast<chrono::microseconds>(stop6 - start); 
	   cout<<"Storage "<<duration6.count()<<endl;
    }
  
      hist.clear();
      A.clear();
      MWD.clear();
      T.clear(); 
    
    }//End Algorithim Function


//Driving Function:
int main(){
  CSVReaderUpdatedStructure reader("data.csv");
  vector<STMDataPacket> DataList = reader.GetData();

  tau = UpdateTau(DataList,100, 2000, 3000);
  factor = (1-1/tau);
  if(writefile){
      outputTfile.open("OptT.dat");
  }
  counting_barrier barrier(15000);
  thread_pool Pool(THREAD_COUNT);
  auto startT0 = chrono::high_resolution_clock::now();
   for(unsigned s=0; s<15000; s++){
      
      auto done = Pool.add_task([&barrier, s= s, Sample= DataList[s], t_inf = time_info,wf=writefile, f=factor]{
          GetDMWPulses(s, Sample, t_inf, wf,f);
          --barrier;
        });
    }
  
  barrier.wait();
  auto stop = chrono::high_resolution_clock::now();
  cout<<"pulses "<<pulses.size()<<endl;
  auto duration = chrono::duration_cast<chrono::microseconds>(stop - startT0); 
  cout <<"time for MWD full process = "<< duration.count() <<" microseconds "<< endl;
  for(size_t p=0;p<4;p++){
      cout<<pulses[p].GetPulseTime()<<endl;
  }
  if(writefile){outputTfile.close();}
  return 0;

}


