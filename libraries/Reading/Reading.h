//
//  Reading.h
//  
//
//  Created by Chris Rice on 5/29/13.
//
//

#ifndef _Reading_h
#define _Reading_h

#include "Arduino.h"

class Reading {
public:
    Reading() {}
    Reading(String time, double dp, double ec, double temp);
    
    String toUploadString();
    String toString();
    
private:
    String reading_time;
    double dielectric_permittivity;
    double electrical_conductivity;
    double temperature;
    
    String doubleToString(double input);
};

#endif
