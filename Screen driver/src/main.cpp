#include <Arduino.h>
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Arduino_ST7735_Fast.h>
#include <microDS18B20.h>
#include <SoftwareSerial.h>



#define SCR_WD   128
#define SCR_HT   128



#define DISPLAY 0

#define	BLACK   0x0000
//#define	BLUE    0x001F
#define	BLUE    	0xF800
#define	RED     	0x001F
#define DARK_RED 	0x0013	
#define	GREEN   0x07E0
#define DARK_GREEN 	0x04C0
#define CYAN    0xFFE0
#define MAGENTA 0xF81F
#define DARK_MAGENTA 	0x9813
#define LIGHT_YELLOW 	0x9FFF
#define YELLOW  0x07FF 
#define DARK_YELLOW 0x04D3
#define WHITE   0xFFFF
#define ORANGE 0x041F

#define PWR_LED 2
#define ON_LED A0
#define GRID_LED A1
#define ERR_LED A2
#define MAS_LED A3
#define PER_LED A4
#define APP_LED A5
#define TFT_CS_PIN 10
#define TFT_DC_PIN  8
#define TFT_RST_PIN 9
#define TFT_LED_PIN 3

#define TEMP_SENS_PIN 9
#define  TRIAC_TEMP_SENS_PIN A7


Arduino_ST7735 tft = Arduino_ST7735(TFT_DC_PIN, TFT_RST_PIN, TFT_CS_PIN);
MicroDS18B20<5> temp_sensor;
SoftwareSerial data(6, 7); 

float disp_volt = 0;
float disp_amp = 0;
int wats=0;
float watt_hours=0;
int hour = 0;
int min = 0;
int sec = 0;
int power = 0;
bool btn_off = 0;
int curent_max_load[6];
int role = 0;
String name = "";
bool unstable_load = 0;
int max_voltage = 0;
bool on_when_master_on = 0;
int delay_before_off = 0;
int bat_charge = 0;

unsigned long request_timer=0;

void draw_grid();
void update_parameters();
void update_time();
void update_power();
void update_load_limit();
void update_overvoltage();
void update_connection();
void update_name();
void update_battery();

 
void setup() {
  Serial.begin(115200);
  data.begin(2400);
  Serial.setTimeout(50);
  data.setTimeout(50);
  tft.init();
  tft.fillScreen(BLACK);
  tft.setTextColor(BLACK);
  tft.setRotation(2);
  
  draw_grid();
  update_parameters();
  update_time();
  update_power();
  update_load_limit();
  update_overvoltage();
  update_connection();
  update_name();

  pinMode(A0, OUTPUT);
  digitalWrite(A0, false);
  pinMode(A1, OUTPUT);
  digitalWrite(A1, false);
}

