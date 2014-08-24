#include "pebble.h"
#include "stdlib.h"
#include "string.h"
#include "time.h"

#include "config.h"
#include "my_math.h"
#include "suncalc.h"
  #define KEY_TEMPERATURE 0
  #define KEY_CONDITIONS 1
  #define KEY_ICON 2
  #define KEY_LOCATION 3
  #define KEY_EPOCH 4
// Ninety_Fank Version 2.201
  
		static Window *window;
		static GBitmap *background_image;
		static BitmapLayer *background_layer;



		static TextLayer *DayOfWeekLayer; 
		static TextLayer *moonLayer; 
			//static TextLayer *cwLayer;    >>> I don't care about displaying the week number on my whatchface
		static TextLayer *text_addTimeZone1_layer; 
		static TextLayer *text_addTimeZone2_layer; 
		static TextLayer *text_addTimeZone3_layer; 
		static TextLayer *text_sunrise_layer; 
		static TextLayer *text_sunset_layer; 
		static TextLayer *battery_layer;
		static TextLayer *connection_layer;
			//static TextLayer *second_layer1;
			//static TextLayer *second_layer2;
		static TextLayer *s_weather_layer;
		static TextLayer *weather_location_layer;
		static BitmapLayer *time_format_layer;
		static GBitmap *time_format_image;
			// TODO: Handle 12/24 mode preference when it's exposed.

		static bool initDone; // e.g. for avoiding "no BT" vibration with initial opening of the watchface


		#define TOTAL_TIME_DIGITS 4
		static GBitmap *time_digits_images[TOTAL_TIME_DIGITS];
		static BitmapLayer *time_digits_layers[TOTAL_TIME_DIGITS];
		const int BIG_DIGIT_IMAGE_RESOURCE_IDS[] = {
		  RESOURCE_ID_IMAGE_NUM_0,
		  RESOURCE_ID_IMAGE_NUM_1,
		  RESOURCE_ID_IMAGE_NUM_2,
		  RESOURCE_ID_IMAGE_NUM_3,
		  RESOURCE_ID_IMAGE_NUM_4,
		  RESOURCE_ID_IMAGE_NUM_5,
		  RESOURCE_ID_IMAGE_NUM_6,
		  RESOURCE_ID_IMAGE_NUM_7,
		  RESOURCE_ID_IMAGE_NUM_8,
		  RESOURCE_ID_IMAGE_NUM_9
		};
		unsigned short the_last_hour = 25;
		unsigned short the_last_minute = 61;

		#define TOTAL_DATE_DIGITS 16	// date digits are also used for timezone (since the size is the same)
										// hence: 	date_digits_images[0-3] are reserved for date
										// 			date_digits_images[4-7] are reserved for tz1
										// 			date_digits_images[8-11] are reserved for tz2
										// 			date_digits_images[12-15] are reserved for tz3
		static GBitmap *date_digits_images[TOTAL_DATE_DIGITS];
		static BitmapLayer *date_digits_layers[TOTAL_DATE_DIGITS];
		const int DATENUM_IMAGE_RESOURCE_IDS[] = {
		  RESOURCE_ID_IMAGE_DATENUM_0,
		  RESOURCE_ID_IMAGE_DATENUM_1,
		  RESOURCE_ID_IMAGE_DATENUM_2,
		  RESOURCE_ID_IMAGE_DATENUM_3,
		  RESOURCE_ID_IMAGE_DATENUM_4,
		  RESOURCE_ID_IMAGE_DATENUM_5,
		  RESOURCE_ID_IMAGE_DATENUM_6,
		  RESOURCE_ID_IMAGE_DATENUM_7,
		  RESOURCE_ID_IMAGE_DATENUM_8,
		  RESOURCE_ID_IMAGE_DATENUM_9
		};
		const int DAY_NAME_IMAGE_RESOURCE_IDS[] = {
		  RESOURCE_ID_IMAGE_DAY_NAME_SUN,
		  RESOURCE_ID_IMAGE_DAY_NAME_MON,
		  RESOURCE_ID_IMAGE_DAY_NAME_TUE,
		  RESOURCE_ID_IMAGE_DAY_NAME_WED,
		  RESOURCE_ID_IMAGE_DAY_NAME_THU,
		  RESOURCE_ID_IMAGE_DAY_NAME_FRI,
		  RESOURCE_ID_IMAGE_DAY_NAME_SAT
		};
		static GBitmap *day_name_image;
		static BitmapLayer *day_name_layer;


		#define TOTAL_WEATHER_IMAGES 1
		static GBitmap *weather_images[TOTAL_WEATHER_IMAGES];
		static BitmapLayer *weather_layers[TOTAL_WEATHER_IMAGES];
		const int WEATHER_IMAGE_RESOURCE_IDS[] = {
		  RESOURCE_ID_IMAGE_WEATHER_CLEAR_DAY,                //0, 01d       (01d is the code on openweather API)
			RESOURCE_ID_IMAGE_WEATHER_CLEAR_NIGHT,              //1, 01n
			RESOURCE_ID_IMAGE_WEATHER_PARTLY_CLOUDY_DAY,        //2, 02d
			RESOURCE_ID_IMAGE_WEATHER_PARTLY_CLOUDY_NIGHT,      //3, 02n
			RESOURCE_ID_IMAGE_WEATHER_SLIGHTLY_CLOUDY,          //4, 03d, 03n
			RESOURCE_ID_IMAGE_WEATHER_CLOUDY,                   //5, 04d, 04n
			RESOURCE_ID_IMAGE_WEATHER_DRIZZLE,                  //6, 09d, 09n
			RESOURCE_ID_IMAGE_WEATHER_RAIN,                     //7, 10d, 10n
			RESOURCE_ID_IMAGE_WEATHER_TSTORM,                   //8, 11d, 11n
			RESOURCE_ID_IMAGE_WEATHER_SNOW,                     //9, 13d, 13n
			RESOURCE_ID_IMAGE_WEATHER_FOG,                      //10,50d, 50n
		  RESOURCE_ID_IMAGE_WEATHER_NO_WEATHER                //11, n/a
		};
		#define TOTAL_MOON_IMAGES 1
		static GBitmap *moon_digits_images[TOTAL_MOON_IMAGES];
		static BitmapLayer *moon_images_layers[TOTAL_MOON_IMAGES];
		const int MOON_IMAGE_RESOURCE_IDS[] = {
		  RESOURCE_ID_IMAGE_MOON_0,
		  RESOURCE_ID_IMAGE_MOON_1,
		  RESOURCE_ID_IMAGE_MOON_2,
		  RESOURCE_ID_IMAGE_MOON_3,
		  RESOURCE_ID_IMAGE_MOON_4,
		  RESOURCE_ID_IMAGE_MOON_5,
		  RESOURCE_ID_IMAGE_MOON_6,
		  RESOURCE_ID_IMAGE_MOON_7
		};


