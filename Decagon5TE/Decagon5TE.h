/*
  Decagon5TE.h - Library for reading data from a Decagon 5TE.
  Created by Chris A. Rice, May 17, 2013.
  Released into the public domain.
*/

#ifndef Decagon5TE_h
#define Decagon5TE_h

#include "Arduino.h"
class Decagon5TE {
	public:
		Decagon5TE();
		Decagon5TE(int excite_pin, int serial_pin);
		
		void readData();

		double getDielectricPermittivity() {return dielectric_permittivity;}
		double getElectricalConductivity() {return electrical_conductivity;}
		double getTemperature() {return temperature;}

	private:
		int excitation_pin;
		int serial_control_pin;		

		boolean ready_for_update;		

		double dielectric_permittivity;
		double electrical_conductivity;
		double temperature;

		void exciteSensor();
		void cycleSerialLine();
		
		double calculateDielectricPermittivty(char data[]);
		double calculateElectricalConductivity(char data[]);
		double calculateTemperature(char data[]);
};

#endif
