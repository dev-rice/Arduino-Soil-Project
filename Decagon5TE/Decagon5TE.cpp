/*
 Decagon5TE.cpp - Library for reading data from a Decagon 5TE.
 Created by Chris A. Rice, May 17, 2013.
 Released into the public domain.
 */

#include "Arduino.h"
#include "Decagon5TE.h"
#include <stdlib.h>

Decagon5TE::Decagon5TE(){
	
}

Decagon5TE::Decagon5TE(int excitation_pin, int serial_control_pin){
	this->excitation_pin = excitation_pin;
	this->serial_control_pin = serial_control_pin;
    update_period = 60000;
    
    last_update_millis = 0;
	ready_for_reading = true;
    
	pinMode(excitation_pin, OUTPUT);
	pinMode(serial_control_pin, OUTPUT);
	
	digitalWrite(excitation_pin, HIGH);
    
}

Decagon5TE::Decagon5TE(int excitation_pin, int serial_control_pin, unsigned long update_period){
	this->excitation_pin = excitation_pin;
	this->serial_control_pin = serial_control_pin;
    this->update_period = update_period;
    
    last_update_millis = 0;
	ready_for_reading = true;
    
	pinMode(excitation_pin, OUTPUT);
	pinMode(serial_control_pin, OUTPUT);
	
	digitalWrite(excitation_pin, HIGH);
    
}

void Decagon5TE::readData(){
    
    last_update_millis = millis();
    ready_for_reading = false;
    
	char data[20] = {'/0'};
	int data_index = 0;
	
	char incoming_byte = '/0';
	
	exciteSensor();
    
	while (Serial.available() > 0){
		incoming_byte = Serial.read();
		data[data_index] = incoming_byte;
		++data_index;
		if (DEBUG) {
			Serial.print(incoming_byte);
		}
	}
    
	dielectric_permittivity = calculateDielectricPermittivty(data);
	electrical_conductivity = calculateElectricalConductivity(data);
   	temperature = calculateTemperature(data);
	
	cycleSerialLine();
    
}

boolean Decagon5TE::isReadyForReading() {
    if (DEBUG){
        Serial.print("millis: ");
        Serial.println(millis());
        
        Serial.print("last update: ");
        Serial.println(last_update_millis);
        
        Serial.print("update period: ");
        Serial.println(millis() - last_update_millis);
        
        Serial.print("ready for reading: ");
        Serial.println(ready_for_reading);
    }
    
    return ((millis() - last_update_millis) >= update_period) && ready_for_reading ;
}

void Decagon5TE::exciteSensor(){
	if (DEBUG) {
		Serial.println("Exciting Sensor...");
	}
    
	digitalWrite(excitation_pin, LOW);
	delay(200);
	digitalWrite(excitation_pin, HIGH);
	
	if (DEBUG) {
		Serial.println("Done.");
	}
}

void Decagon5TE::cycleSerialLine(){
	if (DEBUG) {
		Serial.println("Cycling Serial Line...");
	}
	
	digitalWrite(serial_control_pin, HIGH);
	delay(2500);
	digitalWrite(serial_control_pin, LOW);
	ready_for_reading = true;
	
	if (DEBUG) {
		Serial.println("Done.");
	}
}

double Decagon5TE::calculateDielectricPermittivty(char data[]) {
    String permittivity = "";
    int index = 0;
    
    int raw_dielectric = 0;
    
    while (data[index] != ' '){
        ++index;
    }
    ++index;
    
    while (data[index] != ' '){
        permittivity += data[index];
        ++index;
    }
    
    raw_dielectric = permittivity.toInt();
    
    if (raw_dielectric != 4095){
        return (double)raw_dielectric / 50.0;
    }
    else {
        return 4095;
    }
}
double Decagon5TE::calculateElectricalConductivity(char data[]) {
    String conductivity = "";
    int index = 0;
    
    int raw_conductivity = 0;
    
    for (int i = 0; i < 2; ++i){
        while (data[index] != ' '){
            ++index;
        }
        ++index;
    }
    
    while (data[index] != ' '){
        conductivity += data[index];
        ++index;
    }
    
    raw_conductivity = conductivity.toInt();
    
    if (raw_conductivity <= 700){
        return (double)raw_conductivity / 100.0;
    }
    else if (raw_conductivity != 1023) {
        return (double)(700 + 5 * (raw_conductivity - 700)) / 100;
    }
    else {
        return 1023;
    }
    return 42;
}

double Decagon5TE::calculateTemperature(char data[]) {
    String temperature = "";
    int index = 0;
    
    int raw_temperature = 0;
    
    for (int i = 0; i < 3; ++i){
        while (data[index] != ' '){
            ++index;
        }
        ++index;
    }
    
    while (data[index] != 'z'){
        temperature += data[index];
        
        ++index;
    }
    
    raw_temperature = temperature.toInt();
    
    if (raw_temperature > 900 && raw_temperature != 1023){
        raw_temperature = 900.0 + 5.0 * (raw_temperature - 900.0);
    }
    else if (raw_temperature == 1023) {
        return 1023;
    }
    
    double celsius = (double)(raw_temperature - 400) / 10.0;
    return celsius;
}