// creates generic bitmap layers at top-left position "GPoint origin", with size of bitmap
static void set_container_image(GBitmap **bmp_image, BitmapLayer *bmp_layer, const int resource_id, GPoint origin) {
  GBitmap *old_image = *bmp_image;

  *bmp_image = gbitmap_create_with_resource(resource_id);
  GRect frame = (GRect) {
    .origin = origin,
    .size = (*bmp_image)->bounds.size
  };
  bitmap_layer_set_bitmap(bmp_layer, *bmp_image);
  layer_set_frame(bitmap_layer_get_layer(bmp_layer), frame);

  if (old_image != NULL) {
  	gbitmap_destroy(old_image);
  }
}

// returns an unsigned int for the hour (handling 24h and am-pm mode)
static unsigned short get_display_hour(unsigned short hour) {
  if (clock_is_24h_style()) {
    return hour;
  }

  unsigned short display_hour = hour % 12;

  // Converts "0" to "12"
  return display_hour ? display_hour : 12;
}

// given a string KEY_ICON (from openweather2.5), it returns an int for identifying the weather icon
int weather_id(char weather_string[4]) { 
   if       (strcmp(weather_string, "01d") == 0) { return 0; }
   else   if (strcmp(weather_string, "01n") == 0) { return 1; }
   else   if (strcmp(weather_string, "02d") == 0) { return 2; }
   else   if (strcmp(weather_string, "02n") == 0) { return 3; }
   else   if (strcmp(weather_string, "03d") == 0) { return 4; }
   else   if (strcmp(weather_string, "03n") == 0) { return 4; }
   else   if (strcmp(weather_string, "04d") == 0) { return 5; }
   else   if (strcmp(weather_string, "04n") == 0) { return 5; }
   else   if (strcmp(weather_string, "09d") == 0) { return 6; }
   else   if (strcmp(weather_string, "09n") == 0) { return 6; }
   else   if (strcmp(weather_string, "10d") == 0) { return 7; }
   else   if (strcmp(weather_string, "10n") == 0) { return 7; }
   else   if (strcmp(weather_string, "11d") == 0) { return 8; }
   else   if (strcmp(weather_string, "11n") == 0) { return 8; }
   else   if (strcmp(weather_string, "13d") == 0) { return 9; }
   else   if (strcmp(weather_string, "13n") == 0) { return 9; }
   else   if (strcmp(weather_string, "50d") == 0) { return 10; }
   else   if (strcmp(weather_string, "50n") == 0) { return 10; }
   else                                           { return 11; }
 
}

