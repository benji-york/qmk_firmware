// Copyright 2023 QMK
// SPDX-License-Identifier: GPL-2.0-or-later
#include QMK_KEYBOARD_H
#include "raw_hid.h"

#ifdef SPLIT_KEYBOARD
#    include "transactions.h"
#endif

typedef enum {
    HOST_UNKNOWN = 0,
    HOST_PERSONAL = 1,
    HOST_WORK = 2,
} host_t;

#define HOST_CLAIM_TIMEOUT_MS 2500
#define HOST_SYNC_RETRY_MS 100
#define HOST_MAGIC 0x42
#define HOST_MSG_CLAIM 0x01
#define HOST_ID_PERSONAL 0x01

static host_t current_host = HOST_UNKNOWN;
static uint32_t host_claim_started = 0;
static bool host_claim_pending = false;

#ifdef SPLIT_KEYBOARD
static bool host_sync_dirty = true;
static uint32_t host_sync_last = 0;
#endif

static bool host_state_authority(void) {
#ifdef SPLIT_KEYBOARD
    return is_keyboard_master();
#else
    return true;
#endif
}

#ifdef SPLIT_KEYBOARD
static void mark_host_sync_dirty(void) {
    if (!host_state_authority()) {
        return;
    }

    host_sync_dirty = true;
    host_sync_last = timer_read32() - HOST_SYNC_RETRY_MS;
}
#endif

static bool is_valid_host(host_t host) {
    return host == HOST_UNKNOWN || host == HOST_PERSONAL || host == HOST_WORK;
}

static void set_current_host(host_t host) {
    if (!is_valid_host(host)) {
        return;
    }

    current_host = host;
#ifdef SPLIT_KEYBOARD
    mark_host_sync_dirty();
#endif
}

static void begin_host_claim_window(void) {
    set_current_host(HOST_UNKNOWN);

    if (!host_state_authority()) {
        host_claim_pending = false;
        return;
    }

    host_claim_started = timer_read32();
    host_claim_pending = true;
}

#ifdef SPLIT_KEYBOARD
static void host_sync_slave_handler(uint8_t in_buflen, const void *in_data, uint8_t out_buflen, void *out_data) {
    if (in_buflen < 1 || !in_data) {
        return;
    }

    uint8_t host = *(const uint8_t *)in_data;
    host_t synced_host = (host_t)host;
    if (is_valid_host(synced_host)) {
        current_host = synced_host;
    }
}

static void sync_host_to_slave(void) {
    static host_t last_synced_host = HOST_UNKNOWN;

    if (!host_state_authority()) {
        return;
    }

    if (!host_sync_dirty && last_synced_host == current_host) {
        return;
    }

    if (timer_elapsed32(host_sync_last) < HOST_SYNC_RETRY_MS) {
        return;
    }

    uint8_t host = (uint8_t)current_host;
    host_sync_last = timer_read32();

    if (transaction_rpc_send(HOST_SYNC, sizeof(host), &host)) {
        last_synced_host = current_host;
        host_sync_dirty = false;
    }
}
#endif

void keyboard_post_init_user(void) {
#ifdef SPLIT_KEYBOARD
    transaction_register_rpc(HOST_SYNC, host_sync_slave_handler);
#endif
    begin_host_claim_window();
}

void suspend_wakeup_init_user(void) {
    begin_host_claim_window();
}

void housekeeping_task_user(void) {
    if (host_state_authority() && host_claim_pending && timer_elapsed32(host_claim_started) > HOST_CLAIM_TIMEOUT_MS) {
        set_current_host(HOST_WORK);
        host_claim_pending = false;
    }

#ifdef SPLIT_KEYBOARD
    sync_host_to_slave();
#endif
}

void raw_hid_receive(uint8_t *data, uint8_t length) {
    if (length >= 3 && data[0] == HOST_MAGIC && data[1] == HOST_MSG_CLAIM && data[2] == HOST_ID_PERSONAL) {
        set_current_host(HOST_PERSONAL);
        host_claim_pending = false;
    }
}

#ifdef OLED_ENABLE
oled_rotation_t oled_init_user(oled_rotation_t rotation) {
    return OLED_ROTATION_270;
}

