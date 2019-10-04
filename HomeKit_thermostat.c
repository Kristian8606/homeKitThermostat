/*
 * Copyright 2019 Kristian Dimitrov
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 * 
 * http://www.apache.org/licenses/LICENSE-2.0
 
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 * 
 * EPS8266 HomeKit WIFI Thermostat 
 *
 * Uses a BME280 (temperature sensor)
 *
 *
 */

#define DEVICE_MANUFACTURER "Kristian Dimitrov"
#define DEVICE_NAME "HomeKit-thermostat"
#define DEVICE_MODEL "SSD1306 & DHT22"
#define DEVICE_SERIAL "12345678"
#define FW_VERSION "0.4.2"

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_system.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <etstimer.h>
#include <esplibs/libmain.h>
#include <FreeRTOS.h>
#include <task.h>
#include <ssd1306/ssd1306.h>
#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>
#include <dht/dht.h>
//#include <sysparam.h>
#include "HomeKit_thermostat.h"
#include "button.h"
//#include "bmp280/bmp280.h"
//#include <rboot-api.h>
#include <i2c/i2c.h>
#include <string.h>
#include <stdlib.h>
#include "fonts/fonts.h"
#include "ota-api.h"



#define TEMPERATURE_SENSOR_PIN 4 //D2
#define TEMPERATURE_POLL_PERIOD 10000
#define BUTTON_UP_GPIO 12  //D6
#define BUTTON_DOWN_GPIO 13 //D7
#define BUTTON_RESET 0 //D3
#define RELAY_GPIO 2 //D4
#define  RECIVE_GPIO 16 //D0
const int LED_GPIO = 15; //D8

//Timer
TimerHandle_t xSave_characteristic_Timer = NULL;
TimerHandle_t xTemperatureTimer = NULL;

void process_setting_update();
void on_update(homekit_characteristic_t *ch, homekit_value_t value, void *context)
{
    process_setting_update();
    xTimerReset(xSave_characteristic_Timer,0);
}

