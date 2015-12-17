#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;
static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;
static GFont s_rwby_time_font;
static GFont s_rwby_date_font;
static int s_battery_level;
static Layer *s_battery_layer;
static Layer *s_battery_background_layer;
static BitmapLayer *s_background_layer, *s_bt_icon_layer;
static GBitmap *s_background_bitmap, *s_bt_icon_bitmap;

static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    // Display time
    static char s_time_buffer[8];
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    text_layer_set_text(s_time_layer, s_time_buffer);
}

static void update_date() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    // Display date
    static char s_date_buffer[12];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %d %b", tick_time);
    text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    if((units_changed & MINUTE_UNIT) != 0) {
        update_time();
    }

    if((units_changed & DAY_UNIT) != 0) {
        update_date();
    }
}

static void battery_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);

    // Find the width of the bar
    int width = (int)(float)(((float)s_battery_level / 100.0F) * bounds.size.w);

    // Draw the background
    graphics_context_set_fill_color(ctx, GColorWhite);
    graphics_fill_rect(ctx, bounds, 8, GCornersAll);

    // Draw the bar
    graphics_context_set_fill_color(ctx, GColorBlack);
    graphics_fill_rect(ctx, GRect(0, 0, width, bounds.size.h), 8, GCornersAll);
}

static void battery_background_update_proc(Layer *layer, GContext *ctx) {
    GRect bounds = layer_get_bounds(layer);
    graphics_context_set_fill_color(ctx, GColorBlack);    
    graphics_fill_rect(ctx, bounds, 8, GCornersAll);
}

static void battery_callback(BatteryChargeState state) {
    s_battery_level = state.charge_percent;
    layer_mark_dirty(s_battery_layer);
}

static void bluetooth_callback(bool connected) {
  // Show icon if disconnected
  layer_set_hidden(bitmap_layer_get_layer(s_bt_icon_layer), connected);

  if(!connected) {
    // Issue a vibrating alert
    vibes_double_pulse();
  }
}

static void main_window_load(Window *window) {
    // Get information about the Window
    Layer *window_layer = window_get_root_layer(window);
    GRect bounds = layer_get_bounds(window_layer);

    // Show bitmap
    s_background_bitmap = gbitmap_create_with_resource(RESOURCE_ID_QROW_EMBLEM);
    s_background_layer = bitmap_layer_create(bounds);
    bitmap_layer_set_bitmap(s_background_layer, s_background_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_background_layer));

    // Create fonts
    s_rwby_time_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_RWBY_TIME_FONT_48));
    s_rwby_date_font = fonts_load_custom_font(resource_get_handle(RESOURCE_ID_RWBY_DATE_FONT_20));

    // Show time
    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(10, 2), bounds.size.w, 50));
    text_layer_set_font(s_time_layer, s_rwby_time_font);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Show date
    s_date_layer = text_layer_create(GRect(0, 140, bounds.size.w, 50));
    text_layer_set_font(s_date_layer, s_rwby_date_font);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorBlack);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));

    // Show battery
    s_battery_background_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(24, 14), PBL_IF_ROUND_ELSE(135, 130), bounds.size.w - PBL_IF_ROUND_ELSE(48, 28), 6));
    layer_set_update_proc(s_battery_background_layer, battery_background_update_proc);
    layer_add_child(window_get_root_layer(window), s_battery_background_layer);
    s_battery_layer = layer_create(GRect(PBL_IF_ROUND_ELSE(25, 15), PBL_IF_ROUND_ELSE(136, 131), bounds.size.w - PBL_IF_ROUND_ELSE(50, 30), 4));
    layer_set_update_proc(s_battery_layer, battery_update_proc);
    layer_add_child(window_layer, s_battery_layer);

    // Setup bluetooth indicator
    s_bt_icon_bitmap = gbitmap_create_with_resource(RESOURCE_ID_BT_ICON);
    s_bt_icon_layer = bitmap_layer_create(GRect(PBL_IF_ROUND_ELSE(15, 10), 100, 20, 20));
    bitmap_layer_set_bitmap(s_bt_icon_layer, s_bt_icon_bitmap);
    layer_add_child(window_layer, bitmap_layer_get_layer(s_bt_icon_layer));
    bluetooth_callback(connection_service_peek_pebble_app_connection());
}

static void main_window_unload(Window *window) {
    // Destroy all the things
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    fonts_unload_custom_font(s_rwby_time_font);
    fonts_unload_custom_font(s_rwby_date_font);
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);
    layer_destroy(s_battery_layer);
    layer_destroy(s_battery_background_layer);
    gbitmap_destroy(s_bt_icon_bitmap);
    bitmap_layer_destroy(s_bt_icon_layer);
}

static void init() {
    // Create main Window
    s_main_window = window_create();
    window_set_background_color(s_main_window, GColorWhite);

    // Set handlers to manage the elements inside the Window
    window_set_window_handlers(s_main_window, (WindowHandlers) {
        .load = main_window_load,
        .unload = main_window_unload
    });

    // Show the Window on the watch, with animated=true
    window_stack_push(s_main_window, true);

    // Make sure the time, date and battery are displayed from the start
    update_time();
    update_date();
    battery_callback(battery_state_service_peek());

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);

    // Register for battery level updates
    battery_state_service_subscribe(battery_callback);

    // Register for Bluetooth connection updates
    connection_service_subscribe((ConnectionHandlers) {
        .pebble_app_connection_handler = bluetooth_callback
    });
}

static void deinit() {
    // Destroy Window
    window_destroy(s_main_window);
}

int main(void) {
    init();
    app_event_loop();
    deinit();
}