static void oled_write_host_label(void) {
    switch (current_host) {
        case HOST_PERSONAL:
            oled_write_ln_P(PSTR("<---"), false);
            break;
        case HOST_WORK:
            oled_write_ln_P(PSTR("--->"), false);
            break;
        default:
            break;
    }
}

bool oled_task_user(void) {
    if (is_keyboard_master()) {
        return true;
    }

    oled_clear();
    oled_set_cursor(0, 0);
    oled_write_host_label();
    return false;
}
#endif

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
    KC_LEND,
    DH_CONFIG,
    DH_JITTER,
    DH_SWITCH
};

#define DESKHOP_HOTKEY_TAP_MS 20

const uint16_t PROGMEM keymaps[][MATRIX_ROWS][MATRIX_COLS] = {
[_DEFAULT_LAYER] = LAYOUT(
           KC_GRV,    KC_1,    KC_2,    KC_3,          KC_4,            KC_5,                             KC_6,           KC_7,    KC_8,    KC_9,    KC_0,  KC_MINUS,
           KC_TAB,    KC_Q,    KC_W,    KC_E,          KC_R,            KC_T,                             KC_Y,           KC_U,    KC_I,    KC_O,    KC_P,  KC_BSLS,
          KC_LCMD,    KC_A,    KC_S,    KC_D,          KC_F,            KC_G,                             KC_H,           KC_J,    KC_K,    KC_L, KC_SCLN,  LCMD_T(KC_QUOT),
  LSFT_T(MS_BTN2),    KC_Z,    KC_X,    KC_C,          KC_V,            KC_B, KC_MUTE,        XXXXXXX,    KC_N,           KC_M, KC_COMM,  KC_DOT, KC_SLSH,  RSFT_T(KC_ENT),
                            _______, _______,       TL_UPPR, LALT_T(MS_BTN3), MS_BTN1,        KC_BSPC,  KC_SPC,        TL_LOWR, _______, _______
 ),
[_F1_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, KC_HOME, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, KC_PGDN,  KC_END,                       KC_LEFT, KC_DOWN,   KC_UP, KC_RGHT, _______, _______,
  _______, _______,  KC_DEL, _______, _______, KC_PGUP, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
 ),
[_F2_LAYER] = LAYOUT(
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, KC_LBRC, KC_RBRC,  KC_EQL,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______, _______, _______,
                    _______, _______, _______, _______, _______,     _______, _______, _______, _______, _______
 ),
[_F3_LAYER] = LAYOUT(
  _______, _______, _______,   KC_F3, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, _______, _______, _______, _______,                       _______, _______, _______, _______, _______, _______,
  _______, _______, DH_SWITCH, _______, _______, _______,                     _______, DH_JITTER, _______, _______, _______, _______,
  _______, _______, _______, DH_CONFIG, _______, _______, _______,  _______, _______, _______, _______, _______, _______, _______,
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

static void tap_deskhop_hotkey(uint8_t mods, uint8_t key1, uint8_t key2) {
    uint8_t saved_weak_mods = get_weak_mods();

    set_weak_mods(saved_weak_mods | mods);
    add_key(key1);
    if (key2 != KC_NO) {
        add_key(key2);
    }
    send_keyboard_report();

    wait_ms(DESKHOP_HOTKEY_TAP_MS);

    if (key2 != KC_NO) {
        del_key(key2);
    }
    del_key(key1);
    set_weak_mods(saved_weak_mods);
    send_keyboard_report();
}

bool process_record_user(uint16_t keycode, keyrecord_t *record) {
    switch (keycode) {
        case DH_CONFIG:
            if (record->event.pressed) {
                // DeskHop configuration mode: Left Control + Right Shift + C + O.
                tap_deskhop_hotkey(MOD_BIT_LCTRL | MOD_BIT_RSHIFT, KC_C, KC_O);
            }
            return false;
        case DH_JITTER:
            if (record->event.pressed) {
                // Enable DeskHop jitter mode: Left Control + Right Shift + J.
                tap_deskhop_hotkey(MOD_BIT_LCTRL | MOD_BIT_RSHIFT, KC_J, KC_NO);
            }
            return false;
        case DH_SWITCH:
            if (record->event.pressed) {
                // Switch DeskHop outputs: Left Control + Caps Lock.
                tap_deskhop_hotkey(MOD_BIT_LCTRL, KC_CAPS, KC_NO);
            }
            return false;
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
