#define MODULE 0

#include <Arduino.h>
#include <driver/adc.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include "GyverButton.h"
#include "time.h"
//#include <Adafruit_GFX.h>
//#include <TFT_ILI9163C.h>
//#include <Adafruit_ST7735.h>
#include "OneWire.h"
#include "DallasTemperature.h"



#define SLAVE_SEARCH_TIMEOUT 500
#define READY_TO_BECOME_SLAVE_TIMEOUT 5000
#define MASTER_ONLINE_TIMEOUT 10000
#define ANSWER_WAIT_TIMEOUT 1000
#define SENSOR_ASK_TIMEOUT 500
#define CURRENT_DEBOUNCE_TIMEOUT 1000

#define SAMPLE_LENGHT 50  


#define __CS 15
#define __DC 2
#define __RST 4

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

#define TEMP_SENSOR_PIN 13

#define CUR_SENSOR_PIN 34
#define VOLTAGE_SENSOR_PIN 35
#define BATERY_VOLTAGE_PIN 25
#define ZERO_CROSS_PIN 27
#define TRIAC_PIN 26
#define NUTRAL_RELAY_PIN 32
#define LINE_REALY_PIN 33
#define FAN_PIN 14
#define ATMEGA_RES_PIN 4
#define CNCT_BTN_PIN 36
#define PWR_BTN_PIN 39

//extern Adafruit_ST7735 tft;
//extern TFT_ILI9163C tft;
//extern Arduino_ST7789 tft;
//extern TFT_eSPI tft;
extern OneWire oneWire;
extern DallasTemperature temp_sensor;
extern DeviceAddress sensor_addr;

extern const char *ssid;
extern const char *pass; 
extern const char* ntpServer;
extern const long  gmtOffset_sec;
extern const int   daylightOffset_sec;
extern struct tm Time;

extern unsigned int localPort;
extern IPAddress SendIP;
extern AsyncUDP udp;
extern GButton connect_btn;
extern GButton butt1;

//---------Mesurments---------
extern boolean get_ADC;
extern boolean calc_RMS;
extern unsigned long volts[100];
extern unsigned long amps[100];
extern float rms_volts[5];
extern float rms_amps[5];
extern float rms_volt[10];
extern float rms_amp[10];
extern float disp_volt;
extern float disp_amp;
extern unsigned long zero_volt;
extern unsigned long zero_volt_tmp;
extern unsigned long zero_amp;
extern unsigned long zero_amp_tmp;
extern float momental_amp;
extern float momental_amp2;
extern unsigned int current_debounce_time;
extern int wats;
extern float watt_hours;
extern float minute_awr_wats[60];
extern float minute_watts_hours[60];
extern int last_minute;
extern int last_second;
extern short sine_waves_count;
//----------------------------

//-----power regulation-------
extern unsigned long speed_up_time;
extern int cur_power;
extern int last_powere;
extern int max_power;
extern bool vol_on_output; 
extern unsigned long power_change_timer; 
extern bool overload;
extern int overload_wats;
extern bool shut_by_overload;
extern int last_wats;
extern int last_power;
extern float load_KP;
extern float load_KD;
extern float load_KI;
extern int load_err_sum;
extern int load_err_last;
//----------------------------

//----------groups------------
extern unsigned long int slave_search_time;
extern unsigned long int ready_to_become_slave_time;
extern unsigned long int master_online_time;
extern IPAddress perefirals_ip[10];
extern String perefirals_name[10];
extern short pereferals_load[10];
extern IPAddress master_ip;
extern String master_name;
extern short periferals_number;
extern IPAddress app_ip;
extern bool periferal_led_state;
extern bool master_led_state;
extern unsigned long int blink_time;
extern bool slave_led_blink;
extern bool master_led_blink;
extern bool master_check;
//----------------------------

//----------parameters--------
extern String name;
extern bool unstable_load;
extern bool on_when_master_on;
extern unsigned int delay_before_off;
extern unsigned int delay_before_on;
extern unsigned long delay_before_off_time;
extern unsigned long delay_before_on_time;
extern unsigned int max_consumption;
extern int max_load[10][6];
extern int max_loads_count;
extern int curent_max_load[6];
extern bool connected_to_grid_tmp;
extern bool connected_to_grid;
extern bool connected_to_grid_last;
extern bool btn_off;
extern int statuse;
/*
 * 0 - normal working
 * 1 - master searching for slaves
 * 2 - ready to become slave
 */
 extern int role;
 /* 
  * 0 - normal working  
  * 1 - master
  * 2 - slave
  */
//----------------------------

