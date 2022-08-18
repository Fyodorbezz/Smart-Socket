#include <define.h>

void setup() {
  Serial.println(getCpuFrequencyMhz());
  pinMode(14, OUTPUT);
  pinMode(12, OUTPUT);
  pinMode(25, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(32, OUTPUT);
  pinMode(13, OUTPUT);
  ledcSetup(0, 9000, 8);
  ledcAttachPin(5, 0);
  ledcSetup(1, 100000, 8);
  ledcAttachPin(14, 1);
  digitalWrite(32, 1);

  max_load[0][0] = 2000;
  max_load[0][1] = -1;
  max_load[0][5] = 1;

  Serial.begin(115200);
  Serial2.begin(2400);
  Serial2.setTimeout(50);
  Serial.println();
  Serial.println("Hello");
  WiFi.begin(ssid, pass);
  Serial.println("Hello2");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  udp.connect(SendIP, localPort);
  Serial.println("Starting UDP");
  Serial.print("Local port: ");
  Serial.println(localPort);
  delay(1000);
  noInterrupts();
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &get_data, true);
  timerAlarmWrite(timer, 400, true);
  timerAlarmEnable(timer);
  timer2 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer2, &Dim_ISR, true);
  timerAlarmWrite(timer2, 200000, true);
  timerAlarmEnable(timer2);
  interrupts();
  attachInterrupt(27, &zero_crossed, RISING);
  pinMode(26, OUTPUT);
  Serial.print("start");
  connect_btn.setTimeout(1000);
  connect_btn.isHolded();
  
  digitalWrite(32, 0);
  Serial.println("start");
  if(udp.listen(localPort)) {
      udp.onPacket(parsePacket);
  }
  Serial.println("start");
  analogRead(35);
  Serial.println("a");
  zero_volt = 0;
  zero_amp = 0;
  delay(1000);
  for(int i=0;i<512;i++){
    zero_volt_tmp += round(compute_Volts(analogRead(35))*1000);
    delay(2);
    zero_amp_tmp += round(compute_Volts_2(analogRead(34))*1000);
    delay(2);
  }
  zero_volt = zero_volt_tmp / 512;
  zero_amp = zero_amp_tmp / 512;
  #if (MODULE == 1)
    //zero_amp += 5;
  #endif
  Serial.println(zero_amp);
  Serial.println("Time");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime(&Time);
  last_minute = Time.tm_min;
  Serial.println("Time2");
  digitalWrite(32, 1); 
  Serial2.println("On;1");
  
  temp_sensor.setResolution(11);
  
  #if (DISPLAY == 1)
    //tft.initR(INITR_BLACKTAB);
    //tft.fillScreen(BLACK);
    //tft.setTextColor(BLACK);
    //tft.setRotation(2);
    
    
  #endif
  
 
  Serial.println(getCpuFrequencyMhz());

  //max_load[1][0] = 1000; 
  //max_load[1][1] = Time.tm_hour;
  //max_load[1][2] = Time.tm_min + 2;
  //max_load[1][3] = Time.tm_hour;
  //max_load[1][4] = Time.tm_min + 2;
  //max_load[1][5] = 2;
  //max_loads_count ++;

  calculate_current_power_limit();

  delay(100);
  digitalWrite(13, 1);
  update_name();
  delay(100);
  draw_grid();
  delay(100);
  update_parameters();
  delay(100);
  update_time();
  delay(100);
  update_power();
  delay(100);
  update_load_limit();
  delay(100);
  update_overvoltage();
  delay(100);
  update_connection();
  delay(100);
  
}

