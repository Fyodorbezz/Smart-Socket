#include <define.h>

void get_data(){
    for (int i=1; i<SAMPLE_LENGHT; i++){
        volts[i-1] = volts[i];
    }
    volts[SAMPLE_LENGHT-1] = analogRead(35);

    for (int i=1; i<SAMPLE_LENGHT; i++){
        amps[i-1] = amps[i];
    }
    amps[SAMPLE_LENGHT-1] = analogRead(34);
    count[0] ++;
    if(count[0] == 1250){
      bat_charge = analogRead(33);
      count[0] = 0;
    }
}

void calculate_RMS(){  
    for(int i=1; i < 5; i++){
      rms_volts[i-1] = rms_volts[i];
    }
    rms_volts[4] = 0;

    unsigned long tmp_volts[100];
    for(int i=0; i < SAMPLE_LENGHT; i++){
      tmp_volts[i] = volts[i];
    }
    for(int i=0; i < SAMPLE_LENGHT; i++){
      float tmp = compute_Volts(tmp_volts[i]);
      rms_volts[4] += pow(tmp, 2);
    }
    rms_volts[4] = sqrt(rms_volts[4]/SAMPLE_LENGHT);
    rms_volts[4] = rms_volts[4]*599.73;
    
    
    for(int i=1; i < 5; i++){
      rms_amps[i-1] = rms_amps[i];
    }
    rms_amps[4] = 0;

    unsigned long tmp_amps[100];
    for(int i=0; i < SAMPLE_LENGHT; i++){
      tmp_amps[i] = amps[i];
    }
    //float prev = compute_Volts_2(tmp_amps[0]);
    //unstable_load = false;
    for(int i=0; i < SAMPLE_LENGHT; i++){
      float tmp = compute_Volts_2(tmp_amps[i]);
      //if (abs(tmp-prev)>0.2){
        //unstable_load = true;
      //}
      //prev=tmp;
      rms_amps[4] += pow(tmp, 2);
      //Serial.println(tmp);
      //Serial.println(zero_amp);
      //Serial.print(',');
      //Serial.print(compute_Volts_2(amps[i]));
      //Serial.print(',');
      //Serial.println(pow(compute_Volts_2(amps[i]), 2));
    }   
    rms_amps[4] = sqrt(rms_amps[4]/SAMPLE_LENGHT);
    //Serial.println(millis());
    rms_amps[4] = rms_amps[4]*9.334;
    //Serial.println(rms_amps[4]);
    momental_amp=rms_amps[4];
    connected_to_grid_tmp = true;
    connected_to_grid = true;
    count[1] ++;
}

void filter_RMS(){
    if(!connected_to_grid_tmp){
      connected_to_grid = 0;
    }
    for(int i=1;i<5;i++){
      rms_volt[i-1] = rms_volt[i];
    }
    for (int i=0; i<5; i++){
      for (int j=0; j<5-i-1; j++){
        if (rms_volts[j] > rms_volts[j+1]){
          float tmp = rms_volts[j];
          rms_volts[j] = rms_volts[j+1];
          rms_volts[j+1] = tmp;
        }
      }
    }
    rms_volt[4] = rms_volts[2];

    for(int i=1;i<5;i++){
      rms_amp[i-1] = rms_amp[i];
    }
    count[1] = 0; 
    for (int i=0; i<5; i++){
      for (int j=0; j<5-i-1; j++){
        if (rms_amps[j] > rms_amps[j+1]){
          float tmp = rms_amps[j];
          rms_amps[j] = rms_amps[j+1];
          rms_amps[j+1] = tmp;
        }
      }
    }
    rms_amp[4] = rms_amps[2];
    momental_amp2 = rms_amps[2];
    connected_to_grid_tmp = false;
    count[2] ++;
}

