//
//  Reading.cpp
//  
//
//  Created by Chris Rice on 5/29/13.
//
//

#include "Reading.h"

Reading::Reading(String time, double dp, double ec, double temp){
    reading_time = time;
    dielectric_permittivity = dp;
    electrical_conductivity = ec;
    temperature = temp;
}

String Reading::toUploadString(){
    String data_string = "";
    data_string += "&t=";
    data_string += reading_time;
    data_string += "&p=";
    data_string += doubleToString(dielectric_permittivity);
    data_string += "&c=";
    data_string += doubleToString(electrical_conductivity);
    data_string += "&tp=";
    data_string += doubleToString(temperature);
    
    return data_string;
}

String Reading::toString(){
    String data_string = "";
    data_string += reading_time;
    data_string += ",";
    data_string += doubleToString(dielectric_permittivity);
    data_string += ",";
    data_string += doubleToString(electrical_conductivity);
    data_string += ",";
    data_string += doubleToString(temperature);
    data_string += ",";
    
    return data_string;
}

String Reading::doubleToString(double input){
    char input_buffer[10] = {'/0'};
    return dtostrf(input, 2, 2, input_buffer);
}
