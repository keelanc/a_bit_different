/*
 
 A Bit Different
 
 https://github.com/keelanc/a_bit_different
 
 Based on the Pebble team's Just A Bit
 
 */

#include "pebble_os.h"
#include "pebble_app.h"
#include "pebble_fonts.h"
#include "hobbit_meals.h"


#define MY_UUID { 0x11, 0x28, 0xB6, 0x9D, 0x9F, 0xBA, 0x48, 0x27, 0x8B, 0x18, 0xD2, 0x47, 0x74, 0x56, 0xC7, 0x8C }
PBL_APP_INFO(MY_UUID,
             "A Bit Diff", "keelanchufor.com",
             1, 1, /* App version */
             DEFAULT_MENU_ICON,
             APP_INFO_WATCH_FACE);

Window window;
Layer display_layer;
TextLayer text_month_layer;
TextLayer text_date_layer;
TextLayer text_hobbit_layer;


static char mon_text[] = "XXX";
static char date_text[] = "00";
static char hobbit_hour[] = "something quite long";


#define CIRCLE_RADIUS 10
#define CIRCLE_LINE_THICKNESS 2

void draw_cell(GContext* ctx, GPoint center, bool filled) {
	// Each "cell" represents a binary digit or 0 or 1.
	
	graphics_context_set_fill_color(ctx, GColorWhite);
	
	graphics_fill_circle(ctx, center, CIRCLE_RADIUS);
	
	if (!filled) {
		// This is our ghetto way of drawing circles with a line thickness
		// of more than a single pixel.
		graphics_context_set_fill_color(ctx, GColorBlack);
		
		graphics_fill_circle(ctx, center, CIRCLE_RADIUS - CIRCLE_LINE_THICKNESS);
	}
	
}

#define CELLS_PER_ROW 6
#define CELLS_PER_COLUMN 4

#define CIRCLE_PADDING 12 - CIRCLE_RADIUS // Number of padding pixels on each side
#define CELL_SIZE (2 * (CIRCLE_RADIUS + CIRCLE_PADDING)) // One "cell" is the square that contains the circle.
#define TOP_PADDING (168 - (CELLS_PER_COLUMN * CELL_SIZE))


GPoint get_center_point_from_cell_location(unsigned short x, unsigned short y) {
	// Cell location (0,0) is upper left, location (5,3) is lower right.
	return GPoint((CELL_SIZE/2) + (CELL_SIZE * x),
				  TOP_PADDING + (CELL_SIZE/2) + (CELL_SIZE * y));
}


void draw_cell_column_for_digit(GContext* ctx, unsigned short digit, unsigned short max_rows_to_display, unsigned short cell_column, unsigned short default_max_rows) {
	// Converts the supplied decimal digit into Binary Coded Decimal form and
	// then draws a row of cells on screen--'1' binary values are filled, '0' binary values are not filled.
	// `max_rows_to_display` restricts how many binary digits are shown in the column.
	// `default_max_rows` helps start drawing from the very bottom
	for (int cell_row_index = default_max_rows; cell_row_index >= default_max_rows - max_rows_to_display; cell_row_index--) {
		draw_cell(ctx, get_center_point_from_cell_location(cell_column, cell_row_index), (digit >> (default_max_rows - cell_row_index)) & 0x1);
	}
}


// The cell column offsets for each digit
#define HOURS_FIRST_DIGIT_COL 0
#define HOURS_SECOND_DIGIT_COL 1
#define MINUTES_FIRST_DIGIT_COL 2
#define MINUTES_SECOND_DIGIT_COL 3
#define SECONDS_FIRST_DIGIT_COL 4
#define SECONDS_SECOND_DIGIT_COL 5

// The maximum number of cell columns to display
// (Used so that if a binary digit can never be 1 then no un-filled
// placeholder is shown.)
#define DEFAULT_MAX_ROWS (CELLS_PER_COLUMN - 1)
#define HOURS_FIRST_DIGIT_MAX_ROWS 1
#define MINUTES_FIRST_DIGIT_MAX_ROWS 2
#define SECONDS_FIRST_DIGIT_MAX_ROWS 2


unsigned short get_display_hour(unsigned short hour) {
	
	if (clock_is_24h_style()) {
		return hour;
	}
	
	// convert 24hr to 12hr
	unsigned short display_hour = hour % 12;
	// Converts "0" to "12"
	return display_hour ? display_hour : 12;
	
}


