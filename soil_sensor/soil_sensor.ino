#include <Decagon5TE.h>
#include <Reading.h>
#include <WiFi.h>
#include <SD.h>
#include <SPI.h>
#include <Wire.h>
#include <RTClib.h>
#include <RTC_DS3234.h>

const byte SD_CHIP_SELECT = 4;  // Constant for SD card reading, should be 4 on WiFi Shield

Decagon5TE sensor(1,34,35);  // (Serial Communication Pin, Sensor Excite Pin, Serial Reset Pin);
Decagon5TE sensor2(2,36,37);

RTC_DS3234 RTC(8);  // Creates a RTC(Real Time Clock) with pin number 8 for SS(Serial Selection)

WiFiClient client;  // Later used to upload data

byte status = WL_IDLE_STATUS; 
char ssid[] = "Apartment";  // SSID and password for the network you want to connect to
char password[] = "tenretni";

char to_upload_filename[] = "TOUP";  // Base filenames for the SD card, appended with
char datalog_filename[] = "DATALOG";  // corresponding sensor id later on.
char failed_upload_filename[] = "FAILUP";

void setup(){
  Serial.begin(1200);  //Start serial communications for debugging
  
  sensor.begin();  //Assign serial ports to sensors based on passed in value
  sensor2.begin();  //in the constructor
  
  SPI.begin();  // Initilizes SPI for WiFi shield and RTC
  SD_init();  
  
  connectToNetwork(ssid, password);
  
  RTC.begin();
  
  pinMode(53, OUTPUT);  // This must be done in order for the mega to play nicely with the WiFi Shield
  //RTC.adjust(DateTime(__DATE__, __TIME__));  // Sets date and time of the RTC to compile time
                                               // of the program, should only be done if the clock
                                               // is out of sync.
                            
  static char buf[32];  // Standard procedure for reading the time from the RTC
  Serial.print("Time: ");
  Serial.println(RTC.now().toString(buf,32));
  
  Serial.println("Finished Setup");  // Send out a message to verify everythign set up smoothly.
}

void loop(){
  if (sensor.isReadyForReading()){  // If its time to read from this sensor
    Serial.print("Reading data from sensor ");  
    Serial.println(sensor.getID());
    static char buf[32];
    Reading current_reading = sensor.readData(RTC.now().toString(buf,32)); // Read data and set the timestamp
    Serial.println(current_reading.toString());  // Display the data
    
    writeToSDCard(sensor.getID(), current_reading.toUploadString(), to_upload_filename);  //Write to to_upload
    writeToSDCard(sensor.getID(),current_reading.toString(), datalog_filename);  // and the datalog
    
  }
  else if (sensor2.isReadyForReading()){  // Same thing except for the second sensor
    Serial.print("Reading data from sensor ");
    Serial.println(sensor2.getID());
    static char buf[32];
    Reading current_reading = sensor2.readData(RTC.now().toString(buf,32));
    Serial.println(current_reading.toString());
    
    writeToSDCard(sensor2.getID(), current_reading.toUploadString(), to_upload_filename);
    writeToSDCard(sensor2.getID(),current_reading.toString(), datalog_filename);
    
  }
  else if (connectToNetwork(ssid, password)){  // If we are not reading from any of the sensors
    uploadData(sensor.getID());  // Upload our data, this will be optimized for power when this project is designed for 
    uploadData(sensor2.getID());  // deployability.
  }
  
  delay(5000);  // Wait some time to make sure we don't break anything.
}

//-------------
// SD Stuff
//-------------

String readFromSDCard(File& read_file){  // Reads one line at a time from the file and gives it back as a string
  String data_string = "";
  char data = '/0';
  while (read_file.available()){
    data = read_file.read();
    if ((int)data == 10){
      break;
    }
    else if ((int)data != 13){
      data_string += data;
    }
  }
  
  return data_string;
}

boolean writeToSDCard(byte sensor_id, String data_string, char filename[]){  // Writes to the corresponding file based on the sensor
  String temp_filename = filename;                                           // ID.
  temp_filename += sensor_id;
  temp_filename += ".csv";
  
  char name_buffer[32];
  temp_filename.toCharArray(name_buffer, 32);
  filename = name_buffer;
  
  SPI.setDataMode(SPI_MODE1);
  
  Serial.print(F("I'm going to write to "));
  Serial.println(filename);

  SPI.setDataMode(SPI_MODE0);
  // open the file. note that only one file can be open at a time, 
  // so you have to close this one before opening another.
  File dataFile = SD.open(filename, FILE_WRITE);

  delay(500);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(data_string);
    Serial.println(data_string);
    dataFile.close();
    return true;
    // print to the serial port too:
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.print(F("error opening "));
    Serial.println(filename);
    dataFile.close();  // always close the file
    return false;
  }
}

