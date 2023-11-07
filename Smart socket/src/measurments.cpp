#include <define.h>

bool cur_vol_flag = 0;

void IRAM_ATTR get_data(){
  if(!cur_vol_flag){
    voltage.raw_data[voltage.raw_data_count++] = analogRead(VOLTAGE_SENSOR_PIN);
    voltage.peek_value = compute_Volts(voltage.raw_data[voltage.raw_data_count-1]) * 172.683;
    if(voltage.raw_data_count == 199){
      voltage.raw_data_count = 0;
    }
  }
  else{
    current.raw_data[current.raw_data_count++] = analogRead(CUR_SENSOR_PIN);
    if(current.raw_data_count == 199){
      current.raw_data_count = 0;
    }
  }
  cur_vol_flag = !cur_vol_flag;
}

void calculate_RMS(){
  //if(!connected_to_grid_tmp){
  //  connected_to_grid = 0;
  //  return;
  //}
  connected_to_grid_tmp = 0;
  
  voltage.calculate_rms();
  voltage.rms_data[4]*=122.087;

  current.calculate_rms();
  current.rms_data[4]*=9.334;

  momental_amp=current.rms_data[4];
  
}

void filter_RMS(){
    //if(!connected_to_grid_tmp){
    //  connected_to_grid = 0;
    //}

    voltage.median_filter();
    current.median_filter();

    momental_amp2 = current.rms_data_filtered[2];
    connected_to_grid_tmp = false;
}

void display_values(int time_pass){
  //getLocalTime(&Time);
  if(connected_to_grid){
    voltage.avg_filter();
    current.avg_filter();

    if (voltage.final_data < 10){
      connected_to_grid = 0;
    }

    wats = current.final_data * voltage.final_data;
    watt_hours += wats/3600.0*(500.0/time_pass);
  }
  
  minute_awr_wats[59] += wats/120.0;
  minute_watts_hours[59] += (wats/3600.0*(500.0/time_pass))/120;

  bool tmp = 0;
  
  group_power.limit_power();

  /*if(controll_by_sensor && !unstable_load && power_status == 2 && !overload){
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
  }*/

  
  if (last_minute != Time.tm_min){ 
    last_minute = Time.tm_min;
    for(int i=1; i<60; i++){
      minute_awr_wats[i-1] = minute_awr_wats[i];
      minute_watts_hours[i-1] = minute_watts_hours[i];
      minute_awr_wats[i] = 0;
      minute_watts_hours[i] = 0;
    }
    calculate_current_power_limit();
    if(role == 1 || role == 0){
      group_power.power_limit = curent_max_load[0];
      short sum_load = wats;
      for (int i=0; i<periferals_number; i++){
        sum_load += pereferals_load[i];
      }
      String mes_str = "Load overlimit";
      for (int i=0; i<periferals_number; i++){
        sum_load += pereferals_load[i];
        //send_data_over_UDP(mes_str + String(((curent_max_load[0] - sum_load)*pereferals_load[i])/(sum_load-wats)), perefirals_ip[i], localPort);
      }
    }
  }
  Serial.print("Volts:");
  Serial.println(int(voltage.final_data));
  Serial.print("Amps:");
  Serial.println(current.final_data, 4);
  Serial.println(power_controll.cur_power);
  //Serial.print("Wats:");
  //Serial.println(String(int(wats)) + "," + String(power));
  //Serial.print(String(wats) + "," + String(sensor_temperature) + "," + String(target_temperature) + ",");
  //Serial.println(String(int(cur_power)) + "," + String(Time.tm_hour) + ":" + String(Time.tm_min) + ":" + String(Time.tm_sec));
  //Serial.print(",");
  //Serial.print(Time.tm_hour);
  //Serial.print(":");
  //Serial.print(Time.tm_min);
  //Serial.print(":");
  //Serial.println(Time.tm_sec);
  //Serial.println(power);
  //Serial.print("Wats/H:");
  //Serial.println(watt_hours, 4);

  //if(role == 2){
  //  send_consumption();
  //}
  //if(connected_to_app){ 
  //  String mes_str = "Smart socket parameters"+String(int(voltage.final_data))+';'+String(current.final_data)+';'+String(wats)+';'+String(watt_hours)+';'+String(power_controll.cur_power);
  //  send_data_over_UDP(mes_str, app_ip, localPort);
  //}

  //bat_charge = analogRead(BATERY_VOLTAGE_PIN);
  bat_charge = (((0.801*bat_charge)+127)*100)/2100.0;
  //Serial2.println("Battery;" + String(bat_charge));
  //update_parameters();
  //update_power();

}

float compute_Volts(float data){
    #if (MODULE == 0)
      float tmp = (0.801*data)+127;
      return abs(tmp - zero_amp)/1000.0; 
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