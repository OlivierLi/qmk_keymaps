#ifndef LAYER_WITH_MOD_TAP_H
#define LAYER_WITH_MOD_TAP_H

#include "enums.h"

// Variables ------------------------------------------------------------------
static uint16_t ls_timer = 0;
static bool layer_with_mod_in_progress = false;
// ----------------------------------------------------------------------------

// This function has to be called on process_record_user() to know if a layer
// hold get interrupted. On interruption a tap is not triggered.
void layer_with_mod_tap_on_key_press(uint16_t keycode){
  if(keycode != LAYER_TAP_MOD)
    // Hold was interrupted, no longer in progress.
    layer_with_mod_in_progress = false;
}

// Call this function to get a layer activation on hold with |hold_mod| active and |tap_keycode| on tap.
// 
// TODO: There should be a |layer_with_mod_in_progress| variable for each key that is bound to this function.
// otherwise the tapping will be reactivated because another call to this function set it to true. This is only
// a real problem if we start cording with keys that use this function.
void layer_with_mod_on_hold_key_on_tap(keyrecord_t *record, uint8_t layer, uint8_t hold_mod, uint8_t tap_keycode) {

  // Key down.
  if (record->event.pressed) {
    ls_timer = timer_read();

    // Activate the layer and modifier first.
    layer_on(layer);
    register_mods(MOD_BIT(hold_mod));
    layer_with_mod_in_progress = true;
  }
  // Key up.
  else{

    // If tapping.
    if (timer_elapsed(ls_timer) < TAPPING_TERM) {
      // Reset state before the tap.
      unregister_mods(MOD_BIT(hold_mod));
      layer_off(layer);

      // If 
      if(layer_with_mod_in_progress)
        tap_code(tap_keycode);
    }
    // If holding.
    else {
      // Reset state.
      unregister_mods(MOD_BIT(hold_mod));
      layer_off(layer);
    }

    // Key no longer held, no longer in progress.
    layer_with_mod_in_progress = false;
  }
}

#endif // LAYER_WITH_MOD_TAP_H