void display_layer_update_callback(Layer *me, GContext* ctx) {
	(void)me;
	
	PblTm t;
	get_time(&t);
	
	unsigned short display_hour = get_display_hour(t.tm_hour);
	
	draw_cell_column_for_digit(ctx, display_hour / 10, HOURS_FIRST_DIGIT_MAX_ROWS, HOURS_FIRST_DIGIT_COL, DEFAULT_MAX_ROWS);
	draw_cell_column_for_digit(ctx, display_hour % 10, DEFAULT_MAX_ROWS, HOURS_SECOND_DIGIT_COL, DEFAULT_MAX_ROWS);
	
	draw_cell_column_for_digit(ctx, t.tm_min / 10, MINUTES_FIRST_DIGIT_MAX_ROWS, MINUTES_FIRST_DIGIT_COL, DEFAULT_MAX_ROWS);
	draw_cell_column_for_digit(ctx, t.tm_min % 10, DEFAULT_MAX_ROWS, MINUTES_SECOND_DIGIT_COL, DEFAULT_MAX_ROWS);
	
	draw_cell_column_for_digit(ctx, t.tm_sec / 10, SECONDS_FIRST_DIGIT_MAX_ROWS, SECONDS_FIRST_DIGIT_COL, DEFAULT_MAX_ROWS);
	draw_cell_column_for_digit(ctx, t.tm_sec % 10, DEFAULT_MAX_ROWS, SECONDS_SECOND_DIGIT_COL, DEFAULT_MAX_ROWS);
	
}


void update_watchface(PblTm* t) {
	
	string_format_time(mon_text, sizeof(mon_text), "%b", t);
	string_format_time(date_text, sizeof(date_text), "%e", t);
	text_layer_set_text(&text_month_layer, mon_text);
	text_layer_set_text(&text_date_layer, date_text);
	
	hobbit_time(t->tm_hour, hobbit_hour);
	text_layer_set_text(&text_hobbit_layer, hobbit_hour);
	
}


void handle_init(AppContextRef ctx) {
	// initializing app
	
	(void)ctx;

	window_init(&window, "A Bit Diff watch");
	window_stack_push(&window, true /* Animated */);
	window_set_background_color(&window, GColorBlack);
	
	// init the bit layer
	layer_init(&display_layer, window.layer.frame);
	display_layer.update_proc = &display_layer_update_callback; // REF: .update_proc points to a function that draws the layer
	layer_add_child(&window.layer, &display_layer);
	
	
	resource_init_current_app(&APP_RESOURCES);

	// init the month text layer
	text_layer_init(&text_month_layer, GRect(0, 0, 144/*width 144*/, TOP_PADDING/* height 168*/)); /* REF: void text_layer_init(TextLayer *text_layer, GRect frame); Erases the contents of a text layer and set the following default values: – Font: Raster Gothic 14-point Boldface – Text Alignment: Left – Text color: black – Background color: white – Clips: True – Caching: False The text layer is automatically marked dirty after this operation This sets up the text layer to use the correct graphics subroutines for filling in the background color and text drawing.*/
	text_layer_set_font(&text_month_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_18)));
	text_layer_set_text_color(&text_month_layer, GColorWhite);
	text_layer_set_background_color(&text_month_layer, GColorClear);
	layer_add_child(&window.layer, &text_month_layer.layer);
	
	// init the date text layer
	text_layer_init(&text_date_layer, GRect(40, 0, 144 - 40, TOP_PADDING));
	text_layer_set_font(&text_date_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_BOLD_18)));
	text_layer_set_text_color(&text_date_layer, GColorWhite);
	text_layer_set_background_color(&text_date_layer, GColorClear);
	layer_add_child(&window.layer, &text_date_layer.layer);
	
	// init the hobbit text layer
	text_layer_init(&text_hobbit_layer, GRect(0, 20, 144, TOP_PADDING - 20));
	text_layer_set_font(&text_hobbit_layer, fonts_load_custom_font(resource_get_handle(RESOURCE_ID_FONT_GOTHAM_LIGHT_18)));
	text_layer_set_text_color(&text_hobbit_layer, GColorWhite);
	text_layer_set_background_color(&text_hobbit_layer, GColorClear);
	layer_add_child(&window.layer, &text_hobbit_layer.layer);
	
	// load watchface immediately
	PblTm t;
	get_time(&t);
	update_watchface(&t);
}


void handle_tick(AppContextRef ctx, PebbleTickEvent *t) {
	// doing something on the second
	
	(void)ctx;
	
	update_watchface(t->tick_time);
	
}


void pbl_main(void *params) {
  PebbleAppHandlers handlers = {
    .init_handler = &handle_init,
	  .tick_info = {
		  .tick_handler = &handle_tick,
		  .tick_units = SECOND_UNIT
	  }
  };
  app_event_loop(params, &handlers);
}
