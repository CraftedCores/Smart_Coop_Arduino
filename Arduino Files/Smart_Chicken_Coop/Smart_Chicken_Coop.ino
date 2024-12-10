#include "Arduino_H7_Video.h"
#include "Arduino_GigaDisplayTouch.h"
#include "lvgl.h"
#include "ui.h"
#include <WiFi.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

/* Insert resolution WxH according to your SquareLine studio project settings */
Arduino_H7_Video          Display(800, 480, GigaDisplayShield); 
Arduino_GigaDisplayTouch  Touch;

// WiFi credentials
const char* ssid = "";
const char* password = "";

// NTP client setup
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org");

// Idaho timezone offsets
const int MST_OFFSET = -25200; // UTC-7
const int MDT_OFFSET = -21600; // UTC-6

// Determine if DST is active
bool isDSTActive() {
  time_t now = timeClient.getEpochTime();
  struct tm* timeInfo = gmtime(&now);

  // Check if the current date falls within DST range (March-November)
  int month = timeInfo->tm_mon + 1; // tm_mon is 0-based
  int day = timeInfo->tm_mday;
  int weekday = timeInfo->tm_wday; // 0 = Sunday

  // DST starts on the second Sunday of March
  if (month == 3) {
    int secondSunday = 8 + (7 - weekday) % 7;
    return day >= secondSunday;
  }

  // DST ends on the first Sunday of November
  if (month == 11) {
    int firstSunday = 1 + (7 - weekday) % 7;
    return day < firstSunday;
  }

  // DST is active from April to October
  return month > 3 && month < 11;
}

void updateTime() {
  timeClient.update();

  // Adjust for timezone
  int timezoneOffset = isDSTActive() ? MDT_OFFSET : MST_OFFSET;
  time_t localTime = timeClient.getEpochTime() + timezoneOffset;

  // Format time
  struct tm* timeInfo = gmtime(&localTime);
  char dayBuffer[10];
  char monthBuffer[15];
  char yearBuffer[5];
  char timeBuffer[15];

  strftime(dayBuffer, sizeof(dayBuffer), "%A", timeInfo);            // Day: Monday
  strftime(monthBuffer, sizeof(monthBuffer), "%B", timeInfo);       // Month: December
  strftime(yearBuffer, sizeof(yearBuffer), "%Y", timeInfo);         // Year: 2024
  strftime(timeBuffer, sizeof(timeBuffer), "%I:%M %p", timeInfo); // Time: 02:30:00 PM

  // Extract day without leading zero
  int day = timeInfo->tm_mday;

  // Create the full date string
  char dateBuffer[30];
  snprintf(dateBuffer, sizeof(dateBuffer), "%s %d, %s", monthBuffer, day, yearBuffer);

  // Update labels
  lv_label_set_text(ui_DayWeekTitle, dayBuffer);
  lv_label_set_text(ui_DateTitle, dateBuffer);
  lv_label_set_text(ui_TimeTitle, timeBuffer);
}


static void LightButton_evt_handler(lv_event_t * e) {
  bool checked = lv_obj_has_state(ui_LightButton, LV_STATE_CHECKED);
	if (checked){
    lv_label_set_text(ui_LightLabel, "On");
    digitalWrite(12, HIGH);
  }
	else{
    lv_label_set_text(ui_LightLabel, "Off");
    digitalWrite(12, LOW);
  }
}

static void CoopHeatButton_evt_handler(lv_event_t * e) {
  bool checked = lv_obj_has_state(ui_CoopHeatButton, LV_STATE_CHECKED);
	if (checked){
    lv_label_set_text(ui_CoopHeatLabel, "On");
    digitalWrite(11, HIGH);
  }
	else{
		lv_label_set_text(ui_CoopHeatLabel, "Off");
    digitalWrite(11, LOW);    
  }
}

static void WaterHeatbutton_evt_handler(lv_event_t * e) {
  bool checked = lv_obj_has_state(ui_WaterHeatbutton, LV_STATE_CHECKED);
	if (checked){
		lv_label_set_text(ui_WaterHeatLabel, "On");
    digitalWrite(10, HIGH);    
  }
	else{
		lv_label_set_text(ui_WaterHeatLabel, "Off");
    digitalWrite(10, LOW);    
  }
}

static void DoorButton_evt_handler(lv_event_t * e) {
  bool checked = lv_obj_has_state(ui_DoorButton, LV_STATE_CHECKED);
	if (checked){
		lv_label_set_text(ui_DoorLabel, "Open");
    digitalWrite(9, HIGH);    
  }
	else{
		lv_label_set_text(ui_DoorLabel, "Closed");
    digitalWrite(9, LOW);    
  }
}



void setup() {

  pinMode(12, OUTPUT);
  pinMode(11, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(9, OUTPUT);

  // WiFi setup
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");

  // Start NTP client
  timeClient.begin();

  Display.begin();
  Touch.begin();

  ui_init();

  /* Add buttons event handlers */
  lv_obj_add_event_cb(ui_LightButton, LightButton_evt_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_CoopHeatButton, CoopHeatButton_evt_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_WaterHeatbutton, WaterHeatbutton_evt_handler, LV_EVENT_ALL, NULL);
  lv_obj_add_event_cb(ui_DoorButton, DoorButton_evt_handler, LV_EVENT_ALL, NULL);

}

void loop() {

  /* Feed LVGL engine */
  lv_timer_handler();

  // Update time every second
  static unsigned long lastUpdate = 0;
  if (millis() - lastUpdate > 1000) {
    updateTime();
    lastUpdate = millis();
  }
}