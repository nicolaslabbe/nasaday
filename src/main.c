#include <pebble.h>

Window *g_window;
TextLayer *g_text_layer;
char img_buffer[128];
enum {
  KEY_IMG = 0,
};

void process_tuple(Tuple *t)
{
  //Get key
  int key = t->key;
 
  //Get string value, if present
  char string_value[128];
  strcpy(string_value, t->value->cstring);
 
  //Decide what to do
  switch(key) {
    case KEY_IMG:
      printf("string_value = %s", string_value);
			snprintf(img_buffer, sizeof(string_value), "%s", string_value);
      text_layer_set_text(g_text_layer, (char*) &img_buffer); // (char*) &img_buffer
      break;
  }
}

static void in_received_handler(DictionaryIterator *iter, void *context)
{
  (void) context;
     
  //Get data
  Tuple *t = dict_read_first(iter);
  while(t != NULL)
  {
    process_tuple(t);
    //Get next
    t = dict_read_next(iter);
  }
}

static TextLayer* init_text_layer(GRect location, GColor colour, GColor background, const char *res_id, GTextAlignment alignment)
{
  TextLayer *layer = text_layer_create(location);
  text_layer_set_text_color(layer, colour);
  text_layer_set_background_color(layer, background);
  text_layer_set_font(layer, fonts_get_system_font(res_id));
  text_layer_set_text_alignment(layer, alignment);
 
  return layer;
}

void window_load(Window *window)
{
	g_text_layer = init_text_layer(GRect(5, 0, 144, 60), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(g_text_layer, "Url");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(g_text_layer));
}

void window_unload(Window *window)
{
  //We will safely destroy the Window's elements here!
  text_layer_destroy(g_text_layer);
}

void init()
{
  //Create the app elements here!
  g_window = window_create();
  window_set_window_handlers(g_window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  //Register AppMessage events
  app_message_register_inbox_received(in_received_handler);
  //Largest possible input and output buffer sizes
  app_message_open(app_message_inbox_size_maximum(), app_message_outbox_size_maximum());

  window_stack_push(g_window, true);
}

void deinit()
{
  //Destroy app elements here
  window_destroy(g_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}