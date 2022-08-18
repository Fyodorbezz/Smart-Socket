#include <define.h>

void send_data_over_UDP(String mes_str, IPAddress ip,int localPort){
  uint8_t mes[1000];
  size_t data_len= mes_str.length();
  for(int i=0;i<mes_str.length();i++){
    mes[i] = char(mes_str[i]);
  }
  udp.writeTo(mes, data_len, ip, localPort);
}


bool str_to_bool(String message, int index){
  if(message.substring(index) == "true"){
     return 1;
  }
  else{
     return 0;
  }
}

void send_consumption(){
  String mes_str = "Smart socket consumption"+String(int(wats));
  send_data_over_UDP(mes_str, master_ip, localPort);
  //Serial.println("Load sended");
}


void change_role(int rolee){
  if (rolee == 0){
    for (int i=0;i<slaves_number;i++){
        slaves_ip[i].fromString("0.0.0.0");
    }
    slaves_number = 0;
    master_led_blink = 1;
    slave_led_state = 0;
    role = 0;
    slave_led_state = 0;
    master_check = 0;
    void send_role();
  }
  else if(rolee == 1){
    role = 1;
  }
  else if(rolee == 2){
    statuse = 0;
    role = 2;
    slave_led_blink = 0;
    slave_led_state = 1;
    void send_role();
  }
  update_connection();
  update_name();
}

void send_role(){
  if(connected_to_app){
      String mes_str = "Role"+role;
      send_data_over_UDP(mes_str, app_ip, localPort);
  }
}

void send_max_load(){
  for(int i=0; i<max_loads_count; i++){
    String mes_str = "Send max load";
    for(int j=0; j<6; j++){
      mes_str+= String(max_load[i][j]) + ";";
    }
    send_data_over_UDP(mes_str, app_ip, localPort);
  }
}

void draw_grid(){
}

void update_parameters(){
  Serial2.println("Parameters;" + String(disp_volt) + ";" + String(disp_amp) + ";" + String(wats) + ";" + String(watt_hours));
}

void update_time(){
  Serial2.println("Time;" + String(Time.tm_hour) + ";" + String(Time.tm_min) + ";" + String(Time.tm_sec));
}

void update_power(){
  Serial2.println("Power;" + String(btn_off) + ";" + String(unstable_load) + ";" + String(cur_power));
}

void update_load_limit(){
  Serial2.print("Load_limit;");
  for(int i=0; i<6; i++){
    Serial2.print(String(curent_max_load[i]) + ";");
  }
  Serial2.print("\n");
}

void update_overvoltage(){
  Serial2.println("Voltage_limit;" + String(max_voltage));
}

void update_connection(){
  Serial2.println("Connection;" + String(role) + ";" + String(on_when_master_on) + ";" + String(delay_before_off));
}

void update_name(){
  Serial2.println("Name;" + String(name));
}


