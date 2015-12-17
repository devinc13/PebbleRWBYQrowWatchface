#include <pebble.h>

static Window *s_main_window;
static TextLayer *s_time_layer;
static TextLayer *s_date_layer;

static BitmapLayer *s_background_layer;
static GBitmap *s_background_bitmap;

static GFont s_rwby_time_font;
static GFont s_rwby_date_font;

static void update_time() {
    time_t temp = time(NULL); 
    struct tm *tick_time = localtime(&temp);

    // Display time
    static char s_time_buffer[8];
    strftime(s_time_buffer, sizeof(s_time_buffer), clock_is_24h_style() ? "%H:%M" : "%I:%M", tick_time);
    text_layer_set_text(s_time_layer, s_time_buffer);

    // Display date
    static char s_date_buffer[12];
    strftime(s_date_buffer, sizeof(s_date_buffer), "%a, %b %e", tick_time);
    text_layer_set_text(s_date_layer, s_date_buffer);
}

static void tick_handler(struct tm *tick_time, TimeUnits units_changed) {
    update_time();
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
    s_time_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(10, 5), bounds.size.w, 50));
    text_layer_set_font(s_time_layer, s_rwby_time_font);
    text_layer_set_background_color(s_time_layer, GColorClear);
    text_layer_set_text_color(s_time_layer, GColorBlack);
    text_layer_set_text_alignment(s_time_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_time_layer));

    // Show Date
    s_date_layer = text_layer_create(GRect(0, PBL_IF_ROUND_ELSE(130, 135), bounds.size.w, 50));
    text_layer_set_font(s_date_layer, s_rwby_date_font);
    text_layer_set_background_color(s_date_layer, GColorClear);
    text_layer_set_text_color(s_date_layer, GColorBlack);
    text_layer_set_text_alignment(s_date_layer, GTextAlignmentCenter);
    layer_add_child(window_layer, text_layer_get_layer(s_date_layer));
}

static void main_window_unload(Window *window) {
    // Destroy all the things
    text_layer_destroy(s_time_layer);
    text_layer_destroy(s_date_layer);
    fonts_unload_custom_font(s_rwby_time_font);
    fonts_unload_custom_font(s_rwby_date_font);
    gbitmap_destroy(s_background_bitmap);
    bitmap_layer_destroy(s_background_layer);
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

    // Make sure the time is displayed from the start
    update_time();

    // Register with TickTimerService
    tick_timer_service_subscribe(MINUTE_UNIT, tick_handler);
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