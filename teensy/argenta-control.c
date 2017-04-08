#define aref_voltage 3.3 

long ssr_enabled_time_ms = 0;
long ssr_enabled_last_ms = 0;
long system_armed_time_ms = 0;
long system_ready_time_ms = 0;
long system_armed_last_ms = 0;
long system_ready_last_ms = 0;
long system_uptime_ms = 0;
int boiler_set_point_c =  120500;
int group_l_set_point_c = 10000;
int group_r_set_point_c = 10000;
int temp_sensor_error_limit = 250000;
int state_system_ready = 0;
int state_system_active = 0;
int state_system_armed = 0;
int state_ssr_enabled = 0;
int state_boiler_ready = 0;
int state_group_left_ready = 0;
int state_group_right_ready = 0;

long max_arm_time = 3600000 * 8;
long last_metrics_time_ms = 0;
long now = 0;

int pin_led_system_armed = 3;
int pin_led_system_active = 4; // LED to turn on when heating element is active
int pin_led_boiler_ready = 5; // LED to turn on when temp_probe_boiler > boiler_set_point
int pin_led_left_group_ready = 6; // LED to turn on when temp_probe_group_l > group_l_set_point
int pin_led_right_group_ready = 9; // LED to turn on when temp_probe_group_r > group_r_set_point
int pin_led_system_ready = 10; // LED to turn on when temp_probe_boiler, temp_probe_group_l, temp_probe_group_r are ready

int pin_temp_probe_boiler = 15; // value of boiler temp sensor
int pin_temp_probe_group_l = 16; // value of left grouphead temp sensor
int pin_temp_probe_group_r = 17; // value of right grouphead temp sensor
int pin_temp_probe_case_upper = 18; // value of upper case internals
int pin_temp_probe_case_lower = 20; // value of lower case internals
int pin_temp_probe_case_external = 21; // value of air outside case

int led_intensity = 1;

double temp_probe_boiler = 0.0;
double temp_probe_group_l = 0.0;
double temp_probe_group_r = 0.0;
double temp_probe_case_upper = 0.0;
double temp_probe_case_lower = 0.0;
double temp_probe_case_external = 0.0;

int pin_enable_switch = 22; // momentary switch to arm system

int pin_ssr_enabled = 11; // control the boiler


void setup() {

    pinMode(pin_led_system_armed, OUTPUT);
    pinMode(pin_led_system_active, OUTPUT);
    pinMode(pin_led_boiler_ready, OUTPUT);
    pinMode(pin_led_left_group_ready, OUTPUT);
    pinMode(pin_led_right_group_ready, OUTPUT);
    pinMode(pin_led_system_ready, OUTPUT);

    pinMode(pin_temp_probe_boiler, INPUT_PULLUP);
    pinMode(pin_temp_probe_group_l, INPUT_PULLUP);
    pinMode(pin_temp_probe_group_r, INPUT_PULLUP);
    pinMode(pin_temp_probe_case_upper, INPUT_PULLUP);
    pinMode(pin_temp_probe_case_lower, INPUT_PULLUP);
    pinMode(pin_temp_probe_case_external, INPUT_PULLUP);

    pinMode(pin_enable_switch, INPUT_PULLUP);
    
    pinMode(pin_ssr_enabled, OUTPUT);

    analogReference(EXTERNAL);
}

void getTemps(){
    temp_probe_boiler = tempSensorRead(pin_temp_probe_boiler);
    temp_probe_group_l = tempSensorRead(pin_temp_probe_group_l);
    temp_probe_group_r = tempSensorRead(pin_temp_probe_group_r);
    temp_probe_case_upper = tempSensorRead(pin_temp_probe_case_upper);
    temp_probe_case_lower = tempSensorRead(pin_temp_probe_case_lower);
    temp_probe_case_external = tempSensorRead(pin_temp_probe_case_external);
}

void armSystem(){
    state_system_armed = 1;
    system_armed_last_ms = millis();
    digitalWrite(pin_led_system_armed, HIGH);
}

void disarmSystem(){
    state_system_armed = 0;
    system_armed_time_ms = 0;
    system_armed_last_ms = millis();
    digitalWrite(pin_led_system_armed, LOW);
    digitalWrite(pin_ssr_enabled, LOW);
}

void enableSSR(){
    state_ssr_enabled = 1;
    ssr_enabled_last_ms = millis();
    digitalWrite(pin_ssr_enabled, HIGH);
}

void disableSSR(){
    state_ssr_enabled = 0;
    ssr_enabled_time_ms = ssr_enabled_time_ms + (millis() - ssr_enabled_last_ms);
    digitalWrite(pin_ssr_enabled, LOW);
}