void appendFailedUploads(byte sensor_id) {  // Takes any data in the failed upload file and moves them to the to upload file
  String temp_filename = to_upload_filename;
  temp_filename += sensor_id;
  temp_filename += ".csv";
  
  char to_up_buffer[32];
  temp_filename.toCharArray(to_up_buffer, 32);
  
  temp_filename = failed_upload_filename;
  temp_filename += sensor_id;
  temp_filename += ".csv";
  
  char f_up_buffer[32];
  temp_filename.toCharArray(f_up_buffer, 32);
  
  SD.remove(to_up_buffer);
  
  File failed_file = SD.open(f_up_buffer, FILE_READ);
  File to_upload_file = SD.open(to_up_buffer, FILE_WRITE);
  
  if (failed_file && to_upload_file){
    while (failed_file.available()){
      to_upload_file.print((char)failed_file.read());
    }
  }   
  
  failed_file.close();
  to_upload_file.close();
  
  SD.remove(f_up_buffer);
}

boolean SD_init(){  // Starts the SD card and gets it ready for communication
  SPI.setDataMode(SPI_MODE0);
  Serial.println(F("Initializing SD card..."));
  // make sure that the default chip select pin is set to
  // output, even if you don't use it:
  pinMode(10, OUTPUT);

  // see if the card is present and can be initialized:
  if (!SD.begin(SD_CHIP_SELECT)) {
    Serial.println(F("Card failed, or not present"));
    // don't do anything more:
    return false;
  }
  Serial.println(F("card initialized."));
  return true;
}

//---------------
// Server Stuff
//---------------

void uploadData(byte sensor_id) {  // Mainly prepares the data for uploading
  Serial.print("I'm going to upload data from sensor ");
  Serial.println(sensor_id);
  
  SPI.setDataMode(SPI_MODE0);
  
  String temp_filename = to_upload_filename;
  temp_filename += sensor_id;
  temp_filename += ".csv";
  
  char filename[32];
  temp_filename.toCharArray(filename, 32);
    
  File data_file = SD.open(filename, FILE_READ);  // Open the to upload file corresponding to the sensor id
  String to_upload = "";
    
  for (int i = 0; data_file.available(); ++i){
    to_upload = readFromSDCard(data_file);
    Serial.print("Entry ");
    Serial.print(i);
    Serial.println(":");
      
    Serial.print('\t');
    Serial.println(to_upload);
      
    int rand = random(5);
    Serial.println(rand);
    if (!sendToPushingBox(to_upload, sensor_id)){  // If for some reason we can't upload it
      writeToSDCard(sensor_id, to_upload, failed_upload_filename);  // Write to the corresponding failed upload file
    }
  }
    
  data_file.close();
  appendFailedUploads(sensor_id);
  Serial.println("-----------------------");
}

boolean sendToPushingBox(String data, byte sensor_id) {
  
  String device_id = "";
  
  if (sensor_id == 1){
    device_id = "vC37774D3FBABA5E";  // Translates sensor ids to device ids for pushing box, determines which
  }                                  // spreadsheet the data will upload to.
  else if (sensor_id == 2){
    device_id = "v6CD616D7EAFCC88";
  }
  
  Serial.println(F("I'm going to upload to the spreadsheet"));

  boolean success = false;

  Serial.println(F("Starting connection to server..."));

  if (client.connect("api.pushingbox.com", 80)) {
    Serial.println(data);
    client.print("GET /pushingbox?devid=");
    client.print(device_id);
    client.print(data);
    client.println(" HTTP/1.1");
    client.println("Host:api.pushingbox.com");
    client.println("Connection: close");
    client.println();

    success = true;
    Serial.println(F("Upload Successful"));
  }
  else {
    success = false;
    Serial.println(F("Upload Failed"));
  }

  client.flush();  // This must be done or else the server hates it and kicks you.
  client.stop();

  delay(2500);

  return success;  // Did it work?
}

boolean connectToNetwork(char ssid[], char password[]){
  status = WiFi.status();

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println(F("WiFi shield not present"));
    // don't continue:
    return false;
  }
  int failures = 0;
  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED && failures < 2) {
    //Serial.println(failures);
    Serial.print(F("Attempting to connect to WPA SSID: "));
    Serial.println(ssid);
    // Connect to WPA/WPA2 network:
    status = WiFi.begin(ssid, password);
    ++failures;
  }
  if (failures > 2){
    return false;
  }
  else if (status == WL_CONNECTED) {
    Serial.print(F("Connected to "));
    Serial.println(WiFi.SSID());
  }
  return status;
}

