//Author: S. Middleton
//Date: July 2019
// Purpose: Prototype for the DataProduct of STM analysis code

#include <iostream>

using namespace std;

class STMDataProduct{
private:
     double PulseHeight;
     double PulseTime;
    
public:
  void SetPulseHeight(double height){PulseHeight = height;}
  double GetPulseHeight(){ return PulseHeight;}

  void SetPulseTime(double time){PulseTime= time;}
  double GetPulseTime(){ return PulseTime;}
  

};