int tempSensorRead(int pinNumber){
    analogRead(0);
    //delay(10);
    double reading = analogRead(pinNumber);
    //Serial.print(reading);
    //Serial.print(":");
    //Serial.print(aref_voltage);
    //Serial.print(":");
    double voltage = reading * ((aref_voltage)/1024.0);
    //Serial.print(voltage);
    //Serial.print(":");
    //voltage /= 1024.0; 
 
    double temperatureC = (voltage - 0.5) * 10000.0 ;

    if (state_system_armed){
        if (temperatureC > temp_sensor_error_limit){
            Serial.print("E_SENSOR_ERROR\n");
            state_system_armed = 0;
            disarmSystem;
        }      
    }

    //Serial.print(temperatureC);
    //Serial.print(":\n");
    //delay(100);
    return temperatureC;
}

void printMetrics() {
        int ssr_time_ms = ssr_enabled_time_ms;
        if (state_ssr_enabled){

            ssr_time_ms = ssr_time_ms + (millis() - ssr_enabled_last_ms);
        }
        int loop_time = millis() - last_metrics_time_ms;
        Serial.print(millis());
        Serial.print(";");
        Serial.print(state_system_armed);
        Serial.print(";");
        Serial.print(state_system_active);
        Serial.print(";");
        Serial.print(state_system_ready);
        Serial.print(";");
        Serial.print(system_armed_time_ms);
        Serial.print(";");
        Serial.print(system_ready_time_ms);
        Serial.print(";");
        Serial.print(ssr_time_ms);
        Serial.print(";");
        Serial.print(temp_probe_boiler);
        Serial.print(";");
        Serial.print(temp_probe_group_r);
        Serial.print(";");
        Serial.print(temp_probe_group_l);
        Serial.print(";");
        Serial.print(temp_probe_case_upper);
        Serial.print(";");
        Serial.print(temp_probe_case_lower);
        Serial.print(";");
        Serial.print(temp_probe_case_external);
        Serial.print(";");
        Serial.print(analogRead(pin_enable_switch));
        Serial.print(";\n");
        last_metrics_time_ms = millis();
   
}

void loop() {    
    
    if ((analogRead(pin_enable_switch) < 300 ) && ( (millis() - system_armed_last_ms) > 333 )){
        if (state_system_armed == 0){
            armSystem();
        } else {
            disarmSystem();
        }
    }

    if ( state_system_armed && (system_armed_time_ms > max_arm_time) ){
        disarmSystem();
    } 

    state_system_ready = 1;
    getTemps();

    
    if (state_system_armed) {
       system_armed_time_ms =+ (millis() - system_armed_last_ms);
       if (temp_probe_boiler < boiler_set_point_c){
           state_system_active = 1;

           if (state_ssr_enabled == 0){
               enableSSR();
           }
           //if (state_ssr_enabled){
           //    ssr_enabled_time_ms =+ (millis() - ssr_enabled_last_ms);
           //    ssr_enabled_last_ms = millis();
           //}

           digitalWrite(pin_led_system_active, HIGH);

       } else {
           if (state_ssr_enabled == 1) { 
                disableSSR();
           }
           //digitalWrite(pin_ssr_enabled, LOW);
           //digitalWrite(pin_led_system_active, LOW);
           state_system_active = 0;
           //ssr_enabled_last_ms = millis();
           //state_system_ready = 0;
        }
    } else {
        if (state_ssr_enabled == 1) { 
            disableSSR();
        }
        state_system_ready = 0;
        state_system_active = 0;
        //ssr_enabled_last_ms = millis();
        //digitalWrite(pin_ssr_enabled, LOW);
        digitalWrite(pin_led_system_active, LOW);
    }

    if (temp_probe_boiler > boiler_set_point_c ){
        analogWrite(pin_led_boiler_ready, led_intensity);
    } else {
        //Serial.print('boiler lower than setpoint\n');
        digitalWrite(pin_led_boiler_ready, LOW);
        state_system_ready = 0;
    }

    if ( temp_probe_group_l > group_r_set_point_c ){
        analogWrite(pin_led_left_group_ready, led_intensity);
    } else {
        //Serial.print("l lower than setpoint\n");
        digitalWrite(pin_led_left_group_ready, LOW);
        state_system_ready = 0;
    }


    if ( temp_probe_group_r > group_r_set_point_c ){
        analogWrite(pin_led_right_group_ready, led_intensity);
    } else {
        digitalWrite(pin_led_right_group_ready, LOW);
        //Serial.print("r lower than setpoint\n");
        state_system_ready = 0;
    }

    
    if (state_system_ready == 1) {
        analogWrite(pin_led_system_ready, led_intensity);
        now = millis();
        system_ready_time_ms =+ (now - system_ready_last_ms);
        system_ready_last_ms = now;
    } else {
        digitalWrite(pin_led_system_ready, LOW);
        system_ready_last_ms = now;
    }

    if (millis() - last_metrics_time_ms > 999){
        printMetrics();
    }

}