homekit_characteristic_t current_temperature = HOMEKIT_CHARACTERISTIC_(CURRENT_TEMPERATURE, 0);
homekit_characteristic_t target_temperature = HOMEKIT_CHARACTERISTIC_(TARGET_TEMPERATURE, 20, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t units = HOMEKIT_CHARACTERISTIC_(TEMPERATURE_DISPLAY_UNITS, 0);
homekit_characteristic_t current_state = HOMEKIT_CHARACTERISTIC_(CURRENT_HEATING_COOLING_STATE, 0);
homekit_characteristic_t target_state = HOMEKIT_CHARACTERISTIC_(TARGET_HEATING_COOLING_STATE, 1, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t cooling_threshold = HOMEKIT_CHARACTERISTIC_(COOLING_THRESHOLD_TEMPERATURE, 25, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t heating_threshold = HOMEKIT_CHARACTERISTIC_(HEATING_THRESHOLD_TEMPERATURE, 15, .callback = HOMEKIT_CHARACTERISTIC_CALLBACK(on_update));
homekit_characteristic_t current_humidity = HOMEKIT_CHARACTERISTIC_(CURRENT_RELATIVE_HUMIDITY, 0);

homekit_characteristic_t ota_trigger  = API_OTA_TRIGGER;
homekit_characteristic_t name         = HOMEKIT_CHARACTERISTIC_(NAME, DEVICE_NAME);
homekit_characteristic_t manufacturer = HOMEKIT_CHARACTERISTIC_(MANUFACTURER,  DEVICE_MANUFACTURER);
homekit_characteristic_t serial       = HOMEKIT_CHARACTERISTIC_(SERIAL_NUMBER, DEVICE_SERIAL);
homekit_characteristic_t model        = HOMEKIT_CHARACTERISTIC_(MODEL,         DEVICE_MODEL);
homekit_characteristic_t revision     = HOMEKIT_CHARACTERISTIC_(FIRMWARE_REVISION,  FW_VERSION);

float temp_pause = 0, T = 0;
bool sensor_temp = false;
bool is_display_init = false;

//static ETSTimer thermostat_timer;
static TaskHandle_t  Handle_screen;
bool fire = false;
int count_error = 0;

/* Change this according to you schematics and display size */
#define DISPLAY_WIDTH 128
#define DISPLAY_HEIGHT 64

#define PROTOCOL SSD1306_PROTO_I2C
#define ADDR SSD1306_I2C_ADDR_0
#define I2C_BUS 0
#define SCL_PIN 14 //D5
#define SDA_PIN 5  //D1

//const uint8_t i2c_bus = 0;
//const uint8_t scl_pin = 14;
//const uint8_t sda_pin = 5;

   
#define DEFAULT_FONT FONT_FACE_TERMINUS_16X32_ISO8859_1

/* Declare device descriptor */
static const ssd1306_t dev = {
    .protocol = SSD1306_PROTO_I2C,
    .screen = SSD1306_SCREEN,
   // .screen = SH1106_SCREEN,
    .i2c_dev.bus = I2C_BUS,
    .i2c_dev.addr = SSD1306_I2C_ADDR_0,
    .width = DISPLAY_WIDTH,
    .height = DISPLAY_HEIGHT,
};

/* Local frame buffer */
static uint8_t buffer[DISPLAY_WIDTH * DISPLAY_HEIGHT / 8];

#define SECOND (1000 / portTICK_PERIOD_MS)

void led_write(bool on)
{
    gpio_write(LED_GPIO, on ? 0 : 1);
}

void relay_write(bool on)
{
    gpio_write(RELAY_GPIO, on ? 1 : 0);
    if(on == true){
    	temp_pause = 0;
    }else{
    	temp_pause = 0.2;
    }
}

/// I2C
   





// temp display value
    char target_temp_string[20];
    char mode_string[20];
    char temperature_string[20];
    char humidity_string[20];

    int xT = 0, yT = 0, xM = 93, yM = 50, xH = 90, yH = 2;
// temp display value









void thermostat_identify_task(void *_args)
{
printf("Thermostat identify\n");
 //  vTaskSuspend(xHandle);
   vTaskSuspend(Handle_screen);
	 ssd1306_clear_screen(&dev);

        for (int i = 0; i<= 50;i+=5){
 		if (ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK)){
     
	     }
		 if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_8X14_ISO8859_1], 5, i, "Initialization", OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
 	     {
            printf("Error printing mode\n");
         }
         
        ssd1306_load_frame_buffer(&dev, buffer);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        }
        
   // led_write(false);
	vTaskDelay(500 / portTICK_PERIOD_MS);
//	vTaskResume(xHandle);
	vTaskResume(Handle_screen);
    vTaskDelete(NULL);
}

void thermostat_identify(homekit_value_t _value)
{
    xTaskCreate(thermostat_identify_task, "Thermostat identify", 256, NULL, 2, NULL);
}
//void ota_firmware_callback();










void save_characteristic_to_flash (homekit_characteristic_t *ch, homekit_value_t value);

void load_characteristic_from_flash (homekit_characteristic_t *ch);

void xSave_characteristic_TimerCallback(){
        
        save_characteristic_to_flash (&target_temperature, target_temperature.value);
		save_characteristic_to_flash (&target_state, target_state.value);
	
}



void wifi_led(){
	for(int i = 0; i< 4;i++){
		
			led_write(true);
  		    vTaskDelay(600 / portTICK_PERIOD_MS);
   			led_write(false);
   			vTaskDelay(150 / portTICK_PERIOD_MS);
		
		
	}
    led_write(false); 
    
    vTaskDelete(NULL);
}


void wifi_init_led()
{
    xTaskCreate(wifi_led, "Reset configuration", 256, NULL, 2, NULL);
}

void reset_configuration_task() {
	for(int i = 0; i< 4;i++){
		for(int i = 0; i< 4;i++){
			led_write(true);
  		    vTaskDelay(150 / portTICK_PERIOD_MS);
   			led_write(false);
   			vTaskDelay(100 / portTICK_PERIOD_MS);
		}
		vTaskDelay(300 / portTICK_PERIOD_MS);
	}
    led_write(false);
    
	
	vTaskDelay(500 / portTICK_PERIOD_MS);
   printf("Resetting Wifi Config\n");

   wifi_config_reset();

   vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Resetting HomeKit Config\n");

    homekit_server_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Restarting\n");

    sdk_system_restart();

    vTaskDelete(NULL);
}

void reset_configuration()
{
    printf("Resetting configuration\n");
    xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
}

// LCD ssd1306

 void ssd1306_task(void *pvParameters){

 if (ssd1306_load_xbm(&dev, homekit_logo, buffer));

    if (ssd1306_load_frame_buffer(&dev, buffer));

     
	 vTaskDelay(SECOND * 3);
	 ssd1306_clear_screen(&dev);
	vTaskDelay(SECOND /2);
	 ssd1306_display_on(&dev, true);
/*	
	 UBaseType_t uxHighWaterMark;
	 uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
	 printf("Display free stack >>>>> %d\n", uxHighWaterMark);
*/	
 		while(1){
 		
       uint8_t hum = (uint8_t)current_humidity.value.float_value;
        sprintf(temperature_string, "%g", (float)current_temperature.value.float_value);
        sprintf(humidity_string, "%i", hum /*(float)current_humidity.value.float_value */);
        sprintf(target_temp_string, "%g", (float)target_temperature.value.float_value);

        uint8_t i = 0;

        for (i = 0; temperature_string[i] != '\0'; i++);
        	if (i > 2){
            i = 72;
        	}else{
            i = 42;
        	}
        

        switch ((int)target_state.value.int_value){

        case 0:
            sprintf(mode_string, "OFF ");
            break;
        case 1:
            sprintf(mode_string, "HEAT");
            break;
        case 2:
            sprintf(mode_string, "COOL");
            break;
        case 3:
            sprintf(mode_string, "AUTO");
            break;
        default:
            sprintf(mode_string, "?   ");
        }
       
        if(sensor_temp){
  	      	if (ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK))
  	      {
  	          printf("Error printing rectangle\bn");
  	      } 
  	       if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1], 25, 10,"Sensor", OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
  	      {
  	          printf("Error printing error sensor\n");
  	      }
  	       if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1], 45, 33,"Error!", OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
  	      {
  	          printf("Error printing error sensor\n");
  	      }
        
        }else{

        if (ssd1306_fill_rectangle(&dev, buffer, 0, 0, DISPLAY_WIDTH, DISPLAY_HEIGHT, OLED_COLOR_BLACK))
        {
            printf("Error printing rectangle\bn");
        } 
       
        if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_16X32_ISO8859_1], xT, yT,temperature_string, OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
        {
            printf("Error printing temperature\n");
        }

        if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1], 2, 42, target_temp_string, OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
        {
            printf("Error printing target temp\n");
        }

        if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_8X14_ISO8859_1], xM, yM, mode_string, OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
        {
            printf("Error printing mode\n");
        }

       
        if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1], xH, yH, humidity_string, OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
        {
            printf("Error printing humidity\n");
        }

        ssd1306_draw_circle(&dev, buffer, i, 7, 3, OLED_COLOR_WHITE);
        //   {
        //       printf("Error printing celsius temp\n");
        //  }

        if (ssd1306_draw_string(&dev, buffer, font_builtin_fonts[FONT_FACE_TERMINUS_BOLD_11X22_ISO8859_1], 117, 2, "%", OLED_COLOR_WHITE, OLED_COLOR_BLACK) < 1)
        {
            printf("Error printing mode\n");
        }
       

        if (fire){
         ssd1306_load_xbm(&dev, thermostat_xbm, buffer);
        }

     } 

        if (ssd1306_load_frame_buffer(&dev, buffer))
        is_display_init = false;
        
			printf(" Print Screen\n");
		
		// uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		// printf("Display free stack %d\n", uxHighWaterMark);
			 
   vTaskSuspend(Handle_screen);

	}
	
	

}