// returns the moon phase accurate to 1 segment (0-7). (0 is new moon, 4 is full moon)
int moon_phase(int y, int m, int d) {
    int c,e;
    double jd;
    int b;

    if (m < 3) {
        y--;
        m += 12;
    }
    ++m;
    c = 365.25*y;
    e = 30.6*m;
    jd = c+e+d-694039.09;  	/* jd is total days elapsed */
    jd /= 29.53;        	/* divide by the moon cycle (29.53 days) */
    b = jd;		   			/* int(jd) -> b, take integer part of jd */
    jd -= b;		   		/* subtract integer part to leave fractional part of original jd */
    b = jd*8 + 0.5;	   		/* scale fraction from 0-8 and round by adding 0.5 */
    b = b & 7;		   		/* 0 and 8 are the same so turn 8 into 0 */
    return b;
}

// called only within updateSunsetSunrise() to adjust the sunset to the local timezone 
void adjustTimezone(float* time) {
  *time += TIMEZONE;
  *time = *time - 1; // Winter Time - quick&dirty fix
  if (*time >= 24) *time -= 24;		// I prefer seeing 0:00 than 24:00 (as well as -obviously- 1vs25, 2vs26, etc)
  if (*time < 0) *time += 24;
}

// creates a layer for sunset-sunrise
void updateSunsetSunrise() {
	// Calculating Sunrise/sunset with courtesy of Michael Ehrmann
	// https://github.com/mehrmann/pebble-sunclock
	static char sunrise_text[] = "00:00";
	static char sunset_text[]  = "00:00";
	
	//PblTm pblTime;
	//get_time(&pblTime);
  time_t now = time(NULL);
  struct tm *current_time = localtime(&now);

	char *time_format;

	if (clock_is_24h_style()) 
	{
	  time_format = "%R";
	} 
	else 
	{
	  time_format = "%I:%M";
	}

	float sunriseTime = calcSunRise(current_time->tm_year, current_time->tm_mon+1, current_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
	float sunsetTime = calcSunSet(current_time->tm_year, current_time->tm_mon+1, current_time->tm_mday, LATITUDE, LONGITUDE, 91.0f);
	adjustTimezone(&sunriseTime);
	adjustTimezone(&sunsetTime);

	if (!current_time->tm_isdst) 
	{
	  sunriseTime+=1;
	  sunsetTime+=1;
	} 

	current_time->tm_min = (int)(60*(sunriseTime-((int)(sunriseTime))));
	current_time->tm_hour = (int)sunriseTime;
	strftime(sunrise_text, sizeof(sunrise_text), time_format, current_time);
	text_layer_set_text(text_sunrise_layer, sunrise_text);

	current_time->tm_min = (int)(60*(sunsetTime-((int)(sunsetTime))));
	current_time->tm_hour = (int)sunsetTime;
	strftime(sunset_text, sizeof(sunset_text), time_format, current_time);
	text_layer_set_text(text_sunset_layer, sunset_text);
}

// creates a layer for battery
static void handle_battery(BatteryChargeState charge_state) {
  static char battery_text[] = "100%";

  if (charge_state.is_charging) {
    snprintf(battery_text, sizeof(battery_text), "%d%%+", charge_state.charge_percent);  } else {
    snprintf(battery_text, sizeof(battery_text), "%d%%", charge_state.charge_percent);
  }
  text_layer_set_text(battery_layer, battery_text);
}

// creates a layer for bluetooth, and also it vibrate twice if bluetooth down for more than 10sec
static void handle_bluetooth(bool connected) {
  text_layer_set_text(connection_layer, connected ? "BT" : "no BT");
  if( !connected && initDone)
  {
	psleep(10000);  // this is a pebble implementation of the function "sleep" 
    if( !connected && initDone) { vibes_double_pulse(); }
	else   text_layer_set_text(connection_layer, connected ? "BT" : "no BT");
  }
}


// updates date-time layers (all layers except bt and battery, which are not time bond;
// originally Called once per second within init(), Fa changed it to call it once per minute
static void handle_second_tick(struct tm* current_time, TimeUnits units_changed) {
  // Fa: I disabled the following lines as I don't want to display seconds, nor a second_layer
  //static char time_text[] = "00";
  //static char time_text1[] = "0";
  //static char time_text2[] = "0";
  
  //strftime(time_text, sizeof(time_text), "%S", current_time);
  //time_text1[0] = time_text[0];
  //time_text2[0] = time_text[1];
  //text_layer_set_text(second_layer1, time_text1);
  //text_layer_set_text(second_layer2, time_text2);

  unsigned short display_minute = current_time->tm_min;
  
  //execute the following only once per minute (useful if other layers are executed every second)
  if (the_last_minute != display_minute){  
	  the_last_minute = display_minute;
	  unsigned short display_hour = get_display_hour(current_time->tm_hour);


	  // ===== Set Hour ===== 
	  set_container_image(&time_digits_images[0], time_digits_layers[0], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour/10], GPoint(4, 94));
	  set_container_image(&time_digits_images[1], time_digits_layers[1], BIG_DIGIT_IMAGE_RESOURCE_IDS[display_hour%10], GPoint(37, 94));

	  // ===== Set Minute ===== 
	  set_container_image(&time_digits_images[2], time_digits_layers[2], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(80, 94));
	  set_container_image(&time_digits_images[3], time_digits_layers[3], BIG_DIGIT_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(111, 94));
	  	  
	  // ===== Set Time Zone 1 ===== 
	  if (AdditionalTimezone_1 > -24) {
		  text_layer_set_text(text_addTimeZone1_layer, AdditionalTimezone_1_Description); 
		  short  display_hour_tz1 = display_hour AdditionalTimezone_1;
		  if (display_hour_tz1 >= 24) display_hour_tz1 -= 24;		// 24:00 > 0:00, 26:15 > 2:15, etc
		  if (display_hour_tz1 < 0) display_hour_tz1 += 24;
		  set_container_image(&date_digits_images[4], date_digits_layers[4], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz1/10], GPoint(75, 5));
		  set_container_image(&date_digits_images[5], date_digits_layers[5], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz1%10], GPoint(88, 5));  
		  set_container_image(&date_digits_images[6], date_digits_layers[6], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(108, 5));
		  set_container_image(&date_digits_images[7], date_digits_layers[7], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(121, 5));  
	  }
	  // ===== Set Time Zone 2 ===== 
	  if (AdditionalTimezone_2 > -24) {
		  text_layer_set_text(text_addTimeZone2_layer, AdditionalTimezone_2_Description); 
		  short  display_hour_tz2 = display_hour AdditionalTimezone_2;
		  if (display_hour_tz2 > 24) display_hour_tz2 -= 24;
		  if (display_hour_tz2 < 0) display_hour_tz2 += 24;
		  set_container_image(&date_digits_images[8], date_digits_layers[8], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz2/10], GPoint(75, 26));
		  set_container_image(&date_digits_images[9], date_digits_layers[9], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz2%10], GPoint(88, 26));  
		  set_container_image(&date_digits_images[10], date_digits_layers[10], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(108, 26));
		  set_container_image(&date_digits_images[11], date_digits_layers[11], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(121, 26));  
	  }	
		  // ===== Set Time Zone 3 ===== 
	  if (AdditionalTimezone_3 > -24) {
		  text_layer_set_text(text_addTimeZone3_layer, AdditionalTimezone_3_Description); 
		  short  display_hour_tz3 = display_hour AdditionalTimezone_3;
		  if (display_hour_tz3 > 24) display_hour_tz3 -= 24;
		  if (display_hour_tz3 < 0) display_hour_tz3 += 24;
		  set_container_image(&date_digits_images[12], date_digits_layers[12], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz3/10], GPoint(75, 47));
		  set_container_image(&date_digits_images[13], date_digits_layers[13], DATENUM_IMAGE_RESOURCE_IDS[display_hour_tz3%10], GPoint(88, 47));  
		  set_container_image(&date_digits_images[14], date_digits_layers[14], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min/10], GPoint(108, 47));
		  set_container_image(&date_digits_images[15], date_digits_layers[15], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_min%10], GPoint(121, 47));  
	  }
	
	 //execute the following only very 30 minutes (useful if other layers are executed more often)
    // temporarily changed to 5 minutes for testing purposes
    if(display_minute % 30 == 0) {
		 // ===== Set Weather =====
         DictionaryIterator *iter;
         app_message_outbox_begin(&iter);
         dict_write_uint8(iter, 0, 0);
         app_message_outbox_send();		// this sends a request for the phone to gather the current weather
	}
	
    //execute the following only once per hour (useful if other layers are executed more often) 
	 if (the_last_hour != display_hour){  //execute only once per hour
		 the_last_hour = display_hour;
				 //vibes_short_pulse(); 			//Vibrates once per hour
				 // set_container_image(&day_name_image, day_name_layer, DAY_NAME_IMAGE_RESOURCE_IDS[current_time->tm_wday], GPoint(69, 61));				
						// use lcd-like images for day of week instead of standard 3 char (eng only)
		  
		 // ===== Set Sunset-Sunrise ===== 
		 updateSunsetSunrise();

		 // ===== Set Day of Week ====
		 text_layer_set_text(DayOfWeekLayer, DAY_NAME_LANGUAGE[current_time->tm_wday]); 
		 
		 // ===== Set Day Number ===== 
		  set_container_image(&date_digits_images[0], date_digits_layers[0], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday/10], GPoint(day_month_x[0], 71));
		  set_container_image(&date_digits_images[1], date_digits_layers[1], DATENUM_IMAGE_RESOURCE_IDS[current_time->tm_mday%10], GPoint(day_month_x[0] + 13, 71));

		 // ===== Set Month ===== 
		  set_container_image(&date_digits_images[2], date_digits_layers[2], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)/10], GPoint(day_month_x[1], 71));
		  set_container_image(&date_digits_images[3], date_digits_layers[3], DATENUM_IMAGE_RESOURCE_IDS[(current_time->tm_mon+1)%10], GPoint(day_month_x[1] + 13, 71));
		 
		 // ==== Set AM-PM and hide hour's leading 0 ====	(only in am-pm mode, not in 24h mode)
		  if (!clock_is_24h_style()) {
			if (current_time->tm_hour >= 12) {
				layer_set_hidden(bitmap_layer_get_layer(time_format_layer), false);
				set_container_image(&time_format_image, time_format_layer, RESOURCE_ID_IMAGE_PM_MODE, GPoint(10, 78));
			} else { layer_set_hidden(bitmap_layer_get_layer(time_format_layer), true);	}
			if (display_hour >= 10) {
				layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), true);
			} else { layer_set_hidden(bitmap_layer_get_layer(time_digits_layers[0]), false); }
		  }
		  
		// ==== Set Moon_phase ====
		 int moonphase_number;
		 moonphase_number = moon_phase(current_time->tm_year+1900,current_time->tm_mon,current_time->tm_mday);
      	 set_container_image(&moon_digits_images[0], moon_images_layers[0], MOON_IMAGE_RESOURCE_IDS[moonphase_number], GPoint(6, 44));
		 text_layer_set_text(moonLayer, MOONPHASE_NAME_LANGUAGE[moonphase_number]); 
		  
		// ==== Set Calendar week# ====   (Number 1>53 representing the week# in the year, I commented out as I don't care
		 // static char cw_text[] = "XX00";
		 //strftime(cw_text, sizeof(cw_text), TRANSLATION_CW , current_time);
		 //text_layer_set_text(cwLayer, cw_text); 			  
	 } 
  }
}

