#ifndef LAYER_WITH_MOD_TAP_H
#define LAYER_WITH_MOD_TAP_H

#include QMK_KEYBOARD_H
#include "enums.h"

// Constants ------------------------------------------------------------------
#define PENDING_KEYS_BUFFER_SIZE 8
// ----------------------------------------------------------------------------

// Structs --------------------------------------------------------------------
struct InteruptingPress {
  bool is_down;
  uint16_t keycode;
};
// ----------------------------------------------------------------------------

// Variables ------------------------------------------------------------------
static uint16_t last_layer_tap_mod_down_time = 0;
static bool layer_tap_mod_in_progress = false;
static bool interrupted = false;

static struct InteruptingPress pending_keys[PENDING_KEYS_BUFFER_SIZE] = {0};
static uint8_t pending_keys_count = 0;
// ----------------------------------------------------------------------------


bool complete_press_buffered(void){
  for(int i=0;i<pending_keys_count;++i){
    if(!pending_keys[i].is_down){
      return true;
    }
  }
  //for(int i=0;i<pending_keys_count;++i){
    //if(pending_keys[i].is_down){
      //for(int j=i;j<pending_keys_count;++j){
        //if(!pending_keys[j].is_down && (pending_keys[j].keycode == pending_keys[i].keycode)){
          //return true;
        //}
      //}
    //} 
  //}
  return false;
}

void flush_pending(void){
  for(int i=0;i<pending_keys_count;++i){
    if(pending_keys[i].is_down)
     register_code16(pending_keys[i].keycode);
    else
     unregister_code16(pending_keys[i].keycode);
  } 
  pending_keys_count = 0;
}

// This function has to be called on process_record_user() to know if a layer
// hold get interrupted. On interruption a tap is not triggered.
// Returns true if the key was absorbed and should not buble up.
bool layer_with_mod_tap_on_key_press(bool is_down, uint16_t keycode){
  // Any action on the layer tap mod key should be handled in the handler.
  if(keycode == LAYER_TAP_MOD){
    return false;
  }

  // Outside of layer tap mod just handle normally.
  if(!layer_tap_mod_in_progress){
    return false;
  }

  if (timer_elapsed(last_layer_tap_mod_down_time) > TAPPING_TERM) {
      interrupted = false;
      flush_pending();
  }

  // if a key is tapped up here we can go back on the pending_keys and replace the associated down with KC_NO
  // then we can tap the key?

  // If no more place to buffer keycodes. Just drop.
  if(pending_keys_count != PENDING_KEYS_BUFFER_SIZE-1){
    if (timer_elapsed(last_layer_tap_mod_down_time) < TAPPING_TERM) {
      interrupted = true;
    }

    struct InteruptingPress interupting_press = {.is_down = is_down, .keycode = keycode};
    pending_keys[pending_keys_count++] = interupting_press;
  }

  // Swallow the key.
  return true;
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
    interrupted = false;
  }
  // Key up.
  else{
    if (timer_elapsed(last_layer_tap_mod_down_time) < TAPPING_TERM) {
      if(!interrupted){
        // Reset state.
        unregister_mods(MOD_BIT(hold_mod));
        layer_off(layer);
        tap_code(tap_keycode);

        // Key no longer held, no longer in progress.
        layer_tap_mod_in_progress = false;
        return;
      }

      if(complete_press_buffered()){
        flush_pending();

        // Reset state.
        unregister_mods(MOD_BIT(hold_mod));
        layer_off(layer);
      }
      else {
        // Reset state.
        unregister_mods(MOD_BIT(hold_mod));
        layer_off(layer);

        tap_code(tap_keycode);
        flush_pending();
      }
    }
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
