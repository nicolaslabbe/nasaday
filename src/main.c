#include <pebble.h>
#include "netdownload.h"
#ifdef PBL_PLATFORM_APLITE
#include "png.h"
#endif
Window *g_window;
TextLayer *g_text_layer;
char img_buffer[128];
static BitmapLayer *bitmap_layer;
static GBitmap *current_bmp;

enum {
  KEY_IMG = 0,
};

void show_next_image(char *url) {
  // show that we are loading by showing no image
  bitmap_layer_set_bitmap(bitmap_layer, NULL);

  text_layer_set_text(g_text_layer, "Loading...");

  // Unload the current image if we had one and save a pointer to this one
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
    current_bmp = NULL;
  }
  printf("load url");
  netdownload_request(url);
}

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
      if (string_value != '\0') {
        printf("string_value = %s %d", string_value, key);
  			//snprintf(img_buffer, sizeof(string_value), "%s", string_value);
        //text_layer_set_text(g_text_layer, (char*) &img_buffer); // (char*) &img_buffer
        show_next_image("http://assets.getpebble.com.s3-website-us-east-1.amazonaws.com/pebble-faces/cherie.png");
        // show_next_image(string_value);
      }
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

static void window_load(Window *window)
{
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
	g_text_layer = init_text_layer(GRect(5, 0, 144, 60), GColorBlack, GColorClear, "RESOURCE_ID_GOTHIC_18", GTextAlignmentLeft);
	text_layer_set_text(g_text_layer, "Url");
	layer_add_child(window_get_root_layer(window), text_layer_get_layer(g_text_layer));
  
  bitmap_layer = bitmap_layer_create(bounds);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
  current_bmp = NULL;
}

void window_unload(Window *window)
{
  //We will safely destroy the Window's elements here!
  text_layer_destroy(g_text_layer);
  bitmap_layer_destroy(bitmap_layer);
  gbitmap_destroy(current_bmp);
}

void download_complete_handler(NetDownload *download) {
  printf("Loaded image with %lu bytes", download->length);
  printf("Heap free is %u bytes", heap_bytes_free());

  #ifdef PBL_PLATFORM_APLITE
  GBitmap *bmp = gbitmap_create_with_png_data(download->data, download->length);
  #else
    GBitmap *bmp = gbitmap_create_from_png_data(download->data, download->length);
  #endif
  bitmap_layer_set_bitmap(bitmap_layer, bmp);

  // Save pointer to currently shown bitmap (to free it)
  if (current_bmp) {
    gbitmap_destroy(current_bmp);
  }
  current_bmp = bmp;

  // Free the memory now
  #ifdef PBL_PLATFORM_APLITE
  // gbitmap_create_with_png_data will free download->data
  #else
    free(download->data);
  #endif
  // We null it out now to avoid a double free
  download->data = NULL;
  netdownload_destroy(download);
}

void init()
{
  // Need to initialize this first to make sure it is there when
  // the window_load function is called by window_stack_push.
  netdownload_initialize(download_complete_handler);
  
  g_window = window_create();
  #ifdef PBL_SDK_2
    window_set_fullscreen(g_window, true);
  #endif
  
  //Create the app elements here!
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
  netdownload_deinitialize(); // call this to avoid 20B memory leak
  window_destroy(g_window);
}

int main(void)
{
  init();
  app_event_loop();
  deinit();
}