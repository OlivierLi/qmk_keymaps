#ifndef LAYER_WITH_MOD_TAP_H
#define LAYER_WITH_MOD_TAP_H

//#include "enums.h"

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
static uint8_t current_layer = 0; 
// ----------------------------------------------------------------------------

// translates key to keycode
__attribute__ ((weak))
  uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key)
{
  // Read entire word (16bits)
  return pgm_read_word(&keymaps[(layer)][(key.row)][(key.col)]);
}

//TODO: This works, it just can't grab KC_TRANSPARENT! We can grab the keycode
//TODO: That is sent from the user function and also use keymaps in conjunction!
//TODO: Whenever get grab the key we can run down the layers and grab the key for
//TODO: The highest layer that is activated that is not related to an active layer tap mod
//TODO: That is what we use to save the keycode.

uint16_t GetKeyFromMatrix(uint8_t layer, keyrecord_t *record){
  //const uint8_t col = record->event.key.col;
  //const uint8_t row = record->event.key.row;
  return keymap_key_to_keycode(layer, record->event.key);
}

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

void layer_with_mod_tap_on_layer_change(uint8_t layer){
  current_layer = layer;
}

// This function has to be called on process_record_user() to know if a layer
// hold got interrupted.  Returns true if the press was absorbed and should not buble up.
bool layer_with_mod_tap_on_key_press(uint16_t keycode, keyrecord_t *record){
  const bool is_down = record->event.pressed;

  // Any action on the layer tap mod key should be handled in the layer_with_mod_on_hold_key_on_tap().
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

  // If no more place to buffer keycodes. Just drop.
  if(pending_keys_count != PENDING_KEYS_BUFFER_SIZE-1){
		// TODO: Take the time of |record| here not Now();
    if (timer_elapsed(last_layer_tap_mod_down_time) < TAPPING_TERM) {
      interrupted = true;
    }

    struct InteruptingPress interupting_press = {.is_down = is_down, .keycode = GetKeyFromMatrix(current_layer, record)};
    pending_keys[pending_keys_count++] = interupting_press;
  }

  // Swallow the key.
  return true;
}

// Call this function to get a layer activation on hold with |hold_mod| active and |tap_keycode| on tap.
// 
// TODO: There should be a |layer_tap_mod_in_progress| variable for each key that is bound to this function.
// otherwise the tapping will be reactivated because another call to this function set it to true. This is only
// a real problem if we start cording with keys that use this function.
//
// TODO: Support a tap modifier so we can use this with quotes

//TODO: Things work now but when replaying the keys we need to translate from layer to layer
//TODO: For example if a press of " detected within the tapping term but we end up going with
//TODO: the tap and replay then we need to translate that to the equivalent key in the previous
//TODO: layer. In this case: '
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
      // Normal tap.
      if(!interrupted){
        // Reset state.
        unregister_mods(MOD_BIT(hold_mod));
        layer_off(layer);

        tap_code(tap_keycode);

        // Key no longer held, no longer in progress.
        layer_tap_mod_in_progress = false;
        return;
      }

      // A full key press happened within the tapping term.
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
