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
  const bool is_up = !is_down;

  // Any action on the layer tap mod key should be handled in the handler.
  if(keycode == LAYER_TAP_MOD){
    return false;
  }

  // Outside of layer tap mod just handle normally.
  if(!layer_tap_mod_in_progress){
    return false;
  }

  // Finished tapping a key during the tap mold hold. It cancels it.
  if(is_up){
    flush_pending();
    layer_tap_mod_in_progress = false;
    return false;
  }
  else {
    // If no more place to buffer keycodes. Just drop.
    if(pending_keys_count != PENDING_KEYS_BUFFER_SIZE-1){
      pending_keys[pending_keys_count++] = keycode;
    }

    // Swallow the key.
    return true;
  }
}

// TODO: Support a tap modifier so we can use this with quotes
// TODO: Find out why the quick :a example does not work in vim. What happens right now is that if LT, K, K , LT
// happens all within TAPPING_TERM then we trigger the tapping option. We only want that when LT, LT, K, K happened during
// the tapping term.

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

      // Flush the keys that were buffered outside of the effect of the layer or modifier.
      flush_pending();
    }
    // If holding.
    else {

      // Flush the pending keys under the effect of the modifier and layer.
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
