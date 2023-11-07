#include <define.h>

void parsePacket(AsyncUDPPacket packet){
    const uint8_t* msg = packet.data();
    const size_t len = packet.length();
    String message = "";
    IPAddress sender_ip;
    if (msg != NULL && len > 0) {
        sender_ip = packet.remoteIP();
        for (int i=0;i<len;i++){
            message += char(msg[i]);
        }
        if(message.substring(0, 33) == "Smart socket searching for slaves" && statuse==2){
          master_ip = packet.remoteIP();
          master_name = message.substring(33);
          String mes_str = "Smart socket became you slave"+name;
          send_data_over_UDP(mes_str, master_ip, localPort);
        }
        if(message.substring(0, 29) == "Smart socket became you slave" && statuse==1){
          perefirals_ip[periferals_number] = packet.remoteIP();
          perefirals_name[periferals_number] = message.substring(29);
          slave_search_time = millis();
          uint8_t mes[] = "You became slave";
          udp.writeTo(mes, sizeof(mes), perefirals_ip[periferals_number], localPort);
          periferals_number ++;
          role = 1;
          Serial.println("New_slave" + perefirals_name[periferals_number-1]);
          update_connection();
        }
        if(message == "You became slave" && statuse==2){
          change_role(2);
          Serial.println("Connected to master");
        }
        if(message == "Smart socket check if master online"){
          bool tmp1 = 0;
          for (int i=0;i<periferals_number;i++){
            if (sender_ip == perefirals_ip[i]){ 
              tmp1 = 1;
            }
          }
          if (tmp1 == 1){
            uint8_t mes[] = "Smart socket master online";
            udp.writeTo(mes, sizeof(mes), sender_ip, localPort);
          }
        }
        if(message == "Smart socket master online"){
          master_check = 0;
          //Serial.println("Master online");
        }
        if(message.substring(0, 22) == "Smart socket master on" && on_when_master_on){
          if(group_power.power_trashold > message.substring(22).toInt()){
            group_power.power_on_timer.restart();
            group_power.on_order = 1;
          }
        }
        if(message == "Smart socket master off" && on_when_master_on){
          group_power.power_off_timer.restart();
          group_power.off_order = 1;
        }
        if(message.substring(0, 24) == "Smart socket consumption" && role == 1){
          for(int i=0; i<periferals_number; i++){
            if(perefirals_ip[i] == sender_ip){
              pereferals_load[i] = message.substring(24).toInt();
            }
          } 
        }
        if(message.substring(0, 14) == "Load overlimit"){
          group_power.power_limit = message.substring(14).toInt();
        }
        if(message == "App searching for smart sockets"){
          String mes_str = "Smart socket detected"+name;
          send_data_over_UDP(mes_str, sender_ip, localPort);
          Serial.println("Mobile app");
        }
        if(message == "App smart socket connected"){
          connected_to_app = 1;
          app_ip = sender_ip;
          String mes_str = "Smart socket send parameters"+String(power_controll.power_state)+";"+String(power_controll.dimm_load)+";"+String(role)+";"+String(max_consumption)+";"+String(group_power.power_off_timer.timer_period);
          send_data_over_UDP(mes_str, sender_ip, localPort);
          send_role();
          send_max_load();
          mes_str = "Send max load count"+String(max_loads_count);
          send_data_over_UDP(mes_str, sender_ip, localPort);
        }
        if(message == "App smart socket disconnected"){
          connected_to_app = 0;
          app_ip.fromString("0.0.0.0");
        }
        if(message.substring(0, 13) == "Unstable load"){
          power_controll.dimm_load = !str_to_bool(message, 13);
          update_power();
        }
        if(message.substring(0, 4) == "Sink"){
          on_when_master_on = str_to_bool(message, 4);
          update_connection();
        }
        if(message.substring(0, 16) == "Delay before off"){
          group_power.power_off_timer.set_period(message.substring(16).toInt());
        }
        if(message.substring(0, 15) == "Delay before on"){
          group_power.power_on_timer.set_period(message.substring(15).toInt());
        }
        if(message.substring(0, 13) == "Load treshold"){
          group_power.power_trashold = message.substring(13).toInt();
        }
        if(message.substring(0, 8) == "Max load"){
          
          String tmp = message.substring(8);
          String tmp2 = "";
          int cells_count = 0;
          for (int i = 0;i < tmp.length();i++){
            if(tmp[i] != ';'){
              tmp2+=tmp[i];
            }
            else{
              max_load[max_loads_count][cells_count] = tmp2.toInt();
              cells_count ++;
              if(tmp2.toInt() == -1){
                break;
              }
              tmp2 = "";
            }
          }
          max_loads_count ++;
          calculate_current_power_limit();
        }
        if(message.substring(0, 15) == "Delete max load"){
          int tmp = message.substring(15).toInt();
          bool flag = false;
          for(int i=0; i<max_loads_count; i++){
            if(max_load[i][5] == tmp){
              flag = true;
              max_loads_count-=1;
            }
            if(flag){
              for(int j=0; j<6; j++){
                max_load[i][j] = max_load[i+1][j];
              }
            }
          }
          calculate_current_power_limit();
        }
        if(message.substring(0, 8) == "Power on"){
          power_controll.turn_on();
        }
        if(message.substring(0, 9) == "Power off"){
          power_controll.turn_off();
        }
        if(message.substring(0, 9) == "Max power"){
          power_controll.max_power = message.substring(9).toInt();
        }
        if(message.substring(0, 19) == "Clear power counter"){
          watt_hours = 0;
        }
        if(message.substring(0, 11) == "Change name"){
          name = message.substring(11);
          update_name();
          Serial.println(name);
        }
        if(message == "App check smart socket online"){
          String mes_str = "Smart socket online";
          send_data_over_UDP(mes_str, app_ip, localPort);
          Serial.println("Ping");
        }
        if(message.substring(0, 10) == "Chart_type"){
          Serial.println("Chart");
          chart_period = message.substring(10).toInt() - 1;
        }
        if(message.substring(0, 14) == "Set edpendancy"){
          controll_by_sensor = true;
          target_temperature = message.substring(14).toInt();
        }
        if(message.substring(0, 17) == "Delete edpendancy"){
          controll_by_sensor = false;
        }
        if(message.substring(0, 16) == "Send period data"){
          chart_period = message.substring(16).toInt();
        }
        if(message.substring(0, 14) == "Send data data"){
          chart_type = message.substring(14).toInt();
        }
        if(message.substring(0, 18) == "Request chart data"){
          String mes_str;
          int count = 0;
          int start = 0;
          if(chart_period == 0){
            mes_str = "Smart socket data last hour";
            count = 30;
            
          }
          else if(chart_period == 1){
            mes_str = "Smart socket data last hour";
            count = 0;
          }
          for (int i=count; i<60; i++){
            int tmp = Time.tm_min-59+i;
            if(tmp<0){
              mes_str += String(Time.tm_hour-1) + ':' + String(tmp+60)+'/';
            }
            else{
              mes_str += String(Time.tm_hour) + ':' + String(tmp)+'/';
            }
            if(chart_type == 0){
              mes_str += String(int(minute_awr_wats[i])) + ';';
              Serial.println(i);
              Serial.println(minute_awr_wats[i]);
            }
            else if(chart_type == 1){
              mes_str += String(int(minute_watts_hours[i])) + ';';
            }
          }
          send_data_over_UDP(mes_str, app_ip, localPort);
        }
        if(message.substring(0, 17) == "Overvoltage value"){
          power_controll.overvoltage_value = message.substring(17).toInt();
          update_overvoltage();
        }
        if(message.substring(0, 6) == "Set KP"){
          temperature_KP = message.substring(6).toFloat();
        }
        if(message.substring(0, 6) == "Set KD"){
          temperature_KD = message.substring(6).toFloat();
        }
        if(message.substring(0, 6) == "Set KI"){
          temperature_KI = message.substring(6).toFloat();
        }
    }
}