void loop() {
  butt1.tick();
  connect_btn.tick();

  if(flag == 1){
    calculate_RMS();
    flag = 0;
  }
  if(millis() - display_val > 100){
    filter_RMS();
    display_val = millis();
  }
  if(millis() - display_val2 > 500){
    display_values(millis() - display_val2);
    display_val2 = millis();
  }



  if ((momental_amp >= 0.3 || momental_amp2 >= 0.2) && power_status == 0 && btn_off == 0){
    if(!unstable_load){
      power_status = 1;
    }
    send_start_signal = true;
  }
  if (disp_amp < 0.2 && power_status == 2 && millis() - current_debounce_time > CURRENT_DEBOUNCE_TIMEOUT && !controll_by_sensor){
      Serial.println("Off");
      power_status = 0;
      if(unstable_load){
        cur_power = 255;
      }
      else{
        cur_power = 50;
      }
      send_stop_signal = true;
      delay_before_off_time = millis();
  }
  if((role == 0 || role == 1)){
    overload_wats = curent_max_load[0] - wats;
  }
  if ((role == 0 || role == 1) && wats > curent_max_load[0]){
    if(unstable_load){
      last_wats = wats;
      power_off();
      shut_by_overload = true;
    }
    overload = true;
    //Serial.println("Overlimit"); 
  }
  else if(role == 0 || role == 1){
    overload = false;
  }


  if(tasks == 0){
    if (connect_btn.isHolded()){
      Serial.println("hold");
      if (statuse == 0){
        statuse = 1;
        change_role(0);
        Serial.println("Searching");
      }
      else if (statuse == 1){
        statuse = 0;
        master_led_blink = 0;
        if (role == 1){
          master_led_state = 1;
        }
        else{
          master_led_state = 0;
        }
      }
    }
  }
  else if(tasks == 1){
    if (connect_btn.isClick() && statuse == 0 && role != 1){
      statuse = 2;
      slave_led_blink = 1;
      ready_to_become_slave_time = millis();
    }
  }
  else if(tasks == 2){
    if (statuse == 2 && millis() - ready_to_become_slave_time >= READY_TO_BECOME_SLAVE_TIMEOUT){
      statuse = 0;
      slave_led_blink = 0;
      slave_led_state = 0;
    }
  }
  else if(tasks == 3){
    if (statuse == 1 && millis() - slave_search_time >= SLAVE_SEARCH_TIMEOUT){
      String mes_str = "Smart socket searching for slaves"+name;
      send_data_over_UDP(mes_str, SendIP, localPort);
      slave_search_time = millis();
    }
  }
  else if(tasks == 4){
    if (millis() - master_online_time >= MASTER_ONLINE_TIMEOUT && role == 2 && master_check == 0){
      uint8_t mes[] = "Smart socket check if master online";
      udp.writeTo(mes, sizeof(mes), master_ip, localPort);
      master_check = 1;
      master_online_time = millis();
    }
  }
  else if(tasks == 5){
    if (master_check == 1 && millis() - master_online_time >= ANSWER_WAIT_TIMEOUT){
      change_role(0);
      update_connection();
    }
  }
  else if(tasks == 6){
    if (millis() - blink_time >= 500){
      if (slave_led_blink){
        slave_led_state = !slave_led_state;
      }
      if (master_led_blink){
        master_led_state = !master_led_state;
      }
      blink_time = millis();
    }
  }
  else if(tasks == 7){
    if (send_start_signal && role == 1){
      for (int i=0; i<slaves_number; i++){
        String mes_str = "Smart socket master on" + String(wats);
        send_data_over_UDP(mes_str, slaves_ip[i], localPort);
      }
      send_start_signal = false;
    }
  }
  else if(tasks == 8){
    if (send_stop_signal && role == 1){
      for (int i=0; i<slaves_number; i++){
        udp.connect(slaves_ip[i], localPort);
        udp.broadcast("Smart socket master off");
      }
      udp.listen(localPort);
      send_stop_signal = false;
    }
  }
  else if(tasks == 9){
    if(last_second != Time.tm_sec){
      last_second = Time.tm_sec;
      update_time();
    }
  }
  else if(tasks == 10){
    if ((millis() - speed_up_timer) >= speed_up_time/255.0 && power_status == 1){
      if (cur_power < max_power){
        cur_power++;
      }
      if (cur_power == max_power){
        power_status = 2;
      }
      speed_up_timer = millis();
    }
  }
  else if(tasks == 11){
    if ((millis() - speed_up_timer) >= speed_up_time/255.0 && power_status == 3){
      if (cur_power > 0){
        cur_power--;
      }
      if (cur_power == 0){
        power_status = 0;
      }
      speed_up_timer = millis();
    }
  }
  else if(tasks == 12){
    if (butt1.isClick()){
    if(btn_off){
      power_on();
    }
    else{
      power_off();
    }
  }
  }
  else if(tasks == 13){
    if(cur_power > max_power){ 
      cur_power = max_power;
      update_power();
    }
  }
  else if(tasks == 14){
    if(max_power > cur_power && !btn_off && power_status == 2){
      power_status = 1;
    }
  }
  else if(tasks == 15){
    if (butt1.isDouble()){
      overvoltage = 0;
    }
  }
  else if(tasks == 16){
    if (!connected_to_grid){
      disp_amp = 0;
      disp_volt = 0;
      cur_power = 0;
      momental_amp = 0;
    }
  }
  else if(tasks == 17){
    if(Serial2.available()){
      String message = Serial2.readString();
      if(message.substring(0, 12) == "Temp_sensor;"){
        sensor_temperature = message.substring(12).toFloat();
      }
    }
  }
  else if(tasks == 18){
    if(to_stop && role == 2 && millis() - delay_before_off_time  > delay_before_off){
      power_off();
      to_stop = 0;
    }
    if(to_start && role == 2 && millis() - delay_before_on_time > delay_before_on && master_power>power_treshold){
      power_on();
      to_start = 0;
    }
  }
  else if(tasks == 19){
    if(connected_to_grid != connected_to_grid_last){
      Serial2.println("Grid;" + String(int(connected_to_grid)));
      connected_to_grid_last = connected_to_grid;
    }
  }


  
  
  
  
  
  //Serial.println("a");
  
  //Serial.println("b");
  
  //Serial.println("c");
  
  //Serial.println("d");
  
  //Serial.println("e");
  
  //Serial.println("f");
  
  //Serial.println("g");
  
  //Serial.println("h");
  

  
  //Serial.println("j");
  
  
  //Serial.println("k");
  
  //Serial.println("l");
  
  //Serial.println("m");
  
  //Serial.println("n");
  
  //Serial.println("p");
  
  //Serial.println("q");
  
  //Serial.println("r");
  
  //Serial.println("s");
  
  //Serial.println("t");
  
  //Serial.println("u");
  
  
  //Serial.println("v");
  
  //Serial.println("w");
  
  //Serial.println("x");
  
  digitalWrite(12, slave_led_state);
  digitalWrite(25, overvoltage);
  digitalWrite(4, master_led_state);
  digitalWrite(2, connected_to_app);
  ledcWrite(0, cur_power);
  ledcWrite(1, connected_to_grid*250);
  digitalWrite(32, !overvoltage);

  tasks ++;
  if(tasks == 20){
    tasks = 0;
  }
  //Serial.println("asdas");
} 