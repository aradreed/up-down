#include <pebble.h>
#include <stdlib.h>

static Window *window;
static TextLayer *text_layer;
static TextLayer *up_label;
static TextLayer *score_label;
static TextLayer *high_score_label;
static TextLayer *down_label;

#define INITIAL_TIME 1000

static struct GameState {
  int score;
  int timeInterval;
  int direction;
  bool isRunning;
  bool buttonPushed;
  AppTimer *timer;
} state;

const int HIGH_SCORE_KEY = 1337;

static void reset_labels() {
	layer_set_hidden((Layer*) up_label, true);
	layer_set_hidden((Layer*) down_label, true);
}

static void set_score() {
	int highScore = persist_read_int(HIGH_SCORE_KEY);

	if (state.score > highScore) {
		// Save persistent data
		persist_write_int(HIGH_SCORE_KEY, state.score);	
		highScore = state.score;
	}
	
	// Display high score
	static char buf[32];
	snprintf(buf, 32, "High Score: %u", highScore);
	text_layer_set_text(high_score_label, buf);
	
	layer_set_hidden((Layer*) high_score_label, false);
}

static void timer_callback() {
	if (state.buttonPushed) {
		state.buttonPushed = false;
	
		// Seed the random number with a time
		srand(time(NULL));
	
		// Get a number between 1 and 2
		state.direction = (rand() % 2) + 1;
	
	
		static char buf[32];
		snprintf(buf, 32, "Time interval: %u", state.timeInterval);
		text_layer_set_text(text_layer, buf);
	
		// Set the correct label based on random number
		if (state.direction == 1) {
			layer_set_hidden((Layer*) up_label, false);
		}
	
		else {
			layer_set_hidden((Layer*) down_label, false);
		}
		
		state.timeInterval -= 10;
		app_timer_register(state.timeInterval, timer_callback, NULL);
	}

	else {
		text_layer_set_text(text_layer, "Game over");
		reset_labels();
		state.direction = 0;
		state.isRunning = false;
		set_score();
	}

}

static void add_score() {
	// Increment and display score
  	state.score++;
  	static char buf[32];
  	snprintf(buf, 32, "Score: %u", state.score);
  	text_layer_set_text(score_label, buf);
  	
  	if ((state.score % 50) == 0) {
  		state.timeInterval = INITIAL_TIME - state.score;
  	}
  	
  	reset_labels();

}

static void select_click_handler(ClickRecognizerRef recognizer, void *context) {
	if (!state.isRunning) {
		state.score = 0;
		
		reset_labels();
		
		layer_set_hidden((Layer*) high_score_label, true);
  		layer_add_child(window_get_root_layer(window), text_layer_get_layer(high_score_label));
  		
		state.timeInterval = 1000;
		state.isRunning = true;
		state.buttonPushed = true;
		state.direction = 0;
    	state.timer = app_timer_register(state.timeInterval, timer_callback, NULL);
    	
    	static char buf[32];
  		snprintf(buf, 32, "Score: %u", state.score);
    	text_layer_set_text(score_label, buf);
  }

}

static void up_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (state.direction == 1) {
  	state.buttonPushed = true;
  	add_score();
  }
  
  else {
  	state.isRunning = false;
  	state.direction = 0;
  }
}

static void down_click_handler(ClickRecognizerRef recognizer, void *context) {
  if (state.direction == 2) {
  	add_score();
  	state.buttonPushed = true;
  }
  
  else {
  	state.isRunning = false;
  	state.direction = 0;
  }
}

static void click_config_provider(void *context) {
  window_single_click_subscribe(BUTTON_ID_SELECT, select_click_handler);
  window_single_click_subscribe(BUTTON_ID_UP, up_click_handler);
  window_single_click_subscribe(BUTTON_ID_DOWN, down_click_handler);
}

static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);
  
  // Set defaults
  state.timer = NULL;

  text_layer = text_layer_create((GRect) { .origin = { 0, 72 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(text_layer, "Press select to start");
  text_layer_set_text_alignment(text_layer, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(text_layer));
  
  up_label = text_layer_create((GRect) { .origin = { 0, 25 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(up_label, "Up");
  text_layer_set_text_alignment(up_label, GTextAlignmentCenter);
  layer_set_hidden((Layer*) up_label, true);
  layer_add_child(window_layer, text_layer_get_layer(up_label));
  
  down_label = text_layer_create((GRect) { .origin = { 0, 125 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text(down_label, "Down");
  text_layer_set_text_alignment(down_label, GTextAlignmentCenter);
  layer_set_hidden((Layer*) down_label, true);
  layer_add_child(window_layer, text_layer_get_layer(down_label));
  
  score_label = text_layer_create((GRect) { .origin = { 0, 5 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(score_label, GTextAlignmentCenter);
  layer_add_child(window_layer, text_layer_get_layer(score_label));
  
  high_score_label = text_layer_create((GRect) { .origin = { 0, 45 }, .size = { bounds.size.w, 20 } });
  text_layer_set_text_alignment(high_score_label, GTextAlignmentCenter);

}

static void window_unload(Window *window) {
  text_layer_destroy(text_layer);
  text_layer_destroy(score_label);
  text_layer_destroy(up_label);
  text_layer_destroy(down_label);
}

static void init(void) {
  window = window_create();
  window_set_click_config_provider(window, click_config_provider);
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  
  const bool animated = true;
  window_stack_push(window, animated);
}

static void deinit(void) {
  window_destroy(window);
}

int main(void) {
  init();
  app_event_loop();
  deinit();
}