void loop() {
  if(millis() - request_timer > 500){
    request_timer = millis();
    if(temp_sensor.readTemp()){
      data.println("Temp_sensor;" + String(temp_sensor.getTemp()));
    }
    temp_sensor.requestTemp();

    data.println("Triac_temp;" + String(analogRead(TRIAC_TEMP_SENS_PIN)));
  }

  if (data.available()) {
    String message = "";
    int tmp = 0;
    while (data.available()){
      message += (char)data.read();
      if(message[tmp] == '\n'){
        break;
      }
      tmp ++;
      delay(2);
    }
    Serial.println(message);
    int count = 0;
    int count2 = 0;
    String message_split[10];
    for(int i=0; i < message.length(); i++){
      if(message[i] == ';'){
        count ++; 
      }
      else{
        message_split[count] += message[i];
      }
    }
    if(message_split[0] == "Parameters"){
      disp_volt = message_split[1].toFloat();
      disp_amp = message_split[2].toFloat();
      wats = message_split[3].toInt();
      watt_hours = message_split[4].toFloat(); 
      update_parameters();  
    }
    else if(message_split[0] == "Time"){
      hour = message_split[1].toInt();
      min = message_split[2].toInt();
      sec = message_split[3].toInt();
      update_time();
    }
    else if(message_split[0] == "Power"){
      btn_off = message_split[1].toInt();
      unstable_load = message_split[2].toInt();
      power = message_split[3].toInt();
      update_power();
    }
    else if(message_split[0] == "Load_limit"){
      for(int i=0; i<6; i++){
        curent_max_load[i] = message_split[i+1].toInt();
      }
      update_load_limit();
    }
    else if(message_split[0] == "Voltage_limit"){
      max_voltage = message_split[1].toInt();
      update_overvoltage();
    }
    else if(message_split[0] == "Connection"){
      role = message_split[1].toInt();
      on_when_master_on = message_split[2].toInt();
      delay_before_off = message_split[3].toInt();
      update_connection();
    }
    else if(message_split[0] == "Name"){
      name = message_split[1];
      update_name();
    }
    else if(message_split[0] == "Battery"){
      bat_charge = message_split[1].toInt();
      update_battery();
    }
    else if(message_split[0] == "On"){
      digitalWrite(ON_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "Grid"){
      digitalWrite(GRID_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "Pwr"){
      analogWrite(PWR_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "Err"){
      digitalWrite(ERR_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "App"){
      digitalWrite(APP_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "Mas"){
      digitalWrite(MAS_LED, message_split[1].toInt());
    }
    else if(message_split[0] == "Per"){
      digitalWrite(PER_LED, message_split[1].toInt());
    }
  }
}



void draw_grid(){
  #if (DISPLAY == 0)
    tft.drawLine(0, 0, 0, 128, LIGHT_YELLOW);
    tft.drawLine(127, 0, 127, 128, LIGHT_YELLOW);
    tft.drawLine(0, 0, 128, 0, LIGHT_YELLOW);
    tft.drawLine(0, 127, 128, 127, LIGHT_YELLOW);

    //tft.drawLine(0, 9, 128, 9, LIGHT_YELLOW);
    int y = 12;
    String display_string = "";

    tft.drawLine(56, y-2, 56, 91, LIGHT_YELLOW);
    tft.drawLine(0, y-2, 128, y-2, LIGHT_YELLOW);
    tft.setCursor(2, y);
    tft.setTextColor(RED, BLACK);  
    tft.setTextSize(1);
    tft.print("Voltage");
    y += 9;
    y += 18;

    tft.drawLine(0, y-2, 56, y-2, LIGHT_YELLOW);
    tft.setCursor(2, y);
    tft.setTextColor(BLUE, BLACK);  
    tft.setTextSize(1);
    tft.print("Current");
    y += 9;
    y += 18;

    tft.drawLine(0, y-2, 56, y-2, LIGHT_YELLOW);
    tft.setCursor(2, y);
    tft.setTextColor(YELLOW, BLACK);  
    tft.setTextSize(1);
    tft.print("Power");
    y += 9;
    y += 18;

    tft.drawLine(0, y-2, 56  , y-2, LIGHT_YELLOW);
    tft.setCursor(2, y);
    tft.setTextColor(GREEN, BLACK);  
    tft.setTextSize(1);
    tft.print("Energy");
    y += 9;
    y += 18;

    tft.drawLine(80, 92, 80, y-2, LIGHT_YELLOW);
    tft.drawLine(0, y-2, 79, y-2, LIGHT_YELLOW);

    y = 12;
    tft.setTextColor(MAGENTA, BLACK);
    tft.setCursor(72, y);
    tft.setTextSize(1);
    tft.print("Time");
    y+=9;
    y+=18;
    tft.drawLine(56, y-2, 128, y-2, LIGHT_YELLOW);
    tft.setTextColor(YELLOW, BLACK);
    display_string = "Power limit";
    tft.setCursor(57 + (70 - display_string.length()*6)/2, y);
    tft.setTextSize(1);
    tft.print(display_string);
    y += 29;
    tft.drawLine(56, y-2, 128, y-2, LIGHT_YELLOW);
    y += 10;
    tft.drawLine(56, y-2, 128, y-2, LIGHT_YELLOW);
    y +=15;
    tft.drawLine(56, y-2, 128, y-2, LIGHT_YELLOW);
    tft.setTextColor(CYAN, BLACK);
    display_string = "Status";
    tft.setCursor(81 + (49 - display_string.length()*6)/2, y);
    tft.print(display_string);
  #endif
}

void update_parameters(){
  #if (DISPLAY == 0)
    int y = 21;
    String display_string = "";
    String display_string2 = "";


    display_string = String(int(disp_volt));
    display_string2 = "";
    tft.setCursor(2, y);
    tft.setTextColor(DARK_RED, BLACK);
    tft.setTextSize(2);
    for(int i=0; i<3-display_string.length(); i++){
      display_string2 += "0";
    }
    tft.print(display_string2);
    tft.setTextColor(RED, BLACK); 
    tft.print(display_string);
    tft.setCursor(tft.getCursorX(), y+8);
    tft.setTextSize(1);
    tft.print("V");
    y += 27;

    tft.setTextColor(BLUE, BLACK);
    display_string = String(disp_amp);
    tft.setCursor(2, y);
    tft.setTextSize(2);
    tft.print(display_string);
    tft.setCursor(tft.getCursorX(), y+8);
    tft.setTextSize(1);
    tft.print("A");
    y += 27;

    display_string = String(int(wats));
    tft.setCursor(2, y);
    tft.setTextColor(DARK_YELLOW, BLACK);
    tft.setTextSize(2);
    for(int i=0; i<4-display_string.length(); i++){
      tft.print("0");
    }
    tft.setTextColor(YELLOW, BLACK); 
    tft.print(display_string);
    tft.setCursor(tft.getCursorX(), y+8);
    tft.setTextSize(1);
    tft.print("W");
    y += 27;

    if(watt_hours >= 1000){
      display_string = String(int(watt_hours));
    }
    else if(watt_hours >= 100){
      display_string = String(int(watt_hours*10)/10.0);
    }
    else{
      display_string = String(int(watt_hours*100)/100.0);
    }
    tft.setCursor(2, y);
    tft.setTextColor(DARK_GREEN, BLACK);
    tft.setTextSize(2);
    display_string2 = "";
    for(int i=0; i<5-display_string.length(); i++){
      display_string2 += "0";
    }
    tft.print(display_string2);
    tft.setTextColor(GREEN, BLACK); 
    tft.print(display_string);
    tft.setCursor(tft.getCursorX(), y+8);
    tft.setTextSize(1);
    tft.print("W/H");
    y+=18;
  #endif
}

void update_time(){
  #if (DISPLAY == 0)
    int y = 21;
    String display_string = "";

    display_string = String(hour);
    tft.setCursor(57, y);
    tft.setTextSize(2);
    tft.setTextColor(DARK_MAGENTA, BLACK);
    for(int i=0; i < 2-display_string.length(); i++){
     tft.print("0");
    }
    tft.setTextColor(MAGENTA, BLACK);
    tft.print(hour);
    tft.setCursor(tft.getCursorX()-3, y);
    tft.setTextColor(MAGENTA);
    tft.print(":");
    tft.setTextColor(MAGENTA, BLACK);
    tft.setCursor(tft.getCursorX()-3, y);
    for(int i=0; i<(2-String(min).length()); i++){
      tft.print("0");
    }
    tft.print(min);
    y+=8;

    tft.setCursor(tft.getCursorX(), y);
    tft.setTextSize(1);
    tft.setCursor(tft.getCursorX()-1, y);
    tft.print(":");
    tft.setCursor(tft.getCursorX()-1, y);
    for(int i=0; i<(2-String(sec).length()); i++){
      tft.print("0");
    }
    tft.print(sec);
    y+=10;
  #endif
}

void update_power(){
  #if (DISPLAY == 0)
    int y = 78;
    if(btn_off){
      tft.fillCircle(64, y+6, 6, RED);
    }
    else{
      tft.fillCircle(64, y+6, 6, GREEN);
    }

    if(!unstable_load){    
      tft.fillRoundRect(74, y+4, int(power / 5.0), 5, 2, ORANGE);
      tft.drawRoundRect(73, y+4, 53, 5, 2, ORANGE);
      tft.fillRect(74+int(power / 5.0), y+5, 51-int(power / 5.0), 3, BLACK);
    }
    else{
      tft.fillRect(74, y+4, 51, 5, BLACK);
    }
  #endif
}

void update_load_limit(){
  #if (DISPLAY == 0)
    int y = 48;
    String display_string;

    tft.setTextColor(YELLOW, BLACK);
    if(curent_max_load[0] == -1){
      tft.setCursor(65, y);
      tft.print("None");
      y+=10;
    }
    else if(curent_max_load[1] == -1){
      display_string = String(curent_max_load[0]) + " wats";
      tft.setCursor(57 + (70 - display_string.length()*6)/2, y);
      tft.print(display_string);
      display_string =  "All day";
      tft.setCursor(57 + (70 - display_string.length()*6)/2, y+9);
      tft.print(display_string);
      y+=20;
    }
    else{
      display_string = String(curent_max_load[0]) + " wats";
      tft.setCursor(57 + (70 - display_string.length()*6)/2, y);
      tft.print(display_string);
      display_string = String(curent_max_load[1]) + ":" + String(curent_max_load[2]) + "-" + String(curent_max_load[3]) + ":" + String(curent_max_load[4]);
      tft.setCursor(57 + (70 - display_string.length()*6)/2, y+9);
      tft.print(display_string);
      y+=20;
    }
  #endif
}

void update_overvoltage(){
  #if (DISPLAY == 0)
    int y = 68;
    String display_string;

    tft.setTextColor(RED, BLACK);
    display_string = String(max_voltage) + "V max";
    tft.setCursor(57 + (70 - display_string.length()*6)/2, y);
    tft.print(display_string);
    y+=10;
  #endif
}

void update_connection(){
  #if (DISPLAY == 0)
    int y = 102;
    String display_string;
    tft.fillRect(81, y, 45, 16, BLACK);
    if(role == 0){
      display_string = "Single";
    }
    else if(role == 1){
      display_string = "Master";
    }
    else{
      display_string = "Slave";
    }
    tft.setTextColor(CYAN, BLACK);
    tft.setCursor(80 + (49 - display_string.length()*6)/2, y);
    tft.print(display_string);
    y+=9;

    if(role == 2 && on_when_master_on == 1){

      display_string = "Sinked";
      tft.setCursor(80 + (49 - display_string.length()*6)/2, y);
      tft.print(display_string);
      y+=9;
      display_string = "Extra work time:" + String(round(delay_before_off/100.0) / 10.0) + " S";
      tft.setCursor((128 - display_string.length()*6)/2, y);
      tft.print(display_string);
      y+=9;
    }
    else{
      tft.fillRect(81, y, 37, 16, BLACK);
      y+=9;
      tft.fillRect(1, y, 126 , 7, BLACK);
    }
  #endif
}

void update_name(){
  #if (DISPLAY == 0)
    int y = 2;
    String display_string;
    display_string = "Name: " + name;
    tft.setCursor((128-(display_string.length()*6))/2, y);
    tft.setTextColor(CYAN, BLACK);
    tft.setTextSize(1);
    tft.print(display_string);
    y += 10;
  #endif
}

void update_battery(){
  #if (DISPLAY == 0)
    tft.drawRect(111, 4, 2, 2, GREEN);
    tft.fillRoundRect(114 + (10 - int(bat_charge / 10.0)), 3, int(bat_charge / 10.0), 4, 0, GREEN);
    tft.drawRoundRect(113, 2, 12, 6, 2, GREEN);
    tft.fillRect(114 , 3, 10 - int(bat_charge / 10.0), 4, BLACK);
  #endif
}