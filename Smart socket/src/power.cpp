#include <define.h>

unsigned long long zero_crossed_debounce=0;
bool sem_on=0;
short zero_crosses_count=0;

void IRAM_ATTR zero_crossed(){
  if (millis()-zero_crossed_debounce > 2){
    zero_crosses_count++;
    flag = 1;
    connected_to_grid = 1;
    connected_to_grid_tmp = 1;
    voltage.cache_data();
    current.cache_data();

    sine_waves_count ++;

    digitalWrite(TRIAC_PIN, 0);
    unsigned int time_till_open = 10000 - (power_controll.cur_power/255.0)*10000;
    if(power_controll.cur_power==255){
      digitalWrite(TRIAC_PIN, 1);
      sem_on = true;
      //timerAlarmWrite(timer2, 1000, false);
      //timerAlarmDisable(timer2);
    }
    else{
      sem_on = false;
      //timerAlarmWrite(timer2, time_till_open, true);
      //timerRestart(timer2);
      //timerAlarmEnable(timer2);
    }
    zero_crossed_debounce = millis();
  }
}

void Dim_ISR(){
  /*if (sem_on){
    digitalWrite(26, 0);
    sem_on = false;
  }
  else{  
    if(power_controll.cur_power != 0){
      digitalWrite(26, 1);
    }
    else{    
      digitalWrite(26, 0);
    }
    timerAlarmWrite(timer2, 1000, false);
    timerRestart(timer2);
    timerAlarmEnable(timer2);
    sem_on = true;
  }*/
  
}

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
      if(compare_2_times(max_load[i][1], max_load[i][2], Time.tm_hour, Time.tm_min) && 
      compare_2_times(Time.tm_hour, Time.tm_min, max_load[i][3], max_load[i][4]) && max_load[i][0] > max_time_power){
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