#include <pebble.h>
#include <time.h>

// pointers to window layer and other UI layers
static Window *window;
static TextLayer *text_layer;
static TextLayer *units_layer;
static ActionBarLayer *abl_action;

// icons for action bar layer
static GBitmap *gb_arrow_up;
static GBitmap *gb_arrow_down;
static GBitmap *gb_play;
static GBitmap *gb_pause;

// bpm defaulted to 120 when app opens
int bpm = 60;
float index = 0.73;
static int min_bpm = 40;
static int max_bpm = 120;
char bpmtext[] = "60";
char bpmlabel[] = "m/min";

// timers
AppTimer *betweenbeats_timer;
AppTimer *longupclick_timer;
AppTimer *longdownclick_timer;

// time in between vibrations, in ms
int delta = 500;

//
bool is_vibrating = 0;

// Vibe pattern: ON for 45ms -- seems to be shortest interval that can be felt and is relevant at each tempo between 40 and 240 bpm
static const uint32_t const segments[] = {45};
VibePattern pat = {
    // uint32_t type
    .durations = segments,
    // size_t type
    .num_segments = ARRAY_LENGTH(segments),
};


void betweenbeats_timer_callback(void *data) {
    //
    vibes_enqueue_custom_pattern(pat);
    
    // Register next execution
    betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
}



char* itoa(int val, int base){
    static char buf[32] = {0};
    int i = 30;
    for(; val && i ; --i, val /= base)
        buf[i] = "0123456789abcdef"[val % base];
    return &buf[i+1];
}

void longupclick_timer_callback(void *data) {
    
    if (bpm < max_bpm){
        bpm++;
        char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
        strcpy(bpmChar, itoa(bpm, 10));
        text_layer_set_text(text_layer, bpmChar);
        free(bpmChar);
        
        //Register next execution
        longupclick_timer = app_timer_register(50, (AppTimerCallback) longupclick_timer_callback, NULL);
    }
    
}

void longdownclick_timer_callback(void *data) {
    
    if (bpm > min_bpm){
        bpm--;
        char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
        strcpy(bpmChar, itoa(bpm, 10));
        text_layer_set_text(text_layer, bpmChar);
        free(bpmChar);
        
        //Register next execution
        longdownclick_timer = app_timer_register(50, (AppTimerCallback) longdownclick_timer_callback, NULL);
    }
    
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
    
    // if already is_vibrating, cancel vibration and change ActionBarLayer icon from play icon to pause icon
    if (is_vibrating){
        app_timer_cancel(betweenbeats_timer);
        is_vibrating = 0;

        action_bar_layer_set_icon(abl_action, BUTTON_ID_SELECT, gb_play);
    }
    else{
        // sets time between beat vibrations according to bpm set
        delta = (60000) / (bpm / index);
        betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
        is_vibrating = 1;
        
        action_bar_layer_set_icon(abl_action, BUTTON_ID_SELECT, gb_pause);
    }
}



static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
    
    if (bpm < max_bpm){
        bpm++;
        char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
        strcpy(bpmChar, itoa(bpm, 10));
        text_layer_set_text(text_layer, bpmChar);
        free(bpmChar);
        
        if (is_vibrating){
            app_timer_cancel(betweenbeats_timer);
            delta = (60000) / (bpm / index);
            betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
        }
    }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    
    if (bpm > min_bpm){
        bpm--;
        char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
        strcpy(bpmChar, itoa(bpm, 10));
        text_layer_set_text(text_layer, bpmChar);
        free(bpmChar);
        
        if (is_vibrating){
            app_timer_cancel(betweenbeats_timer);
            delta = (60000) / (bpm / index);
            betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
        }
    }

}

void up_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    
    if (is_vibrating){
        app_timer_cancel(betweenbeats_timer);
    }
    
    longupclick_timer = app_timer_register(50, (AppTimerCallback) longupclick_timer_callback, NULL);
}
void up_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
   
    app_timer_cancel(longupclick_timer);
    if (is_vibrating){
        delta = (60000) / (bpm / index);
        betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
    }
}