extern char packetBuffer[9];
extern bool connected_to_app;
extern String message;
extern bool send_start_signal;
extern bool send_stop_signal;
extern bool to_stop;
extern bool to_start;
extern int power_treshold;
extern int master_power;
extern hw_timer_t * timer;
extern hw_timer_t * timer2;
extern unsigned long display_val;
extern unsigned long display_val2;
extern unsigned long timee2;
extern unsigned long value0, value1;
extern bool overvoltage;
extern int max_voltage;
extern int bat_charge;

extern int power_status;
/* starting: 1
 *  working: 2
 *  slowing: 3
 */

extern int chart_period;
extern int chart_type;

extern unsigned long sensor_ask_time;
extern bool controll_by_sensor;
extern int target_temperature;
extern float sensor_temperature;
extern float temperature_last_err;
extern float temperature_err_sum;

extern float temperature_KP;
extern float temperature_KD;
extern float temperature_KI;

extern bool flag; 
extern int tasks;

extern unsigned long long timere;
extern unsigned int dur;
//extern int raw_data_count;

void parsePacket(AsyncUDPPacket packet);
void send_data_over_UDP(String mes_str, IPAddress ip,int localPort);
bool str_to_bool(String message, int index);
void power_off();
void power_on();
void Dim_ISR();
void get_data();
void zero_crossed();
void calculate_RMS();
void filter_RMS();
void display_values(int time_pass);
float compute_Volts(float data);
float compute_Volts_2(float data);
void send_consumption();
void change_role(int role);
//bool overload(int load);
bool compare_2_times(int hour1, int min1, int hour2, int min2);
void update_screen();
void calculate_current_power_limit();
void send_max_load();
void send_role();
void draw_grid();
void update_parameters();
void update_time();
void update_power();
void update_load_limit();
void update_overvoltage();
void update_connection();
void update_name();
void update_leds();

struct Mil_Timer{
  unsigned long long timer_counter=0;
  unsigned int timer_period=0;
  bool timer_started = 0;

  Mil_Timer(){}

  Mil_Timer(unsigned long long period){
    timer_period = period;
  }

  void set_period(unsigned long long period){
    timer_period = period;
  }

  void restart(){
    timer_counter=millis();
    timer_started = 1;
  }

  bool is_started(){
    timer_started = !((millis()-timer_counter) > timer_period);
    return timer_started;
  }

  bool is_ready(){
    timer_started = !((millis()-timer_counter) > timer_period);
    return (millis()-timer_counter) > timer_period;
  }

};

struct PID{
  float kP = 0.5;
  float kD = 2;
  float kI = 0.5;

  float last_error = 0;
  float integral_sum = 0;

  int get_result(int error){
    float P = error * kP;
    float D = (error - last_error) * kD;
    float I = integral_sum * kI;
    last_error = error;
    integral_sum += error;
    if (integral_sum > 1000){
      integral_sum = 1000;
    }
    if (integral_sum < -1000){
      integral_sum = -1000;
    }
    return int(P+D+I+0.5);
  }
  
};

struct Measurments{
private:
  

public:

  unsigned int raw_data[200];
  float rms_data[5];
  float rms_data_filtered[5];
  float final_data=0;
  short peek_value=0;
  int raw_data_count=0;

  unsigned int raw_data_tmp[200];
  int raw_data_tmp_count=0;

  void cache_data(){
    for(int i=0; i < raw_data_count; i++){
      raw_data_tmp[i] = raw_data[i];
    }
    raw_data_tmp_count = raw_data_count;
    raw_data_count = 0;
  }

  void calculate_rms(){
    for(int i=1; i < 5; i++){
      rms_data[i-1] = rms_data[i];
    }
    rms_data[4] = 0;
    double data_sum=0;
    for(int i=0; i < raw_data_tmp_count; i++){
      float tmp = compute_Volts(raw_data_tmp[i]);
      //Serial.print(raw_data_tmp[i]);
      //Serial.print(" ");
      //Serial.print(zero_volt);
      //Serial.print(" ");
      //Serial.println(tmp);
      data_sum += tmp*tmp;
    }
    rms_data[4] = sqrt(data_sum/raw_data_tmp_count);
    //Serial.println(rms_data[4]);
    //Serial.println(" ");
  }

  void median_filter(){
    for(int i=1;i<5;i++){
      rms_data_filtered[i-1] = rms_data_filtered[i];
    }

    for (int i=0; i<5; i++){
      for (int j=0; j<5-i-1; j++){
        if (rms_data[j] > rms_data[j+1]){
          float tmp = rms_data[j];
          rms_data[j] = rms_data[j+1];
          rms_data[j+1] = tmp;
        }
      }
    }

    rms_data_filtered[4] = rms_data[2];
  }

  void avg_filter(){
    final_data = 0;
    for (int i=0;i<5;i++){
      final_data += rms_data_filtered[i];
    }
    final_data = final_data/5.0;
  }
};