// following 4 procedures are the answers to app_message_outbox_send() that is invoked every 30 minutes
// handles weather (1 of 4) - request acknowledged successfully
		static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
		  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
		  
		}
// handles weather (2 of 4) - request not acknowledged
		static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
		  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
		/* Set weather icon and text
		  static char weather_layer_buffer[32];
		  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "nack");
		  text_layer_set_text(s_weather_layer, weather_layer_buffer);
		  set_container_image(&weather_images[0], weather_layers[0], WEATHER_IMAGE_RESOURCE_IDS[11], GPoint(2, 2)); 
		 */
		}
// handles weather (3 of 4) - message reception was unsuccessful
		static void inbox_dropped_callback(AppMessageResult reason, void *context) {  
		  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
		  /* Set weather icon and text
		  static char weather_layer_buffer[32];
		  snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "recpt fail");
		  text_layer_set_text(s_weather_layer, weather_layer_buffer);
		  set_container_image(&weather_images[0], weather_layers[0], WEATHER_IMAGE_RESOURCE_IDS[11], GPoint(2, 2)); 
		  */
		}
// handles weather (4 of 4) - message received successfully >> elaboration of the message
		static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
		  // Store incoming information
		  static char temperature_buffer[8];
		  static char conditions_buffer[32];
		  static char icon_buffer[4];
		  static char location_buffer[32];
      static char age_buffer[32];
      time_t epoch;
      struct tm *struct_epoch;
      static char weather_layer_buffer[32];
      static char location_layer_buffer[32];
		  
		  // Read first item
		  Tuple *t = dict_read_first(iterator);

		  // For all items
		  while(t != NULL) {
			// Which key was received?
			switch(t->key) {
			case KEY_TEMPERATURE:
        if ((int)t->value->int32 == -273.15) {
         snprintf(temperature_buffer, sizeof(temperature_buffer), "  -");  
        } else {
         snprintf(temperature_buffer, sizeof(temperature_buffer), "%dC", (int)t->value->int32); 
        }
				break;
			case KEY_CONDITIONS:
				snprintf(conditions_buffer, sizeof(conditions_buffer), "%s", t->value->cstring);
			  break;
			case KEY_ICON:
				snprintf(icon_buffer, sizeof(icon_buffer), "%s", t->value->cstring);
			  break;
			case KEY_LOCATION:
				snprintf(location_buffer, sizeof(location_buffer), "%s", t->value->cstring);
			  break;
			case KEY_EPOCH:
        epoch = (time_t)t->value->int32;
        struct_epoch = localtime(&epoch);
        strftime(age_buffer, sizeof age_buffer, "%H:%M", struct_epoch);
				//snprintf(age_buffer, sizeof(age_buffer), "%d", (int)t->value->int32);
			  break;
			default:
			  APP_LOG(APP_LOG_LEVEL_ERROR, "Key %d not recognized!", (int)t->key);
			  break;
			}

			// Look for next item
			t = dict_read_next(iterator);
		  }
		// Set weather string and display
		snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "%s", temperature_buffer);
				// simplified showing only temp and icon, the original string commented out below also shows conditions detail
				// snprintf(weather_layer_buffer, sizeof(weather_layer_buffer), "  %s\n%s", temperature_buffer, conditions_buffer);  
		text_layer_set_text(s_weather_layer, weather_layer_buffer);
		
		// display location
    snprintf(location_layer_buffer, sizeof(location_layer_buffer), "%s (@ %s)", location_buffer, age_buffer);
		text_layer_set_text(weather_location_layer, location_layer_buffer);
		  
		 // Set weather icon and display
		int current_weather = weather_id(icon_buffer);
		set_container_image(&weather_images[0], weather_layers[0], WEATHER_IMAGE_RESOURCE_IDS[current_weather], GPoint(2, 2)); 
		  
		}

