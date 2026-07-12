#include <pebble.h>

static Window *s_main_window;
static BitmapLayer *s_bgart_layer;
static TextLayer *s_hour_layer;
static TextLayer *s_min_layer;
static BitmapLayer *s_bat_layer;
static GBitmap *s_bat_ch1_bitmap;
static GBitmap *s_bat_ch2_bitmap;
static GBitmap *s_bat_ch3_bitmap;
static GBitmap *s_bat_low_bitmap;
static GBitmap *s_bat_full_bitmap;
static GBitmap *s_bgart_bitmap;

const int text_h = 28;
const int text_w = 64;
const int hour_x = 112;
const int hour_y = 86;
const int min_x = hour_x + 8;
const int min_y = hour_y + 24;
const int bat_w = 32;
const int bat_h = 11;
const int bat_inner_w = 20;

static void update_screen() {
  // Time
  time_t temp = time(NULL);
  struct tm *tick_time = localtime(&temp);
  static char s_hour_buf[4];
  static char s_min_buf[4];
  strftime(s_hour_buf, sizeof(s_hour_buf), clock_is_24h_style() ? "%H" : "%I", tick_time);
  strftime(s_min_buf, sizeof(s_min_buf), "%M", tick_time);
  text_layer_set_text(s_hour_layer, s_hour_buf);
  text_layer_set_text(s_min_layer, s_min_buf);
  
  // Battery
  BatteryChargeState battery_info = battery_state_service_peek();
  layer_set_hidden((Layer *) s_bat_layer, false);
  if (battery_info.is_charging && battery_info.charge_percent <= 33) {
    bitmap_layer_set_bitmap(s_bat_layer, s_bat_ch1_bitmap);
  } else if (battery_info.is_charging && battery_info.charge_percent <= 66) {
    bitmap_layer_set_bitmap(s_bat_layer, s_bat_ch2_bitmap);
  } else if (battery_info.is_charging && battery_info.charge_percent) {
    bitmap_layer_set_bitmap(s_bat_layer, s_bat_ch3_bitmap);
  } else if (battery_info.is_plugged) {
    bitmap_layer_set_bitmap(s_bat_layer, s_bat_full_bitmap);
  } else if (battery_info.charge_percent < 15) {
    bitmap_layer_set_bitmap(s_bat_layer, s_bat_low_bitmap);
  } else {
    layer_set_hidden((Layer *) s_bat_layer, true);
  }
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
  update_screen();
}

static void bat_handler(BatteryChargeState state) {
  update_screen();
}

static void main_window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

  s_bgart_layer = bitmap_layer_create(bounds);
  s_bgart_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BGART_A);
  bitmap_layer_set_bitmap(s_bgart_layer, s_bgart_bitmap);
  bitmap_layer_set_compositing_mode(s_bgart_layer, GCompOpSet);

  s_hour_layer = text_layer_create(GRect(hour_x, hour_y, text_w, text_h));
  text_layer_set_background_color(s_hour_layer, GColorClear);
  text_layer_set_text_color(s_hour_layer, GColorBlack);
  text_layer_set_font(s_hour_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_hour_layer, GTextAlignmentLeft);

  s_min_layer = text_layer_create(GRect(min_x, min_y, text_w, text_h));
  text_layer_set_background_color(s_min_layer, GColorClear);
  text_layer_set_text_color(s_min_layer, GColorBlack);
  text_layer_set_font(s_min_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28_BOLD));
  text_layer_set_text_alignment(s_min_layer, GTextAlignmentLeft);
  
  int bat_x = (bounds.size.w - 12) / 2;
  s_bat_layer = bitmap_layer_create(GRect(bat_x, bat_h, bat_w, bat_h));
  s_bat_ch1_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_CH1);
  s_bat_ch2_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_CH2);
  s_bat_ch3_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_CH3);
  s_bat_low_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_LOW);
  s_bat_full_bitmap = gbitmap_create_with_resource(RESOURCE_ID_IMG_BAT_FULL);
  bitmap_layer_set_compositing_mode(s_bat_layer, GCompOpSet);
  bitmap_layer_set_alignment(s_bat_layer, GAlignLeft);

  layer_add_child(window_layer, bitmap_layer_get_layer(s_bgart_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_hour_layer));
  layer_add_child(window_layer, text_layer_get_layer(s_min_layer));
  layer_add_child(window_layer, bitmap_layer_get_layer(s_bat_layer));
}

static void main_window_unload(Window *window) {
  bitmap_layer_destroy(s_bgart_layer);
  text_layer_destroy(s_hour_layer);
  text_layer_destroy(s_min_layer);
  bitmap_layer_destroy(s_bat_layer);
}

static void init() {
  s_main_window = window_create();

  window_set_window_handlers(s_main_window, (WindowHandlers) {
    .load = main_window_load,
    .unload = main_window_unload
  });

  window_stack_push(s_main_window, true); // animated=true
  update_screen();
  tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
  battery_state_service_subscribe(bat_handler);
}

static void deinit() {
  window_destroy(s_main_window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}
