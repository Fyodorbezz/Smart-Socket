#include <define.h>

void setup() {
  noInterrupts();
  Serial.println(getCpuFrequencyMhz());
  pinMode(ATMEGA_RES_PIN, OUTPUT);
  pinMode(NUTRAL_RELAY_PIN, OUTPUT);
  pinMode(LINE_REALY_PIN, OUTPUT);
  pinMode(13, OUTPUT);


  //adc1_config_width(ADC_WIDTH_BIT_12);
  //adc1_config_channel_atten(ADC1_CHANNEL_6,ADC_ATTEN_MAX);
  //adc1_config_channel_atten(ADC1_CHANNEL_7,ADC_ATTEN_MAX);

  ledcSetup(1, 100000, 8);
  ledcAttachPin(FAN_PIN, 1);

  max_load[0][0] = 2000;
  max_load[0][1] = -1;
  max_load[0][5] = 1;

  Serial.begin(500000);
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

  
  pinMode(TRIAC_PIN, OUTPUT);

  Serial.print("start");
  connect_btn.setTimeout(1000);
  connect_btn.isHolded();
  
  digitalWrite(NUTRAL_RELAY_PIN, 0);
  digitalWrite(LINE_REALY_PIN, 0);
  Serial.println("start");
  if(udp.listen(localPort)) {
      udp.onPacket(parsePacket);
  }
  Serial.println("start");
  

  Serial.println("Time");
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  getLocalTime(&Time);
  last_minute = Time.tm_min;
  Serial.println("Time2");
  
  Serial2.println("On;1");
  
  //temp_sensor.setResolution(11);
 
  Serial.println(getCpuFrequencyMhz());

  //max_load[1][0] = 1000; 
  //max_load[1][1] = Time.tm_hour;
  //max_load[1][2] = Time.tm_min + 2;
  //max_load[1][3] = Time.tm_hour;
  //max_load[1][4] = Time.tm_min + 2;
  //max_load[1][5] = 2;
  //max_loads_count ++;

  //calculate_current_power_limit();

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

  digitalWrite(NUTRAL_RELAY_PIN, 1);
  digitalWrite(LINE_REALY_PIN, 1); 

  Serial.println(analogRead(VOLTAGE_SENSOR_PIN));
  Serial.println(analogRead(CUR_SENSOR_PIN));
  zero_volt = 0;
  zero_amp = 0;
  //delay(1000);
  for(int i=0;i<4096;i++){
    zero_volt_tmp += analogRead(VOLTAGE_SENSOR_PIN);
    //delay(1);
    zero_amp_tmp += analogRead(CUR_SENSOR_PIN);
    //delay(1);
  }
  zero_volt_tmp = int(zero_volt_tmp / 4096.0 + 0.5);
  Serial.println(zero_volt_tmp);
  voltage.zero_value = compute_Volts(zero_volt_tmp);
  Serial.println(voltage.zero_value);

  zero_amp_tmp = int(zero_amp_tmp / 4096.0 + 0.5);
  Serial.println(zero_amp_tmp);
  current.zero_value = compute_Volts_2(zero_amp_tmp)*1000;
  Serial.println(current.zero_value);

  
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &get_data, true);
  //timerAlarmWrite(timer, 120, true);
  timerAlarmWrite(timer, 150, true);
  timerAlarmEnable(timer);

  timer2 = timerBegin(1, 80, true);
  timerAttachInterrupt(timer2, &Dim_ISR, true);
  timerAlarmWrite(timer2, 200000, true);
  timerAlarmEnable(timer2);

  attachInterrupt(ZERO_CROSS_PIN, &zero_crossed, FALLING);
  interrupts();
  
}

unsigned long long timeee = 0;


void loop() {
  //butt1.tick();
  //connect_btn.tick();
  //Serial.println(digitalRead(ZERO_CROSS_PIN));
  if(flag == 1){
    for(int i=0; i<voltage.raw_data_tmp_count; i+=2){
      //Serial.print("0,");
      //Serial.println(voltage.raw_data_tmp[i]);
    }
    //Serial.println(0);
    //Serial.println(voltage.raw_data_tmp_count);
    //Serial.println(millis()-timeee);
    timeee = millis();
    calculate_RMS();
    //Serial.println(voltage.rms_data[0]);
    flag = 0;
  }
  if(sine_waves_count % 5 == 0){
    filter_RMS();
  }
  if(sine_waves_count >= 25){
    display_values(millis() - display_val2);
    Serial.println(millis() - display_val2);
    //Serial.println(sine_waves_count);
    sine_waves_count = 0;
    display_val2 = millis();
    update_leds();
  }

  //power_controll.update();
  //group_power.update();

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
      periferal_led_state = 0;
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
        periferal_led_state = !periferal_led_state;
      }
      if (master_led_blink){
        master_led_state = !master_led_state;
      }
      blink_time = millis();
    }
  }
  else if(tasks == 7){
    if (power_controll.send_power_on_signal && role == 1){
      for (int i=0; i<periferals_number; i++){
        String mes_str = "Smart socket master on" + String(wats);
        send_data_over_UDP(mes_str, perefirals_ip[i], localPort);
      }
      power_controll.send_power_on_signal = false;
    }
  }
  else if(tasks == 8){
    if (power_controll.send_power_off_signal && role == 1){
      for (int i=0; i<periferals_number; i++){
        udp.connect(perefirals_ip[i], localPort);
        udp.broadcast("Smart socket master off");
      }
      udp.listen(localPort);
      power_controll.send_power_off_signal = false;
    }
  }
  else if(tasks == 9){
    if(last_second != Time.tm_sec){
      last_second = Time.tm_sec;
      update_time();
    }
  }
  else if(tasks == 10){
  }
  else if(tasks == 11){
    
  }
  else if(tasks == 12){
    if (butt1.isClick()){
      Serial.println("jkj");
      if(!power_controll.power_state){
        power_controll.turn_on();
      }
      else{
        power_controll.turn_off();
      }
    }
  }
  else if(tasks == 15){
    if (butt1.isDouble()){
      power_controll.overvoltage_state = 0;
      if(connected_to_app){
        uint8_t mes[] = "Smart socket normalvoltage";
        udp.writeTo(mes, sizeof(mes), app_ip, localPort);
      }
    }
  }
  else if(tasks == 16){
    if (!connected_to_grid){
      current.final_data = 0;
      voltage.final_data = 0;
      power_controll.cur_power = 0;
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
  else if(tasks == 19){
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

  ledcWrite(1, (power_controll.cur_power>0)*230);

  //tasks ++;
  if(tasks == 20){
    tasks = 0;
  }
  //Serial.println("asdas");
} 