void screen_init(void)
{
    //uncomment to test with CPU overclocked
    //sdk_system_update_cpu_freq(160);

    printf("Screen Init SDK version:%s\n", sdk_system_get_sdk_version());

    i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_400K);


    if (ssd1306_init(&dev)) {
        printf(" Failed to init SSD1306 lcd\n");
        is_display_init = false;
      return;
    }

    printf("SSD1306 lcd --------->>>>>>>> init\n");
    ssd1306_set_whole_display_lighting(&dev, false);
    ssd1306_set_scan_direction_fwd(&dev, true);
    ssd1306_set_segment_remapping_enabled(&dev, false);
    is_display_init = true;
    
   xTaskCreate(ssd1306_task, "ssd1306_task", 384, NULL, 2, &Handle_screen);
   // }
}

// LCD ssd1306



//Button


void button_up_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Button UP\n");
	    if ((target_temperature.value.float_value + 0.5) <= 38)
        {
            target_temperature.value.float_value += 0.5;
            homekit_characteristic_notify(&target_temperature, target_temperature.value);
           // save_characteristic_to_flash (&target_temperature, target_temperature.value);
        }
            
            
            break;
        case button_event_long_press:
            printf("Button UP\n");
            if ((target_temperature.value.float_value + 1) <= 38)
        {
            target_temperature.value.float_value += 1;
            homekit_characteristic_notify(&target_temperature, target_temperature.value);
          //  save_characteristic_to_flash (&target_temperature, target_temperature.value);
        }
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}


