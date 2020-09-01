#ifndef ENUMS_H
#define ENUMS_H

#ifndef TAPPING_TERM
#define TAPPING_TERM 200
#endif

enum planck_keycodes {
  RGB_SLD = EZ_SAFE_RANGE,
  CSA_LSPO,
  CSA_RSPC,
  LAYER_TAP_MOD,
};

enum planck_layers {
  _BASE,
  _LOWER,
  _RAISE,
  _ADJUST,
  _LAYER4,
  _LAYER5,
  _LAYER6,
  _LAYER7,
};

#endif // ENUMS_H
