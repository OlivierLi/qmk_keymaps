#ifndef LAYER_WITH_MOD_TAP_H
#define LAYER_WITH_MOD_TAP_H

#include "enums.h"

// Variables ------------------------------------------------------------------
static uint16_t ls_timer = 0;
static bool layer_with_mod_in_progress = false;
// ----------------------------------------------------------------------------

void layer_with_mod_tap_on_key_press(uint16_t keycode){
  if(keycode != LAYER_TAP_MOD)
    layer_with_mod_in_progress = false;
}

void layer_with_mod_on_hold_key_on_tap(keyrecord_t *record, uint8_t layer, uint8_t holdMod, uint8_t keycode) {

  // Key down.
  if (record->event.pressed) {
    ls_timer = timer_read();

    // Activate the layer and modifier no matter what.
    layer_on(layer);
    register_mods(MOD_BIT(holdMod));
    layer_with_mod_in_progress = true;
  }
  // Key up.
  else{
    // If tapping.
    if (timer_elapsed(ls_timer) < TAPPING_TERM) {
      unregister_mods(MOD_BIT(holdMod));
      layer_off(layer);

      if(layer_with_mod_in_progress)
        tap_code(keycode);
    }
    // If holding.
    else {
      unregister_mods(MOD_BIT(holdMod));
      layer_off(layer);
    }

    layer_with_mod_in_progress = false;
  }
}

#endif // LAYER_WITH_MOD_TAP_H
