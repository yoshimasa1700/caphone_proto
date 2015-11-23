#include <pebble.h>

#include "showdigits.h"
#include "accsensorfilter.h"

static Window *window;
static TextLayer *text_layer;
static TextLayer *hour_text, *minute_text, *second_text;
static TextLayer *acc_text;


static GRect current_hour_rect, current_minute_rect, current_second_rect;
static GRect hour_rect, minute_rect, second_rect;

static GRect bounds;

bool changing = false;
int changecount = 0;

bool show_big_font = true;
int current_selected_item = 1;

char hour[] = "00";
char minute[] = "00";
char second[] = "00";

// アニメーションが停止した際のハンドラ
void on_animation_stopped(Animation *anim, bool finished, void *context){
  // メモリ解放
  property_animation_destroy((PropertyAnimation*) anim);

  show_big_font = true;
}

// アニメーションレイヤ
void animate_layer(Layer *layer, GRect *start, GRect *finish, int duration, int delay){
  // アニメーション構造体の宣言
  PropertyAnimation *anim = property_animation_create_layer_frame(layer, start, finish);

  // アニメの特徴を設定
  animation_set_duration((Animation*)anim, duration);
  animation_set_delay((Animation*)anim, delay);

  // アニメが停止した際にメモリを解放するためのハンドラを設定
  AnimationHandlers handlers = {
    .stopped = (AnimationStoppedHandler) on_animation_stopped
  };
  animation_set_handlers((Animation*)anim, handlers, NULL);

  unload_digit_image_from_slot(0);
  unload_digit_image_from_slot(1);

  show_big_font = false;
  
  // アニメーション開始
  animation_schedule((Animation*)anim);
}


static void accel_raw_handler(AccelData *data, uint32_t num_samples)
{
  /* double z = data[0].z + 980; */
  /* double accVecLength = data[0].x * data[0].x */
  /* 			     + data[0].y * data[0].y */
  /* 			     + z * z; */

  double vec_length = accel_sensor_filter(data);
  
  if (3 < vec_length){
    changing = true;
    changecount = 0;
  }

  static char buffer[] = "9999";
  snprintf(buffer, sizeof("9999"), "%d", (int)vec_length);
  // text_layer_set_text(acc_text, buffer);
}


void set_current_state(){
  current_hour_rect = hour_rect;
  current_minute_rect = minute_rect;
  current_second_rect = second_rect;
}

static GRect main_rect;

void set_hour_state(){
  set_current_state();

  hour_rect = main_rect;
  text_layer_set_font(hour_text, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  minute_rect = GRect(bounds.size.w / 2 - 10, bounds.size.h - 20, 20, 20);
  text_layer_set_font(minute_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  second_rect = GRect(126, bounds.size.h - 20, 20, 20);
  text_layer_set_font(second_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  current_selected_item = 0;
}

void set_minute_state(){
  set_current_state();

  hour_rect = GRect(0, bounds.size.h - 20, 20, 20);
  text_layer_set_font(hour_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  minute_rect = main_rect;
  text_layer_set_font(minute_text, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  second_rect = GRect(126, bounds.size.h - 20, 20, 20);
  text_layer_set_font(second_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));

  current_selected_item = 1;
}

void set_second_state(){
  set_current_state();

  hour_rect = GRect(0, bounds.size.h - 20, 20, 20);
  text_layer_set_font(hour_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  minute_rect = GRect(bounds.size.w / 2 - 10, bounds.size.h - 20, 20, 20);
  text_layer_set_font(minute_text, fonts_get_system_font(FONT_KEY_GOTHIC_18_BOLD));
  second_rect = main_rect;
  text_layer_set_font(second_text, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));

  current_selected_item = 2;
}


void start_animation(){
  animate_layer(text_layer_get_layer(hour_text), &current_hour_rect, &hour_rect, 300, 500);
  animate_layer(text_layer_get_layer(minute_text), &current_minute_rect, &minute_rect, 300, 500);
  animate_layer(text_layer_get_layer(second_text), &current_second_rect, &second_rect, 300, 500);
}

void tick_handler(struct tm *tick_time, TimeUnits units_changed){
  // update window
  strftime(hour, sizeof("00"), "%H", tick_time);
  strftime(minute, sizeof("00"), "%M", tick_time);
  strftime(second, sizeof("00"), "%S", tick_time);

  text_layer_set_text(hour_text, hour);
  text_layer_set_text(minute_text, minute);
  text_layer_set_text(second_text, second);

  int seconds = tick_time->tm_sec;
  int minutes = tick_time->tm_min;
  int hours = tick_time->tm_hour;

  if (changing){
    changecount++;
  }else{
    changecount = 0;
  }

  if(changecount == 1){
    set_hour_state();
    start_animation();
  }else if(changecount == 3){
    set_minute_state();
    start_animation();
  }else if(changecount == 5){
    set_second_state();
    start_animation();
  }else if(10 < changecount){
    changing = false;
  }

  if (show_big_font){
    switch(current_selected_item){
    case 0:
      display_value(window, hours, 0, true);
      break;
    case 1:
      display_value(window, minutes, 0, true);
      break;
    case 2:
      display_value(window, seconds, 0, true);
      break;
    default:
      break;
    }
  }else{
    unload_digit_image_from_slot(0);
    unload_digit_image_from_slot(1);
  }
}

void init_time(){
  struct tm *t;
  time_t temp;
  temp = time(NULL);
  t = localtime(&temp);

  tick_handler(t, SECOND_UNIT);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  bounds = layer_get_bounds(window_layer);

  hour_text = text_layer_create(GRect(0, bounds.size.h - 20, 20, 20));
  text_layer_set_background_color(hour_text, GColorWhite);
  text_layer_set_text_color(hour_text, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(hour_text));

  hour_rect = GRect(0, bounds.size.h - 20, 20, 20);

  text_layer_set_text(hour_text, "12");

  minute_text = text_layer_create(GRect(0, 0, bounds.size.w, bounds.size.h - 20));
  text_layer_set_background_color(minute_text, GColorWhite);
  text_layer_set_text_color(minute_text, GColorBlack);
  text_layer_set_font(minute_text, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
  layer_add_child(window_layer, text_layer_get_layer(minute_text));

  minute_rect = GRect(0, 0, bounds.size.w, bounds.size.h - 20);

  text_layer_set_text(minute_text, "53");

  second_text = text_layer_create(GRect(126, bounds.size.h - 20, 20, 20));
  text_layer_set_background_color(second_text, GColorWhite);
  text_layer_set_text_color(second_text, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(second_text));

  second_rect = GRect(126, bounds.size.h - 20, 20, 20);
  
  text_layer_set_text(second_text, "23");

  acc_text = text_layer_create(GRect(100, bounds.size.h - 40, 80, 20));
  text_layer_set_background_color(acc_text, GColorWhite);
  text_layer_set_text_color(acc_text, GColorBlack);
  layer_add_child(window_layer, text_layer_get_layer(acc_text));

  init_time();
  accel_data_service_subscribe(1, accel_raw_handler);

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
}

static void init(void) {
  window = window_create();
  // window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  main_rect = GRect(50, 40, 94, 108);
  
  const bool animated = true;
  window_stack_push(window, animated);

  tick_timer_service_subscribe(SECOND_UNIT, (TickHandler) tick_handler);
  accel_data_service_subscribe(1, accel_raw_handler);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

  app_event_loop();
  deinit();
}
