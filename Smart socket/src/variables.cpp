#include <define.h>

//TFT_ILI9163C tft = TFT_ILI9163C(__CS, __DC, __RST);
//Adafruit_ST7735 tft = Adafruit_ST7735(__CS, __DC, __RST);
//Arduino_ST7789 tft = Arduino_ST7789(__DC, __RST);
//TFT_eSPI tft = TFT_eSPI();




OneWire oneWire(TEMP_SENSOR_PIN);   
//DallasTemperature temp_sensor(&oneWire);
DeviceAddress sensor_addr = {0x3C, 0x2B, 0xD, 0x40, 0x3C, 0x7A, 0xF9, 0x3F};

const char *ssid = "SpaceX";
const char *pass = "#Challenge2020"; 
const char* ntpServer = "pool.ntp.org";
const long  gmtOffset_sec = 10800;
const int   daylightOffset_sec = 0;

struct tm Time;

unsigned int localPort = 2000;
IPAddress SendIP(192,168,0,255);
AsyncUDP udp;
GButton connect_btn(CNCT_BTN_PIN);
GButton butt1(PWR_BTN_PIN);

//---------Mesurments---------
unsigned long zero_volt = 1653;
unsigned long long zero_volt_tmp = 0;
unsigned long zero_amp = 1295;
unsigned long long zero_amp_tmp = 0;
float momental_amp = 0;
float momental_amp2 = 0;
int wats=0;
float watt_hours=0;
float minute_awr_wats[60];
float minute_watts_hours[60];
int last_minute = 0;
int last_second = 0;
short sine_waves_count = 0;
volatile int peek_voltage = 0;
//----------------------------
unsigned long speed_up_time = 3000;
//----------------------------

//----------groups------------
unsigned long int slave_search_time = 0;
unsigned long int ready_to_become_slave_time = 0;
unsigned long int master_online_time = 0;
IPAddress perefirals_ip[10];
String perefirals_name[10];
IPAddress master_ip;
String master_name;
short pereferals_load[10];
short periferals_number = 0;

IPAddress app_ip;
bool periferal_led_state = 0;
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
bool on_when_master_on = 1;
unsigned int max_consumption = 2000;
int max_load[10][6];
int max_loads_count = 1;
int curent_max_load[6];
bool connected_to_grid_tmp = false;
bool connected_to_grid = false;
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
hw_timer_t * timer = NULL;
hw_timer_t * timer2 = NULL;
unsigned long display_val = 0;
unsigned long display_val2 = 0;
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

bool flag = 0; 
int tasks = 0;
int raw_data_count = 0;

unsigned long long timere = 0;
unsigned int dur = 0;

Measurments current;
Measurments voltage;
Power_controll power_controll;
Group_power group_power;