void button_down_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Button DOWN\n");
           	if ((target_temperature.value.float_value - 0.5) >= 10)
        {
            target_temperature.value.float_value -= 0.5;
            homekit_characteristic_notify(&target_temperature, target_temperature.value);
          //  save_characteristic_to_flash (&target_temperature, target_temperature.value);
        }
            break;
        case button_event_long_press:
            printf("Button DOWN\n");
             if ((target_temperature.value.float_value - 1) >= 10)
        {
            target_temperature.value.float_value -= 1;
            homekit_characteristic_notify(&target_temperature, target_temperature.value);
           // save_characteristic_to_flash (&target_temperature, target_temperature.value);
        }
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}




void reset_button_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Button event: %d, doing nothin\n", event);
              uint8_t state = target_state.value.int_value + 1;
   

        switch (state){
        case 1:
            //heat
            state = 1;
            break;
 /*           //cool
        case 2:
            state = 2;

   */          break;
            //auto
        case 2:
            state = 3;
           break;

        default:
            //off

            state = 0;
            break;
        }
        target_state.value = HOMEKIT_UINT8(state);
        homekit_characteristic_notify(&target_state, target_state.value);
            break;
        case button_event_long_press:
            printf("Button event: %d, resetting homekit config\n", event);
            reset_configuration();
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}


void process_setting_update(){

    uint8_t state = target_state.value.int_value;
     if (state == 2) {
        state = 1;
        target_state.value = HOMEKIT_UINT8(1);
        homekit_characteristic_notify(&target_state, target_state.value);
        }
    if ((state == 1 && (current_temperature.value.float_value + temp_pause) < target_temperature.value.float_value) ||
        (state == 3 && current_temperature.value.float_value < heating_threshold.value.float_value))
    {
        if (current_state.value.int_value != 1)
        {
            current_state.value = HOMEKIT_UINT8(1);
            homekit_characteristic_notify(&current_state, current_state.value);

            relay_write(true);
        }
    }
    else if ((state == 2 && current_temperature.value.float_value > target_temperature.value.float_value) ||
             (state == 3 && current_temperature.value.float_value > cooling_threshold.value.float_value))
    {
        if (current_state.value.int_value != 2)
        {
            current_state.value = HOMEKIT_UINT8(2);
            homekit_characteristic_notify(&current_state, current_state.value);

            relay_write(false);
        }
    }
    else
    {
        if (current_state.value.int_value != 0)
        {
            current_state.value = HOMEKIT_UINT8(0);
            homekit_characteristic_notify(&current_state, current_state.value);

            relay_write(false);
        }
    }
 	 if(is_display_init){
 	  vTaskResume(Handle_screen);
	  }
}    

void temperature_sensor_task(){


    
    float humidity_value, temperature_value;
       fire = gpio_read(RECIVE_GPIO) == 1;
        
    if (dht_read_float_data(DHT_TYPE_DHT22, TEMPERATURE_SENSOR_PIN, &humidity_value, &temperature_value)) {
    	if(T != temperature_value){
    		T = temperature_value;
            current_temperature.value = HOMEKIT_FLOAT(temperature_value);
            homekit_characteristic_notify(&current_temperature, current_temperature.value);
            
            current_humidity.value = HOMEKIT_FLOAT(humidity_value);
            homekit_characteristic_notify(&current_humidity, current_humidity.value);
                
            process_setting_update();
           }
           printf("Temperature: %.2f C", temperature_value);
           printf(", Humidity: %.2f\n", humidity_value);
           count_error = 0;
           sensor_temp = false;
    } else {
    
   		count_error++;
   		
   		if(count_error >= 5){
        printf(">>> Sensor: ERROR\n");
        sensor_temp = true;
        count_error = 0;
        
     	   if (current_state.value.int_value != 0) {
        	    current_state.value = HOMEKIT_UINT8(0);
            	homekit_characteristic_notify(&current_state, current_state.value);
            
           	 relay_write(false);
        	}
        }
        
       
    }

	/*	UBaseType_t uxHighWaterMark;
		uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
		printf("Temperature sensor stack >>>>> %d\n", uxHighWaterMark); */
   
}

