#include <pebble.h>

#ifndef SHOWDIGITS
#define SHOWDIGITS

// Slot on-screen layout:
//     0 1
#define TOTAL_IMAGE_SLOTS 2
#define NUMBER_OF_IMAGES 10

// These images are 72 x 128 pixels
const int IMAGE_RESOURCE_IDS[NUMBER_OF_IMAGES] = {
  RESOURCE_ID_IMAGE_NUM_0, RESOURCE_ID_IMAGE_NUM_1, RESOURCE_ID_IMAGE_NUM_2,
  RESOURCE_ID_IMAGE_NUM_3, RESOURCE_ID_IMAGE_NUM_4, RESOURCE_ID_IMAGE_NUM_5,
  RESOURCE_ID_IMAGE_NUM_6, RESOURCE_ID_IMAGE_NUM_7, RESOURCE_ID_IMAGE_NUM_8,
  RESOURCE_ID_IMAGE_NUM_9
};

static GBitmap *s_images[TOTAL_IMAGE_SLOTS];
static BitmapLayer *s_image_layers[TOTAL_IMAGE_SLOTS];

#define EMPTY_SLOT -1

// The state is either "empty" or the digit of the image currently in the slot.
static int s_image_slot_state[TOTAL_IMAGE_SLOTS] = {
  EMPTY_SLOT, EMPTY_SLOT
};

// Loads the digit image from the application's resources and
// displays it on-screen in the correct location.
// Each slot is a quarter of the screen.
static void load_digit_image_into_slot(Window *s_main_window, int slot_number, int digit_value) {
  GRect bounds = layer_get_bounds(window_get_root_layer(s_main_window));
  GRect tile_bounds = gbitmap_get_bounds(s_images[slot_number]);

  s_image_slot_state[slot_number] = digit_value;
  s_images[slot_number] = gbitmap_create_with_resource(IMAGE_RESOURCE_IDS[digit_value]);

  const int x_offset = 0;//(bounds.size.w - (2 * tile_bounds.size.w)) / 2;
  const int y_offset = 0;//(bounds.size.h - (2 * tile_bounds.size.h)) / 2;
  BitmapLayer *bitmap_layer = bitmap_layer_create(
    GRect(x_offset + ((slot_number % 2) * tile_bounds.size.w),
      y_offset + ((slot_number / 2) * tile_bounds.size.h),
      tile_bounds.size.w, tile_bounds.size.h));
  s_image_layers[slot_number] = bitmap_layer;
  bitmap_layer_set_bitmap(bitmap_layer, s_images[slot_number]);

  Layer *window_layer = window_get_root_layer(s_main_window);
  layer_add_child(window_layer, bitmap_layer_get_layer(bitmap_layer));
}

static void unload_digit_image_from_slot(int slot_number) {
  if (s_image_slot_state[slot_number] != EMPTY_SLOT) {
    layer_remove_from_parent(bitmap_layer_get_layer(s_image_layers[slot_number]));
    bitmap_layer_destroy(s_image_layers[slot_number]);
    gbitmap_destroy(s_images[slot_number]);

    // This is now an empty slot
    s_image_slot_state[slot_number] = EMPTY_SLOT;
  }
}

static void display_value(Window *s_main_window, unsigned short value, unsigned short row_number, bool show_first_leading_zero) {
  value = value % 100; // Maximum of two digits per row.

  // Column order is: | Column 0 | Column 1 |
  // (We process the columns in reverse order because that makes
  // extracting the digits from the value easier.)
  for (int column_number = 1; column_number >= 0; column_number--) {
    int slot_number = (row_number * 2) + column_number;
    unload_digit_image_from_slot(slot_number);
    if (!((value == 0) && (column_number == 0) && !show_first_leading_zero)) {
      load_digit_image_into_slot(s_main_window, slot_number, value % 10);
    }
    value = value / 10;
  }
}

#endif
