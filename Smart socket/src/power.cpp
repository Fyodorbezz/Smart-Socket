#include <define.h>

void zero_crossed(){
  if (millis()-debounce > 2){
    croses++;
    if (croses%2 == 0){
      flag = 1;
      
    }
    digitalWrite(26, 0);
    time_till_open = 10000 - (cur_power/255.0)*10000;
    
    if(cur_power==255){
      digitalWrite(26, 1);
      sem_on = true;
      timerAlarmWrite(timer2, 1000, false);
      timerAlarmDisable(timer2);
    }
    else{
      sem_on = false;
      timerAlarmWrite(timer2, time_till_open, true);
      timerRestart(timer2);
      timerAlarmEnable(timer2);
    }
    debounce = millis();
    timee2 = micros();
  }
  /*
  if(croses==100){
    freq=croses/((millis()-freq_time)/1000);
    freq_time=millis();
    croses=0;
  }*/
  
  //Serial.println(micros()-timee2);
  //timee2=micros();
}

void power_off(){
  if (unstable_load || disp_amp < 0.2){
     power_status = 0;
     cur_power = 0;
  }
  else{
     power_status = 3;  
  }
  //send_stop_signal = true;
  btn_off = true;
  if(connected_to_app){
    uint8_t mes[] = "Smart socket off";
    udp.writeTo(mes, sizeof(mes), app_ip, localPort);
  }
  shut_by_overload = false;
  update_power();
}

void power_on(){
  if(unstable_load){
    power_status = 2;
    cur_power = 255;
  }
  else{
    power_status = 0; 
    cur_power = 50;
  }
  if(connected_to_app){
    uint8_t mes[] = "Smart socket on";
    udp.writeTo(mes, sizeof(mes), app_ip, localPort);
  }
  btn_off = false;
  current_debounce_time = millis();
  update_power();
}

void Dim_ISR(){
  if (sem_on){
    digitalWrite(26, 0);
    sem_on = false;
  }
  else{  
    if(cur_power != 0){
      digitalWrite(26, 1);
    }
    else{    
      digitalWrite(26, 0);
    }
    timerAlarmWrite(timer2, 1000, false);
    timerRestart(timer2);
    timerAlarmEnable(timer2);
    sem_on = true;
  }
  
}

/*bool overload(int load){
  bool output = false;
  for(int i =0;i<max_loads_count;i++){
    if(max_load[i][1] == -1){
      //Serial.println(max_load[i][0]);
      //Serial.println(load);
      if(load>max_load[i][0]){
        output = true;
      }
    }
    else{
      if(compare_2_times(max_load[i][1], max_load[i][2], Time.tm_hour, Time.tm_min) && compare_2_times(Time.tm_hour, Time.tm_min, max_load[i][3], max_load[i][4])){
        if(load > max_load[i][0]){
          output = true;
        }
      }
    }
  }
  return output;
}*/

bool compare_2_times(int hour1, int min1, int hour2, int min2){
  if(hour2>hour1 || (hour1 == hour2 && min2>=min1)){
    return true;
  }
  else{
    return false;
  }
}

void calculate_current_power_limit(){
  int all_day_limit = -1;
  int time_limit = -1;
  int max_time_power = -1;
  for(int i=0; i<max_loads_count; i++){
    if(max_load[i][1] == -1){
      all_day_limit = i;
    }
    else{
      if(compare_2_times(max_load[i][1], max_load[i][2], Time.tm_hour, Time.tm_min) && compare_2_times(Time.tm_hour, Time.tm_min, max_load[i][3], max_load[i][4]) && max_load[i][0] > max_time_power){
        max_time_power = max_load[i][0];
        time_limit = i;
      }
    }
  }
  if(time_limit != -1){
    all_day_limit = time_limit;
  }
  if(all_day_limit != -1){
    for(int i=0; i<6; i++){
      curent_max_load[i] = max_load[all_day_limit][i];
    }
  }
  else{
    for(int i=0; i<6; i++){
      curent_max_load[i] = -1;
    }
  }
  update_load_limit();
}