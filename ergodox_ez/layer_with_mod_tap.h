#ifndef LAYER_WITH_MOD_TAP_H
#define LAYER_WITH_MOD_TAP_H

#include QMK_KEYBOARD_H
#include "enums.h"

// Constants ------------------------------------------------------------------
#define PENDING_KEYS_BUFFER_SIZE 4
// ----------------------------------------------------------------------------

// Variables ------------------------------------------------------------------
static uint16_t last_layer_tap_mod_down_time = 0;
static uint16_t last_layer_tap_mod_up_time = 0;
static bool layer_tap_mod_in_progress = false;

static uint16_t pending_keys[PENDING_KEYS_BUFFER_SIZE] = {KC_NO, KC_NO, KC_NO, KC_NO};
static uint8_t pending_keys_count = 0;
// ----------------------------------------------------------------------------

//TODO: Setup the planck ez as a dev board. 

//TODO: We can start handling thing differently depending on the length of the hold
//TODO: Currently if the hold is held for less than TAPPING_TERM then it will trigger a tap. The alphas can still be buffered meanwhile
//TODO: and then they only make it out on the next flush. Make sure the fix applied corrects that problem.

//TODO: You have to handle the difference between up and down fully. if an up code is swallowed they the down code needs to and vice versa
// the logic needs to be enhanced to take that into account

//TODO: Also it's possible we need to change the logic of whether to tap or not when interrupted or it might conflic with this logic.

void flush_pending(void){
  for(int i=0;i<pending_keys_count;++i){
    tap_code(pending_keys[i]);
  } 

  pending_keys_count = 0;
}

// This function has to be called on process_record_user() to know if a layer
// hold get interrupted. On interruption a tap is not triggered.
// Returns true if the key was absorbed and should not buble up.
bool layer_with_mod_tap_on_key_press(bool is_down, uint16_t keycode){

  bool is_other = (keycode != LAYER_TAP_MOD);
  bool is_up = !is_down;

  //Key was released that is not layer hold tap.
  if(is_up && is_other){
    flush_pending();
    return false;
  }

  if(layer_tap_mod_in_progress && is_other){

    // No more place to buffer keycodes. Just drop.
    if(pending_keys_count == PENDING_KEYS_BUFFER_SIZE-1){
      return true;
    }

    pending_keys[pending_keys_count++] = keycode;
    return true;
  }

  // // // Hold was interrupted, no longer in progress.
  if(is_other){
    layer_tap_mod_in_progress = false;
  }

  return false;
}

// Call this function to get a layer activation on hold with |hold_mod| active and |tap_keycode| on tap.
// 
// TODO: There should be a |layer_tap_mod_in_progress| variable for each key that is bound to this function.
// otherwise the tapping will be reactivated because another call to this function set it to true. This is only
// a real problem if we start cording with keys that use this function.
void layer_with_mod_on_hold_key_on_tap(keyrecord_t *record, uint8_t layer, uint8_t hold_mod, uint8_t tap_keycode) {

  // Key down.
  if (record->event.pressed) {
    last_layer_tap_mod_down_time = timer_read();

    // Activate the layer and modifier first.
    layer_on(layer);
    register_mods(MOD_BIT(hold_mod));
    layer_tap_mod_in_progress = true;
  }
  // Key up.
  else{
    last_layer_tap_mod_up_time = timer_read();

    // If tapping.
    if (timer_elapsed(last_layer_tap_mod_down_time) < TAPPING_TERM) {
      // Reset state before the tap.
      unregister_mods(MOD_BIT(hold_mod));
      layer_off(layer);

      // If the operation was not interrupted.
      if(layer_tap_mod_in_progress)
        tap_code(tap_keycode);

      flush_pending();
    }
    // If holding.
    else {

      flush_pending();

      // Reset state.
      unregister_mods(MOD_BIT(hold_mod));
      layer_off(layer);
    }

    // Key no longer held, no longer in progress.
    layer_tap_mod_in_progress = false;
  }
}

#endif // LAYER_WITH_MOD_TAP_H
