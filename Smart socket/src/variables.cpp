#include <define.h>

//TFT_ILI9163C tft = TFT_ILI9163C(__CS, __DC, __RST);
//Adafruit_ST7735 tft = Adafruit_ST7735(__CS, __DC, __RST);
//Arduino_ST7789 tft = Arduino_ST7789(__DC, __RST);
//TFT_eSPI tft = TFT_eSPI();
OneWire oneWire(TEMP_SENSOR_PIN);   
DallasTemperature temp_sensor(&oneWire);
DeviceAddress sensor_addr = {0x3C, 0x2B, 0xD, 0x40, 0x3C, 0x7A, 0xF9, 0x3F};

//const char *ssid = "Beeline_2G_FF8F84";
//const char *pass = "kfNKgKA5g6"; 
const char *ssid = "SpaceX";
const char *pass = "#Challenge2020"; 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;
const int   daylightOffset_sec = 0;

struct tm Time;

unsigned int localPort = 2000;
IPAddress SendIP(192,168,0,255);
AsyncUDP udp;
GButton connect_btn(36);
GButton butt1(39);

//---------Mesurments---------
bool get_ADC = false;
bool calc_RMS = false;
unsigned long volts[100];
unsigned long amps[100];
float rms_volts[5];
float rms_amps[5];
float rms_volt[10];
float rms_amp[10];
float disp_volt = 0;
float disp_amp = 0;
unsigned long zero_volt = 1653;
unsigned long zero_volt_tmp = 0;
unsigned long zero_amp = 1295;
unsigned long zero_amp_tmp = 0;
float momental_amp = 0;
float momental_amp2 = 0;
unsigned int current_debounce_time = 0;
int wats=0;
float watt_hours=0;
float minute_awr_wats[60];
float minute_watts_hours[60];
int last_minute = 0;
int last_second = 0;
//----------------------------

//-----power regulation-------
int power_status = 0;
/* starting: 1
 *  working: 2
 *  slowing: 3
 */
unsigned long speed_up_time = 3000;
int cur_power = 0;
int last_powere = 0;
int max_power = 255;
bool vol_on_output = 1; 
unsigned long speed_up_timer = 0; 
unsigned long time_till_open = 0;
unsigned long time_till_open_last = 0;
unsigned long debounce = 0;
bool sem_on = false;
bool overload = false;
int overload_wats = 0;
bool shut_by_overload = false;
int last_wats = 0;
int last_power = 0;
float load_KP = 0.1;
float load_KD = 0.05;
float load_KI = 0.05;
int load_err_sum = 0;
int load_err_last = 0;
//----------------------------

//----------groups------------
unsigned long int slave_search_time = 0;
unsigned long int ready_to_become_slave_time = 0;
unsigned long int master_online_time = 0;
IPAddress slaves_ip[10];
String slaves_name[10];
IPAddress master_ip;
String master_name;
short slaves_number = 0;
IPAddress app_ip;
bool slave_led_state = 0;
bool master_led_state = 0;
unsigned long int blink_time = 0;
bool slave_led_blink = 0;
bool master_led_blink = 0;
bool master_check = 0;
//----------------------------

//----------parameters--------
#if (MODULE == 0)
  String name = "Node0";
#else if (MODULE == 1)
  String name = "Node1";
#endif
bool unstable_load = false;
bool on_when_master_on = 1;
unsigned int delay_before_off = 3000;
unsigned int delay_before_on = 1000;
unsigned long delay_before_off_time = 0;
unsigned long delay_before_on_time = 0;
int power_treshold = 0;
int master_power = 0;
unsigned int max_consumption = 2000;
int max_load[10][6];
int max_loads_count = 1;
int curent_max_load[6];
bool connected_to_grid_tmp = false;
bool connected_to_grid = false;
bool connected_to_grid_last = false;
bool btn_off = 1;
int statuse = 0;
/*
 * 0 - normal working
 * 1 - master searching for slaves
 * 2 - ready to become slave
 */
int role = 0;
 /* 
  * 0 - normal working  
  * 1 - master
  * 2 - slave
  */
//----------------------------

char packetBuffer[9];
bool connected_to_app = 0;
String message = "";
bool send_start_signal = 0;
bool send_stop_signal = 0;
bool to_stop = 0;
bool to_start = 0;
hw_timer_t * timer = NULL;
hw_timer_t * timer2 = NULL;
unsigned long display_val = 0;
unsigned long display_val2 = 0;
unsigned long timee2 = 0;
unsigned long value0, value1;
int croses=0;
unsigned long freq_time=0;
float freq=0;
bool overvoltage = false;
int max_voltage = 300;
int bat_charge = 0;

int chart_period = 0;
int chart_type = 0;

unsigned long sensor_ask_time = 0;
bool controll_by_sensor = false;
int target_temperature = 40;
float sensor_temperature = 0;
float temperature_last_err = 0;
float temperature_err_sum = 0;

float temperature_KP = 20;
float temperature_KD = 8;
float temperature_KI = 4;

int count[4];

//unsigned long count = 0;
boolean flag = 0; 

int tasks = 0;

bool flage = 0;