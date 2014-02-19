<<<<<<< HEAD
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
    Decagon5TE(int excite_pin, int serial_pin, unsigned long update_period);
    
    void readData();
    
    boolean isReadyForReading();
    
    double getDielectricPermittivity() {return dielectric_permittivity;}
    double getElectricalConductivity() {return electrical_conductivity;}
    double getTemperature() {return temperature;}
    
private:
    const static boolean DEBUG = false;
    
    unsigned long update_period;
    
    int excitation_pin;
    int serial_control_pin;
    
    unsigned long last_update_millis;
    boolean ready_for_reading;
    
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
=======
/*
 Decagon5TE.h - Library for reading data from a Decagon 5TE.
 Created by Chris A. Rice, May 17, 2013.
 Released into the public domain.
 */

#ifndef Decagon5TE_h
#define Decagon5TE_h

#include "Arduino.h"
#include "Reading.h"

class Decagon5TE {
public:
    Decagon5TE();
    Decagon5TE(int excite_pin, int serial_pin);
    Decagon5TE(int excite_pin, int serial_pin, unsigned long update_period);
    Decagon5TE(int serial_line, int excite_pin, int serial_pin);
    
    Reading readData(String time);
    
    boolean isReadyForReading();
    
    byte getID() { return serial_line;}
    
    void begin();
    
private:
    const static boolean DEBUG = false;
    const static double DRY_THRESHOLD = 25.0;
    
    const static long DRY_UPDATE_PERIOD = 360000;
    const static long WET_UPDATE_PERIOD = 60000;
    const static unsigned int check_period = 10000;
    
    unsigned long update_period;
    
    Reading current_reading;
    
    int excitation_pin;
    int serial_control_pin;
    
    byte serial_line;
    HardwareSerial* serial_port;
    
    unsigned long last_update_millis;
    unsigned long last_check_millis;
    boolean ready_for_reading;
    
    void exciteSensor();
    void cycleSerialLine();
    
    void checkMoisture();
    
    double calculateDielectricPermittivty(char data[]);
    double calculateElectricalConductivity(char data[]);
    double calculateTemperature(char data[]);
    
    String doubleToString(double input);
};

#endif
>>>>>>> bf08569b2fc4a5403d0f7368db9f24cc9603e31f
