// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H

// Define combo events
enum combo_events {
    JK_ESC,
    COMBO_LENGTH
};

uint16_t COMBO_LEN = COMBO_LENGTH;

// Declare the combo key sequence (must be terminated by COMBO_END)
const uint16_t PROGMEM jk_combo[] = {KC_J, KC_K, COMBO_END};

// Map the sequence to the output key
combo_t key_combos[] = {
    [JK_ESC] = COMBO(jk_combo, KC_ESC),
};

enum sofle_layers {
    _DEFAULT_LAYER,
    _F1_LAYER,
    _F2_LAYER,
    _F3_LAYER,
    _UNUSED_LAYER,
};

enum custom_keycodes {
    KC_PRVWD = QK_USER,
    KC_NXTWD,
    KC_LSTRT,
    KC_LEND
};

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_DEFAULT_LAYER] = LAYOUT(
           KC_GRV,    KC_1,    KC_2,    KC_3,          KC_4,            KC_5,                             KC_6,           KC_7,    KC_8,    KC_9,    KC_0,  KC_MINUS,
           KC_TAB,    KC_Q,    KC_W,    KC_E,          KC_R,            KC_T,                             KC_Y,           KC_U,    KC_I,    KC_O,    KC_P,  KC_BSLS,
          KC_LCTL,    KC_A,    KC_S,    KC_D,          KC_F,            KC_G,                             KC_H,           KC_J,    KC_K,    KC_L, KC_SCLN,  LCTL_T(KC_QUOT),
  LSFT_T(MS_BTN2),    KC_Z,    KC_X,    KC_C,          KC_V,            KC_B, KC_MUTE,        XXXXXXX,    KC_N,           KC_M, KC_COMM,  KC_DOT, KC_SLSH,  RSFT_T(KC_ENT),
                            _______, _______, MO(_F2_LAYER), LALT_T(MS_BTN3), MS_BTN1,        KC_BSPC,  KC_SPC,  MO(_F1_LAYER), _______, _______
 ),
[_F1_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, KC_LBRC, KC_RBRC,  KC_EQL,
  _______, _______, _______, KC_HOME, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______,  KC_END,                       KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, _______, _______,
  _______, _______,  KC_DEL, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
 ),
[_F2_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
 ),
[_F3_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
 ),
[_UNUSED_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
   )
};

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case KC_PRVWD:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    register_mods(mod_config(MOD_LALT));
                    register_code(KC_LEFT);
                } else {
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_LEFT);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LALT));
                    unregister_code(KC_LEFT);
                } else {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_LEFT);
                }
            }
            break;
        case KC_NXTWD:
             if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    register_mods(mod_config(MOD_LALT));
                    register_code(KC_RIGHT);
                } else {
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_RIGHT);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LALT));
                    unregister_code(KC_RIGHT);
                } else {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_RIGHT);
                }
            }
            break;
        case KC_LSTRT:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                     //CMD-arrow on Mac, but we have CTL and GUI swapped
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_LEFT);
                } else {
                    register_code(KC_HOME);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_LEFT);
                } else {
                    unregister_code(KC_HOME);
                }
            }
            break;
        case KC_LEND:
            if (record->event.pressed) {
                if (keymap_config.swap_lctl_lgui) {
                    //CMD-arrow on Mac, but we have CTL and GUI swapped
                    register_mods(mod_config(MOD_LCTL));
                    register_code(KC_RIGHT);
                } else {
                    register_code(KC_END);
                }
            } else {
                if (keymap_config.swap_lctl_lgui) {
                    unregister_mods(mod_config(MOD_LCTL));
                    unregister_code(KC_RIGHT);
                } else {
                    unregister_code(KC_END);
                }
            }
            break;
    }
    return true;
}