void thermostat_init()
{

    gpio_enable(BUTTON_UP_GPIO, GPIO_INPUT);
    gpio_enable(BUTTON_DOWN_GPIO, GPIO_INPUT);
    gpio_enable(BUTTON_RESET, GPIO_INPUT);
    gpio_enable(LED_GPIO, GPIO_OUTPUT);
    gpio_enable(RELAY_GPIO, GPIO_OUTPUT);
    gpio_enable(RECIVE_GPIO, GPIO_INPUT);
	gpio_enable(TEMPERATURE_SENSOR_PIN, GPIO_INPUT);
	
    if (button_create(BUTTON_UP_GPIO, 0, 600, button_up_callback)) {
        printf("Failed to initialize button Up\n");
    }

   if (button_create(BUTTON_DOWN_GPIO, 0, 600, button_down_callback)) {
        printf("Failed to initialize button down\n");
    }

    if (button_create(BUTTON_RESET, 0, 10000, reset_button_callback)) {
        printf("Failed to initialize button\n");
    }

	gpio_set_pullup(TEMPERATURE_SENSOR_PIN, false, false);
 
    xTemperatureTimer = xTimerCreate("Temperature Timer",(5000/portTICK_PERIOD_MS),pdTRUE,0, temperature_sensor_task);
    xTimerStart(xTemperatureTimer, 3000 );
    
    xSave_characteristic_Timer = xTimerCreate("Save characteristic Timer",(5000/portTICK_PERIOD_MS),pdFALSE,0, xSave_characteristic_TimerCallback);
   // xTimerStart( xSave_characteristic_Timer, 0 );

}




homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_thermostat, .services=(homekit_service_t*[]) {
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]) {
            &name,
            &manufacturer,
            &serial,
            &model,
            &revision,
            HOMEKIT_CHARACTERISTIC(IDENTIFY, thermostat_identify),
            NULL
        }),
        HOMEKIT_SERVICE(THERMOSTAT, .primary=true, .characteristics=(homekit_characteristic_t*[]) {
            HOMEKIT_CHARACTERISTIC(NAME, "Thermostat"),
            &current_temperature,
            &target_temperature,
            &current_state,
            &target_state,
            &cooling_threshold,
            &heating_threshold,
            &units,
            &current_humidity,
            &ota_trigger,
           
            NULL
        }),
        NULL
    }),
    NULL
};


homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
    };


void on_wifi_ready()
{
	wifi_init_led();

	homekit_server_init(&config);
	
}
void load_settings_from_flash (){
    
    load_characteristic_from_flash(&target_state);
    load_characteristic_from_flash(&target_temperature);
  
}

void create_accessory_name() {
   
	
  uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);
    
    int name_len = snprintf(NULL, 0, "Thermostat-%02X%02X%02X",macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Thermostat-%02X%02X%02X",macaddr[3], macaddr[4], macaddr[5]);
    name.value = HOMEKIT_STRING(name_value);
     
    int serial_len = snprintf(NULL, 0, "%02X:%02X:%02X:%02X:%02X",macaddr[1],macaddr[2],macaddr[3], macaddr[4], macaddr[5]);
    char *serial_value = malloc(serial_len+1);
    snprintf(serial_value, serial_len+1, "%02X:%02X:%02X:%02X:%02X",macaddr[1],macaddr[2],macaddr[3], macaddr[4], macaddr[5]);
    serial.value = HOMEKIT_STRING(serial_value);
   
   
}

   


void user_init(void){
    uart_set_baud(0, 115200);
	load_settings_from_flash();
	create_accessory_name();
    screen_init();
    thermostat_init(); 
    wifi_config_init("HomeKit-Thermostat", NULL, on_wifi_ready);
        
    
   
    
//-------    
}