void display_values(int time_pass){

  getLocalTime(&Time);
  if(connected_to_grid){
    disp_volt = 0;
    disp_amp = 0;
    for (int i=0;i<5;i++){
      disp_volt += rms_volt[i];
      
    }
    disp_volt = disp_volt/5.0;

    for (int i=0;i<5;i++){
      disp_amp += rms_amp[i];
    }
    disp_amp = disp_amp/5.0;

    if (disp_amp < 0.1){
      disp_amp = 0;
    }
  }
  wats = disp_amp * disp_volt;
  watt_hours += wats/3600.0*(500.0/time_pass);
  minute_awr_wats[59] += wats/120.0;
  minute_watts_hours[59] += (wats/3600.0*(500.0/time_pass))/120;

  bool tmp = 0;
  
  if(!unstable_load && power_status == 2){
    if(overload_wats/20 + load_err_last/50 > 0 && !controll_by_sensor && cur_power < last_power && cur_power < max_power){
      max_power += overload_wats/20 + load_err_last/50;
      last_power = -1;
    }
    else if(overload_wats/20 + load_err_last/50 < 0){
      if(last_power != -1){
        last_power = cur_power;
      }
      max_power += (overload_wats/20 + load_err_last/50);
      overload = false;
      tmp = true;
    }
    load_err_last = overload_wats;
    if(max_power > 255){
      max_power = 255;
    }
    if(max_power < 0){
      max_power = 0;
    }
    cur_power = max_power;
  }

  if(controll_by_sensor && !unstable_load && power_status == 2 && !overload){
    if(tmp && (target_temperature-sensor_temperature)*temperature_KP + ((target_temperature-sensor_temperature) - temperature_last_err)*temperature_KD + temperature_err_sum*temperature_KI < max_power){
      max_power = (target_temperature-sensor_temperature)*temperature_KP + ((target_temperature-sensor_temperature) - temperature_last_err)*temperature_KD;
    }
    else if(!tmp){
      if((target_temperature-sensor_temperature)*temperature_KP + ((target_temperature-sensor_temperature) - temperature_last_err)*temperature_KD  + temperature_err_sum*temperature_KI - max_power < 5){
        max_power = (target_temperature-sensor_temperature)*temperature_KP + ((target_temperature-sensor_temperature) - temperature_last_err)*temperature_KD;
      }
      else if((target_temperature-sensor_temperature)*temperature_KP + ((target_temperature-sensor_temperature) - temperature_last_err)*temperature_KD  + temperature_err_sum*temperature_KI > 0){
        max_power += 5;
      }
    }
    if(max_power > 255){
      max_power = 255;
    }
    if(max_power < 0){
      max_power = 0;
    }
    cur_power = max_power;
    temperature_last_err = target_temperature-sensor_temperature;
    temperature_err_sum += (target_temperature-sensor_temperature);
    if(temperature_err_sum > 50){
      temperature_err_sum = 50;
    }
    if(temperature_err_sum < -50){
      temperature_err_sum = -50;
    }
  }

  

  if(unstable_load && overload_wats > last_wats && role == 2 && shut_by_overload){
    power_on();
    last_wats = 99999;
    shut_by_overload = false;
  }
  
  if (last_minute != Time.tm_min){ 
    last_minute = Time.tm_min;
    for(int i=1; i<60; i++){
      minute_awr_wats[i-1] = minute_awr_wats[i];
      minute_watts_hours[i-1] = minute_watts_hours[i];
      minute_awr_wats[i] = 0;
      minute_watts_hours[i] = 0;
    }
    calculate_current_power_limit();
  }
  //Serial.print("Volts:");
  //Serial.println(int(disp_volt));
  //Serial.print("Amps:");
  //Serial.println(disp_amp, 4);
  
  
  //Serial.println(power);
  //Serial.print("Wats:");
  
  //Serial.println(String(int(wats)) + "," + String(power));
  Serial.print(String(wats) + "," + String(sensor_temperature) + "," + String(target_temperature) + ",");
  Serial.println(String(int(cur_power)) + "," + String(int(max_power)) + "," + String(Time.tm_hour) + ":" + String(Time.tm_min) + ":" + String(Time.tm_sec));
  
  //Serial.print(",");
  //Serial.print(Time.tm_hour);
  //Serial.print(":");
  //Serial.print(Time.tm_min);
  //Serial.print(":");
  //Serial.println(Time.tm_sec);
  
  //Serial.println(power);
  //Serial.print("Wats/H:");
  //Serial.println(watt_hours, 4);
  if(role == 2){
    send_consumption();
  }
  if(connected_to_app){ 
    String mes_str = "Smart socket parameters"+String(int(disp_volt))+';'+String(disp_amp)+';'+String(wats)+';'+String(watt_hours)+';'+String(cur_power);
    send_data_over_UDP(mes_str, app_ip, localPort);
  }
  bat_charge = (((0.801*bat_charge)+127)*100)/2100.0;
  Serial2.println("Battery;" + String(bat_charge));
  update_parameters();
  update_power();

}

float compute_Volts(float data){
    #if (MODULE == 0)
      float tmp = (0.801*data)+127;
      return abs(tmp - zero_volt)/1000.0; 
    #else
      float tmp = (0.796*data)+93;
      return abs(tmp-zero_volt)/1000.0;
    #endif

}

float compute_Volts_2(float data){
    #if (MODULE == 0) 
      float tmp = (0.801*data)+127;
      return abs(tmp - zero_amp)/1000.0; 
    #else
      float tmp = (0.796*data)+121;
      //Serial.println(String(tmp) + "," + String(zero_amp));
      return abs(tmp-zero_amp)/1000.0;
    #endif
}