extern Measurments current;
extern Measurments voltage;

struct Power_controll{
  private:

  bool load_connected=0;
  Mil_Timer power_change_timer = Mil_Timer(speed_up_time/255.0);
  Mil_Timer zero_current_timer = Mil_Timer(CURRENT_DEBOUNCE_TIMEOUT);
  short cur_power_last = 0;
  short target_power=0;

  public:

  short cur_power=0;
  short max_power=255;
  bool power_state=0;
  //0-off
  //1-on
  bool overload = 0;
  bool dimm_load=0; // is load dimmerable?
  
  short overvoltage_value=0;
  bool overvoltage_state=0;

  bool send_power_on_signal=0;
  bool send_power_off_signal=0;

  void turn_on(){
    power_state = 1;
    load_connected = 1;
    send_power_on_signal = true;

    if(connected_to_app){
      uint8_t mes[] = "Smart socket on";
      udp.writeTo(mes, sizeof(mes), app_ip, localPort);
    }

    if(dimm_load){
      target_power = max_power;
      return;
    }
    cur_power = 255;
    target_power = 255;
  }

  void turn_off(){
    power_state = 0;
    overload = 0;
    send_power_off_signal = true;

    if(connected_to_app){
      uint8_t mes[] = "Smart socket off";
      udp.writeTo(mes, sizeof(mes), app_ip, localPort);
    }

    if(dimm_load){
      target_power = 0;
      return;
    }
    cur_power = 0;
    target_power = 0;
  }

  void update(){
    digitalWrite(LINE_REALY_PIN, !overvoltage_value);
    digitalWrite(NUTRAL_RELAY_PIN, !overvoltage_value);

    if(voltage.peek_value > overvoltage_value){
      overvoltage_state=1;
      if(connected_to_app){
        uint8_t mes[] = "Smart socket overvoltage";
        udp.writeTo(mes, sizeof(mes), app_ip, localPort);
      }
    }

    if(load_connected && power_state && dimm_load){
      target_power = max_power;
    }

    if (power_change_timer.is_ready()){
      if(cur_power != target_power && load_connected){
        if (target_power > cur_power){
          cur_power ++;
        }
        else if(target_power < cur_power){
          cur_power--;
        }
      }
      power_change_timer.restart();
    }

    if ((momental_amp >= 0.3 || momental_amp2 >= 0.2) && dimm_load && !load_connected){
      load_connected = 1;
      send_power_on_signal = true;
    }

    if (current.final_data < 0.1 && cur_power == target_power && load_connected && power_state && !zero_current_timer.is_started()){
      zero_current_timer.restart();
    }

    if (current.final_data < 0.1 && cur_power == target_power && zero_current_timer.is_ready() && load_connected && power_state){
      load_connected = 0;
      if (dimm_load){
        target_power = 50;
        cur_power = 50;
      }
      send_power_off_signal = true;
    }

    if(cur_power != cur_power_last){
      if(connected_to_app){
        String mes = "Smart socket power" + String(cur_power);
        send_data_over_UDP(mes, app_ip, localPort);
      }
      update_power();
      cur_power_last = cur_power;
    }
  }
};

extern Power_controll power_controll;

struct Group_power{
  int power_trashold;
  bool on_order;
  bool off_order;
  //short role = 0;
  short power_limit = 0;
  short power_before_overload = 0;
  short max_power = 0;

  Mil_Timer power_on_timer;
  Mil_Timer power_off_timer;

  PID load_limiter;

  void limit_power(){
    if(wats > power_limit || max_power > power_controll.max_power && power_controll.dimm_load){
      power_controll.max_power = 255 - load_limiter.get_result(power_limit - wats);
      if(power_controll.max_power > max_power){
        power_controll.max_power = max_power;
      }
      if(connected_to_app){
        uint8_t mes[] = "Smart socket overload";
        udp.writeTo(mes, sizeof(mes), app_ip, localPort);
      }
    }
  }

  void update(){
    if(off_order && role == 2 && power_off_timer.is_ready()){
      power_controll.turn_off();
      off_order = 0;
    }
    if(on_order && role == 2 && power_on_timer.is_ready()){
      power_controll.turn_on();
      off_order = 0;
    }

    if(power_limit > wats && !power_controll.dimm_load){
      power_before_overload = wats;
      power_controll.turn_off();
      if(connected_to_app){
        uint8_t mes[] = "Smart socket overload";
        udp.writeTo(mes, sizeof(mes), app_ip, localPort);
      }
      power_controll.overload = 1;
    }

    if(power_limit < power_before_overload && power_controll.overload && !power_controll.dimm_load){
      power_controll.turn_on();
      power_controll.overload = 0;
    }
  }
};

extern Group_power group_power;