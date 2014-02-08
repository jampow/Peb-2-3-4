#include <pebble.h>
#include <time.h>

// pointer declared for Window element
static Window *window;

// used to show any standard string of characters in a predefined area
static TextLayer *text_layer;

char buffer[] = "00:00";
char bpmtext[32] = "80";

AppTimer *timer;


int bpm = 80;

// time in between vibrations, in ms
int delta = 500;

bool vibrating = 0;

// Vibe pattern: ON for 50ms -- seems to be shortest interval that can be felt
//static const uint32_t const segments[] = {50};
uint32_t segments[1] = {50};
VibePattern pat = {
    // uint32_t type
    .durations = segments,
    // size_t type
    .num_segments = ARRAY_LENGTH(segments),
};

void timer_callback(void *data) {
    vibes_enqueue_custom_pattern(pat);
    //vibes_short_pulse();
    
    //Register next execution
    timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
}


static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
  //text_layer_set_text(text_layer, "Metronome stopped");
    
    if (vibrating){
        app_timer_cancel(timer);
        vibrating = 0;
    }
    else{
        delta = (60 * 1000)/ bpm;
        timer = app_timer_register(delta, (AppTimerCallback) timer_callback, NULL);
        vibrating = 1;
    }
}

char* itoa(int val, int base){
    
    static char buf[32] = {0};
    
    int i = 30;
    
    for(; val && i ; --i, val /= base)
        
        buf[i] = "0123456789abcdef"[val % base];
    
    return &buf[i+1];
    
}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  app_timer_cancel(timer);
    
    bpm = bpm + 1;
    char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
    strcpy(bpmChar, itoa(bpm, 10));
    text_layer_set_text(text_layer, bpmChar);
    free(bpmChar);
    
}



static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
    bpm = bpm - 1;
    char * bpmChar = malloc(strlen(itoa(bpm,10))+1);
    strcpy(bpmChar, itoa(bpm, 10));
    text_layer_set_text(text_layer, bpmChar);
    free(bpmChar);
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}


/*static void handle_tick(struct tm *tick_time, TimeUnits units_changed) {
    APP_LOG(APP_LOG_VERBOSE, "Time flies!");
    
}*/

/*static void tick_handler(struct tm *tick_time, TimeUnits units_changed)
{
    //Here we will update the watchface display
    
    //Format the buffer string using tick_time as the time source
    strftime(buffer, sizeof("00:00"), "%H:%M", tick_time);
    
    //Change the TextLayer text to show the new time!
    text_layer_set_text(text_layer, buffer);
}*/


// adds the creation of Window's elements
static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
 // GRect bounds = layer_get_bounds(window_layer);
    
    
   /* text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
    
    
   
    
  text_layer_set_text(text_layer, bpmtext);
    
    
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));*/
    
    
    
    
    text_layer = text_layer_create(GRect(0, 53, 132, 168));
    text_layer_set_background_color(text_layer, GColorClear);
    text_layer_set_text_color(text_layer, GColorBlack);
    text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
    text_layer_set_font(text_layer, fonts_get_system_font(FONT_KEY_BITHAM_42_BOLD));
    
    text_layer_set_text(text_layer, bpmtext);
    
    layer_add_child(window_get_root_layer(window), (Layer*) text_layer);
    
}

// safely destroys the Window's elements
static void window_unload(Window *window) {
    
    app_timer_cancel(timer);
  text_layer_destroy(text_layer);
}

static void init(void) {
    
    
    // initialize app elements here
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
    
    // slides window into view
  window_stack_push(window, animated);
    
    //tick_timer_service_subscribe(MINUTE_UNIT, (TickHandler) tick_handler);
    
    //tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}

static void deinit(void) {
    window_destroy(window);
    //tick_timer_service_unsubscribe();
}

int main(void) {
  init();

  APP_LOG(APP_LOG_LEVEL_DEBUG, "Done initializing, pushed window: %p", window);

    
  app_event_loop();
  deinit();
}
