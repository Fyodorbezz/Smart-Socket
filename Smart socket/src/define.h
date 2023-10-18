#define MODULE 0
#define DISPLAY 1

#include <Arduino.h>
#include <WiFi.h>
#include <AsyncUDP.h>
#include "GyverButton.h"
#include "time.h"
//#include <Adafruit_GFX.h>
//#include <TFT_ILI9163C.h>
#include <Adafruit_ST7735.h>
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

extern Adafruit_ST7735 tft;
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
//----------------------------

//-----power regulation-------
extern unsigned long speed_up_time;
extern int cur_power;
extern int last_powere;
extern int max_power;
extern bool vol_on_output; 
extern unsigned long speed_up_timer; 
extern unsigned long time_till_open;
extern unsigned long time_till_open_last;
extern unsigned long debounce;
extern bool sem_on;
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
extern IPAddress slaves_ip[10];
extern String slaves_name[10];
extern IPAddress master_ip;
extern String master_name;
extern short slaves_number;
extern IPAddress app_ip;
extern bool slave_led_state;
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
extern int croses;
extern unsigned long freq_time;
extern float freq;
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

extern int count[4];

//extern unsigned long count;
extern boolean flag; 

extern int tasks;

extern bool flage;

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