#include <pebble.h>
#include "main.h"

#define NUM_TIMETABLE_ENTRIES  5


static Window*  s_main_window;
static TextLayer* s_time_layer;
static TextLayer* s_timetable_layer[NUM_TIMETABLE_ENTRIES];

struct TimetableEntry {
  char service[10];
  char destination[10];
  char time[8];
  char linebuf[16];
  int liveFeed;
};


struct TimetableEntry entries[NUM_TIMETABLE_ENTRIES];
int next_timetable = 0;
static void update_time();
static void update_timetable();

static int min(int a, int b){
  return a < b? a: b;
}

static void handle_timetable_entry(DictionaryIterator *iterator){
  Tuple *CLEAR = dict_find(iterator, MESSAGE_KEY_CLEAR);
  if(CLEAR){
    APP_LOG(APP_LOG_LEVEL_INFO,"Clearing timetable");
    next_timetable = 0;
    for(int i = 0; i < NUM_TIMETABLE_ENTRIES; i++){
      entries[next_timetable].linebuf[0] = '\0';
    }
    return;
  }
  
  Tuple *NEXT = dict_find(iterator, MESSAGE_KEY_NEXT);
  if(NEXT){
    next_timetable = NEXT->value->int32;
    APP_LOG(APP_LOG_LEVEL_INFO, "Next timetable slot is now %d",next_timetable);
  }

  Tuple *timeval = dict_find(iterator, MESSAGE_KEY_time);
  Tuple *service = dict_find(iterator,MESSAGE_KEY_service);
  Tuple *destination = dict_find(iterator,MESSAGE_KEY_destination);
  Tuple *liveFeed = dict_find(iterator,MESSAGE_KEY_isLiveFeed);
  if(timeval && service && destination && liveFeed && next_timetable < NUM_TIMETABLE_ENTRIES){
    APP_LOG(APP_LOG_LEVEL_INFO,"Have got %s for service %s to %s", timeval->value->cstring,service->value->cstring,destination->value->cstring);
    struct TimetableEntry *entry = &entries[next_timetable];
    entry -> liveFeed = liveFeed->value->int16;
    snprintf(entry->time,sizeof entry->time,"%s",timeval->value->cstring);
    snprintf(entry->service,sizeof entry->service,"%s",service->value->cstring);
    snprintf(entry->destination,sizeof entry->destination,"%s",destination->value->cstring);
    
    snprintf(entry->linebuf, sizeof entry->linebuf, "%s %s %s", entry->time,entry->service,entry->destination);
    
    APP_LOG(APP_LOG_LEVEL_INFO,"Have got %s for service %s to %s", entry->time,entry->service,entry->destination);
    update_timetable();
    return;
  }
  APP_LOG(APP_LOG_LEVEL_INFO,"Have an unknown message");
}



static void inbox_received_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Received message!");
  handle_timetable_entry(iterator);
}

static void inbox_dropped_callback(AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Message dropped!");
}

static void outbox_failed_callback(DictionaryIterator *iterator, AppMessageResult reason, void *context) {
  APP_LOG(APP_LOG_LEVEL_ERROR, "Outbox send failed!");
}

static void outbox_sent_callback(DictionaryIterator *iterator, void *context) {
  APP_LOG(APP_LOG_LEVEL_INFO, "Outbox send success!");
}




static void main_window_load(Window* window){
  Layer* window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  s_time_layer = text_layer_create(
    GRect(0,PBL_IF_ROUND_ELSE(70,64),bounds.size.w, 50)
  );
  
  
  for(int i = 0; i < NUM_TIMETABLE_ENTRIES; i++){
    int place = 10 + i * 12;
    s_timetable_layer[i] = text_layer_create(
      GRect(0,PBL_IF_ROUND_ELSE(place,place-5),bounds.size.w,50)
    );
  }
  
  text_layer_set_background_color(s_time_layer,GColorClear);
  text_layer_set_text_color(s_time_layer,GColorBlack);
  text_layer_set_text(s_time_layer,"00:00");
  text_layer_set_font(s_time_layer,fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
  text_layer_set_text_alignment(s_time_layer,GTextAlignmentCenter);
  
  layer_add_child(window_layer,text_layer_get_layer(s_time_layer));
  
  
  for(int i = 0; i < NUM_TIMETABLE_ENTRIES; i++){
    TextLayer *text_layer = s_timetable_layer[i];
    
    text_layer_set_background_color(text_layer,GColorClear);
    text_layer_set_text_color(text_layer,GColorBlack);
    text_layer_set_text(text_layer,"No data");
    text_layer_set_font(text_layer,fonts_get_system_font(FONT_KEY_GOTHIC_14));
    text_layer_set_text_alignment(text_layer,GTextAlignmentCenter);

    layer_add_child(window_layer,text_layer_get_layer(text_layer));
    
    struct TimetableEntry *entry = &entries[i];
    text_layer_set_text(text_layer, entry->linebuf);
  }
}

static void main_window_unload(Window* window){
  text_layer_destroy(s_time_layer); 
  for(int i = 0; i < NUM_TIMETABLE_ENTRIES; i++){
    text_layer_destroy(s_timetable_layer[i]); 
  }
}

static void update_time() {
  // Get a tm structure
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);

  // Write the current hours and minutes into a buffer
  static char s_buffer[8];
  strftime(s_buffer, sizeof(s_buffer), clock_is_24h_style() ?
                                          "%H:%M" : "%I:%M", tick_time);

  // Display this time on the TextLayer
  text_layer_set_text(s_time_layer, s_buffer);
}

static void update_timetable(){
  for(int n = 0; n < min(NUM_TIMETABLE_ENTRIES,next_timetable+1);n++){    
    struct TimetableEntry *entry = &entries[n];
    text_layer_set_text(s_timetable_layer[n], entry->linebuf);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_time();
  update_timetable();
}

static void init(){
  
  app_message_register_inbox_received(inbox_received_callback);
  app_message_register_inbox_dropped(inbox_dropped_callback);
  app_message_register_outbox_failed(outbox_failed_callback);
  app_message_register_outbox_sent(outbox_sent_callback);
  
  const int inbox_size = 512;
  const int outbox_size = 128;
  app_message_open(inbox_size, outbox_size);
  
  s_main_window = window_create();
  
  window_set_window_handlers(s_main_window, (WindowHandlers){
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window,true);
  
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  update_time();
  update_timetable();
}

static void deinit(){
  window_destroy(s_main_window);
}



int main(){
  init();
  app_event_loop();
  deinit();
  return 0;
}