void down_long_click_handler(ClickRecognizerRef recognizer, void *context) {
    
    if (is_vibrating){
        app_timer_cancel(betweenbeats_timer);
    }
    
    longdownclick_timer = app_timer_register(50, (AppTimerCallback) longdownclick_timer_callback, NULL);
}

void down_long_click_release_handler(ClickRecognizerRef recognizer, void *context) {
    
    app_timer_cancel(longdownclick_timer);
    if (is_vibrating){
        delta = (60000) / (bpm / index);
        betweenbeats_timer = app_timer_register(delta, (AppTimerCallback) betweenbeats_timer_callback, NULL);
    }
}

static void abl_click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
    
    window_long_click_subscribe(BUTTON_ID_UP, 300, up_long_click_handler, up_long_click_release_handler);
    window_long_click_subscribe(BUTTON_ID_DOWN, 300, down_long_click_handler, down_long_click_release_handler);
}

// adds the creation of Window's elements
static void window_load(Window *window) {
    
    Layer *window_layer = window_get_root_layer(window);
    
    abl_action = action_bar_layer_create();
    action_bar_layer_add_to_window(abl_action, window);
    action_bar_layer_set_click_config_provider(abl_action, abl_click_config_provider);
    
    // create bitmap icons for action bar
    gb_arrow_up = gbitmap_create_with_resource(RESOURCE_ID_ARROW_UP);
    gb_arrow_down = gbitmap_create_with_resource(RESOURCE_ID_ARROW_DOWN);
    gb_play = gbitmap_create_with_resource(RESOURCE_ID_PLAY);
    gb_pause = gbitmap_create_with_resource(RESOURCE_ID_PAUSE);
    
    // set
    action_bar_layer_set_icon(abl_action, BUTTON_ID_UP, gb_arrow_up);
    action_bar_layer_set_icon(abl_action, BUTTON_ID_DOWN, gb_arrow_down);
    action_bar_layer_set_icon(abl_action, BUTTON_ID_SELECT, gb_play);
    
    GRect bounds = layer_get_bounds(window_layer);
    text_layer = text_layer_create((GRect) { .origin = { 0, 48 }, .size = { bounds.size.w, 168 } });
    
    //text_layer = text_layer_create(GRect(0, 45, 132, 168));
    text_layer_set_background_color(text_layer, GColorClear);
    text_layer_set_text_color(text_layer, GColorBlack);
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    text_layer_set_text(text_layer, bpmtext);
    
    layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
    
    
    //units_layer = text_layer_create(GRect(0, 100, 132, 30));
    units_layer = text_layer_create((GRect) { .origin = { 0, 92 }, .size = { bounds.size.w, 35 } });
    text_layer_set_background_color(units_layer, GColorClear);
    text_layer_set_text_color(units_layer, GColorBlack);
    text_layer_set_text_alignment(units_layer, GTextAlignmentCenter);
    text_layer_set_font(units_layer, fonts_get_system_font(FONT_KEY_GOTHIC_28));
    
    text_layer_set_text(units_layer, bpmlabel);
    layer_add_child(window_get_root_layer(window), (Layer*) units_layer);
}

// safely destroys the Window's elements
static void window_unload(Window *window) {
    
    app_timer_cancel(betweenbeats_timer);
    
    text_layer_destroy(text_layer);
    action_bar_layer_destroy(abl_action);
    gbitmap_destroy(gb_arrow_up);
    gbitmap_destroy(gb_arrow_down);
    gbitmap_destroy(gb_play);
    gbitmap_destroy(gb_pause);
}


static void init(void) {
    
    
    // initialize app elements here
      window = window_create();
      window_set_click_config_provider(window, abl_click_config_provider);
      window_set_window_handlers(window, (WindowHandlers) {
        .load = window_load,
        .unload = window_unload,
      });
      const bool animated = true;
    
    // slides window into view
    window_stack_push(window, animated);
    
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
