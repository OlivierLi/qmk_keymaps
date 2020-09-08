#include "quantum.h"
#include "layer_with_mod_tap.h"

uint16_t last_layer_tap_mod_down_time = 0;
bool layer_tap_mod_in_progress = false;
bool interrupted = false;

struct InteruptingPress pending_keys[PENDING_KEYS_BUFFER_SIZE] = {0};
uint8_t pending_keys_count = 0;
uint8_t current_layer = 0; 

uint16_t GetKeyFromMatrix(uint8_t layer, keyrecord_t *record){
  //const uint8_t col = record->event.key.col;
  //const uint8_t row = record->event.key.row;
  return keymap_key_to_keycode(layer, record->event.key);
}

__attribute__ ((weak))
  uint16_t keymap_key_to_keycode(uint8_t layer, keypos_t key)
{
  // Read entire word (16bits)
  return pgm_read_word(&keymaps[(layer)][(key.row)][(key.col)]);
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

  const uint16_t elapsed_time = record->event.time - last_layer_tap_mod_down_time;
  if (elapsed_time > TAPPING_TERM) {
      flush_pending();
      return false;
  }
  else{
      interrupted = true;
  }

  // If no more place to buffer keycodes. Just drop.
  if(pending_keys_count != PENDING_KEYS_BUFFER_SIZE-1){
    struct InteruptingPress interupting_press = {
      .is_down = is_down, 
      .keycode = keycode,
      .previous_layer_keycode = GetKeyFromMatrix(current_layer, record)};

    pending_keys[pending_keys_count++] = interupting_press;
  }

  // Swallow the key.
  return true;
}

void layer_with_mod_on_hold_key_on_tap(keyrecord_t *record, uint8_t layer, uint8_t hold_mod, uint8_t tap_keycode) {
  // Key down.
  if (record->event.pressed) {
    last_layer_tap_mod_down_time = record->event.time;

    // Activate the layer and modifier first.
    layer_on(layer);
    register_mods(MOD_BIT(hold_mod));

    layer_tap_mod_in_progress = true;
    interrupted = false;
  }
  // Key up.
  else{
    const uint16_t elapsed_time = record->event.time - last_layer_tap_mod_down_time;
    if (elapsed_time < TAPPING_TERM) {
      // Normal tap.
      if(!interrupted){
        // Reset state.
        unregister_mods(MOD_BIT(hold_mod));
        layer_off(layer);

        register_code16(tap_keycode);
        unregister_code16(tap_keycode);

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

        register_code16(tap_keycode);
        unregister_code16(tap_keycode);

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