static void init(void) {
  initDone = false;
  memset(&time_digits_layers, 0, sizeof(time_digits_layers));
  memset(&time_digits_images, 0, sizeof(time_digits_images));
  memset(&date_digits_layers, 0, sizeof(date_digits_layers));
  memset(&date_digits_images, 0, sizeof(date_digits_images));
  memset(&moon_images_layers, 0, sizeof(moon_images_layers));
  memset(&moon_digits_images, 0, sizeof(moon_digits_images));
  memset(&weather_layers, 0, sizeof(weather_layers));
  memset(&weather_images, 0, sizeof(weather_images));

  window = window_create();
  if (window == NULL) {
      APP_LOG(APP_LOG_LEVEL_DEBUG, "OOM: couldn't allocate window");
      return;
  }
  window_stack_push(window, true /* Sliding Animation */);
  Layer *window_layer = window_get_root_layer(window);
  if (AdditionalTimezone_2 < -24) 		{  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_1TZ); }
  else if (AdditionalTimezone_3 < -24) 	{  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND); }
  else 									{  background_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_BACKGROUND_3TZ); }
  background_layer = bitmap_layer_create(layer_get_frame(window_layer));
  bitmap_layer_set_bitmap(background_layer, background_image);
  layer_add_child(window_layer, bitmap_layer_get_layer(background_layer));

   // ==== Create and define Day of week Layer ====
  DayOfWeekLayer = text_layer_create(GRect(35, 62, 40 /* width */, 30 /* height */));
  text_layer_set_text_color(DayOfWeekLayer, GColorWhite);
  text_layer_set_background_color(DayOfWeekLayer, GColorClear );
  text_layer_set_font(DayOfWeekLayer, fonts_get_system_font(FONT_KEY_GOTHIC_24_BOLD));
  layer_add_child(window_layer, text_layer_get_layer(DayOfWeekLayer));
  
   // ==== Create and define Moon text Layer ====
  moonLayer = text_layer_create(GRect(20, 41, 30 /* width */, 30 /* height */)); 
  text_layer_set_text_color(moonLayer, GColorWhite);
  text_layer_set_background_color(moonLayer, GColorClear );
  text_layer_set_font(moonLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(moonLayer));  
  
   // ==== Create and define Calendar Week Layer ====
      //cwLayer = text_layer_create(GRect(2, 40, 80 /* width */, 30 /* height */)); 
      //text_layer_set_text_color(cwLayer, GColorWhite);
      //text_layer_set_background_color(cwLayer, GColorClear );
      //text_layer_set_font(cwLayer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
      //layer_add_child(window_layer, text_layer_get_layer(cwLayer));  
  
    
  // ==== Create and define Time Zone 1 Layer ====
  text_addTimeZone1_layer = text_layer_create(GRect(51, 6, 100 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_addTimeZone1_layer, GColorWhite);
  text_layer_set_background_color(text_addTimeZone1_layer, GColorClear );
  text_layer_set_font(text_addTimeZone1_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_addTimeZone1_layer));   
  
  // ==== Create and define Time Zone 2 Layer ====
  text_addTimeZone2_layer = text_layer_create(GRect(51, 26, 100 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_addTimeZone2_layer, GColorWhite);
  text_layer_set_background_color(text_addTimeZone2_layer, GColorClear );
  text_layer_set_font(text_addTimeZone2_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_addTimeZone2_layer));  
  
  // ==== Create and define Time Zone 3 Layer ====
  text_addTimeZone3_layer = text_layer_create(GRect(51, 46, 100 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_addTimeZone3_layer, GColorWhite);
  text_layer_set_background_color(text_addTimeZone3_layer, GColorClear );
  text_layer_set_font(text_addTimeZone3_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_addTimeZone3_layer));  
  
  // ==== Create and define Sunrise Layer ====
  text_sunrise_layer = text_layer_create(GRect(7, 152, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_sunrise_layer, GColorWhite);
  text_layer_set_background_color(text_sunrise_layer, GColorClear );
  text_layer_set_font(text_sunrise_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_sunrise_layer));     
  
  // ==== Create and define Sunset Layer ====
  text_sunset_layer = text_layer_create(GRect(110, 152, 50 /* width */, 30 /* height */)); 
  text_layer_set_text_color(text_sunset_layer, GColorWhite);
  text_layer_set_background_color(text_sunset_layer, GColorClear );
  text_layer_set_font(text_sunset_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  layer_add_child(window_layer, text_layer_get_layer(text_sunset_layer));      
  
  // ==== Create and define BT Connection Layer ====
  connection_layer = text_layer_create(GRect(60, 152, /* width */ 50, 34 /* height */));
  text_layer_set_text_color(connection_layer, GColorWhite);
  text_layer_set_background_color(connection_layer, GColorClear);
  text_layer_set_font(connection_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(connection_layer, GTextAlignmentCenter);
  text_layer_set_text(connection_layer, "BT");
  layer_add_child(window_layer, text_layer_get_layer(connection_layer));  

  // ==== Create and define Battery Layer ====
  battery_layer = text_layer_create(GRect(40, 152, /* width */ 50, 34 /* height */));
  text_layer_set_text_color(battery_layer, GColorWhite);
  text_layer_set_background_color(battery_layer, GColorClear);
  text_layer_set_font(battery_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(battery_layer, GTextAlignmentLeft);
  text_layer_set_text(battery_layer, "100%");  
  layer_add_child(window_layer, text_layer_get_layer(battery_layer));  
  
  
  // ==== Create and define weather text Layer ====
  s_weather_layer = text_layer_create(GRect(2, 24, /* width */100, 60/* height */));
  text_layer_set_text_color(s_weather_layer, GColorWhite);
  text_layer_set_background_color(s_weather_layer, GColorClear);
  text_layer_set_font(s_weather_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(s_weather_layer, GTextAlignmentLeft);
  text_layer_set_text(s_weather_layer, " ");
  layer_add_child(window_layer, text_layer_get_layer(s_weather_layer));
  
  // ==== Create and define weather location text Layer ====
  weather_location_layer = text_layer_create(GRect(2, 134, /* width */142, 40/* height */));
  text_layer_set_text_color(weather_location_layer, GColorWhite);
  text_layer_set_background_color(weather_location_layer, GColorClear);
  text_layer_set_font(weather_location_layer, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  text_layer_set_text_alignment(weather_location_layer, GTextAlignmentCenter);
  text_layer_set_text(weather_location_layer, " ");
  layer_add_child(window_layer, text_layer_get_layer(weather_location_layer));

 
  
  // ==== Create and define AM-PM Layer ====
  if (!clock_is_24h_style()) {
    time_format_image = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_24_HOUR_MODE);
    GRect frame = (GRect) {
      .origin = { .x = 17, .y = 68 },
      .size = time_format_image->bounds.size
    };
    time_format_layer = bitmap_layer_create(frame);
    bitmap_layer_set_bitmap(time_format_layer, time_format_image);
    layer_add_child(window_layer, bitmap_layer_get_layer(time_format_layer));
  }

   GRect dummy_frame = { {0, 0}, {0, 0} };
 // ==== Create and define time layer ====
  for (int i = 0; i < TOTAL_TIME_DIGITS; ++i) {
    time_digits_layers[i] = bitmap_layer_create(dummy_frame);
	   bitmap_layer_set_background_color(time_digits_layers[i], GColorClear );
    layer_add_child(window_layer, bitmap_layer_get_layer(time_digits_layers[i]));
  }
  // ==== Create and define date layer ====
  for (int i = 0; i < TOTAL_DATE_DIGITS; ++i) {
    date_digits_layers[i] = bitmap_layer_create(dummy_frame);
	   bitmap_layer_set_background_color(date_digits_layers[i], GColorClear );
    layer_add_child(window_layer, bitmap_layer_get_layer(date_digits_layers[i]));
  }
  // ==== Create and define moon icon layer ====
  for (int i = 0; i < TOTAL_MOON_IMAGES; ++i) {
    moon_images_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(moon_images_layers[i]));
  }
  // ==== Create and define weather icon layer ====
  for (int i = 0; i < TOTAL_WEATHER_IMAGES; ++i) {
    weather_layers[i] = bitmap_layer_create(dummy_frame);
    layer_add_child(window_layer, bitmap_layer_get_layer(weather_layers[i]));
  }

  
  //  // ==== Create and define Second Layers ====
  //second_layer1 = text_layer_create(GRect(89, 114, 50 /* width */, 30 /* height */)); 
  //text_layer_set_text_color(second_layer1, GColorWhite);
  //text_layer_set_background_color(second_layer1, GColorClear );
  //text_layer_set_font(second_layer1, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  //layer_add_child(window_layer, text_layer_get_layer(second_layer1));      
  //second_layer2 = text_layer_create(GRect(121, 114, 50 /* width */, 30 /* height */)); 
  //text_layer_set_text_color(second_layer2, GColorWhite);
  //text_layer_set_background_color(second_layer2, GColorClear );
  //text_layer_set_font(second_layer2, fonts_get_system_font(FONT_KEY_GOTHIC_14));
  //layer_add_child(window_layer, text_layer_get_layer(second_layer2));  
	
	

  // Avoids a blank screen on watch start.
  time_t now = time(NULL);
  struct tm *tick_time = localtime(&now);

  // update_display(tick_time); // causes wrong time displayed initially


  
  // ==== call funcions to populate all layers ====
  handle_second_tick(tick_time, MINUTE_UNIT);
  tick_timer_service_subscribe(MINUTE_UNIT, &handle_second_tick);
	// Fa: i changed Second tick to what is really a "Minute tick" as I didn't care about seconds being shown
  battery_state_service_subscribe(&handle_battery);
  handle_battery(battery_state_service_peek());
  handle_bluetooth(bluetooth_connection_service_peek());
  bluetooth_connection_service_subscribe(&handle_bluetooth);

  
  // Register callbacks for weather messages received
  // and open AppMessage, setting the inbox and outbox size to the max possible for current firmware
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());


  initDone = true;
}


static void deinit(void) {
  layer_remove_from_parent(bitmap_layer_get_layer(background_layer));
  bitmap_layer_destroy(background_layer);
  gbitmap_destroy(background_image);

  layer_remove_from_parent(bitmap_layer_get_layer(time_format_layer));
  bitmap_layer_destroy(time_format_layer);
  gbitmap_destroy(time_format_image);

  layer_remove_from_parent(bitmap_layer_get_layer(day_name_layer));
  bitmap_layer_destroy(day_name_layer);
  gbitmap_destroy(day_name_image);
  
  text_layer_destroy(DayOfWeekLayer);
  text_layer_destroy(moonLayer);
  // text_layer_destroy(cwLayer);
  text_layer_destroy(text_addTimeZone1_layer);
  text_layer_destroy(text_addTimeZone2_layer);  
  text_layer_destroy(text_addTimeZone3_layer);  
  text_layer_destroy(text_sunrise_layer);
  text_layer_destroy(text_sunset_layer);  
  text_layer_destroy(connection_layer);
  text_layer_destroy(battery_layer);  
  //text_layer_destroy(second_layer1);  
  //text_layer_destroy(second_layer2); 
  text_layer_destroy(s_weather_layer);
  text_layer_destroy(weather_location_layer);
  
 
  for (int i = 0; i < TOTAL_DATE_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(date_digits_layers[i]));
    gbitmap_destroy(date_digits_images[i]);
    bitmap_layer_destroy(date_digits_layers[i]);
  }
  
   for (int i = 0; i < TOTAL_MOON_IMAGES; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(moon_images_layers[i]));
    gbitmap_destroy(moon_digits_images[i]);
    bitmap_layer_destroy(moon_images_layers[i]);
  } 
  
 for (int i = 0; i < TOTAL_WEATHER_IMAGES; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(weather_layers[i]));
    gbitmap_destroy(weather_images[i]);
    bitmap_layer_destroy(weather_layers[i]);
  }
  

  for (int i = 0; i < TOTAL_TIME_DIGITS; i++) {
    layer_remove_from_parent(bitmap_layer_get_layer(time_digits_layers[i]));
    gbitmap_destroy(time_digits_images[i]);
    bitmap_layer_destroy(time_digits_layers[i]);
  }
  battery_state_service_unsubscribe();
  bluetooth_connection_service_unsubscribe();
  tick_timer_service_unsubscribe();
  window_destroy( window );
  
  // Fa note:   app_message_register_inbox_received and app_message_open are not unsubscribed in the examples, so I presume they don't need to be in deinit
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
