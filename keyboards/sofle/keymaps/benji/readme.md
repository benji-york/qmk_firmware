![SofleKeyboard default keymap](https://i.imgur.com/MZxVvm9.png)
![SofleKeyboard adjust layer](https://i.imgur.com/f5sKy0I.png)

# Default keymap for Sofle Keyboard

Layout in [Keyboard Layout Editor](http://www.keyboard-layout-editor.com/#/gists/76efb423a46cbbea75465cb468eef7ff) and [adjust layer](http://www.keyboard-layout-editor.com/#/gists/4bcf66f922cfd54da20ba04905d56bd4)

Features:

- Symmetric modifiers (CMD/Super, Alt/Opt, Ctrl, Shift)
- J+K combo for Escape.
- Tri-layer navigation and symbol layers.
- The OLED on the master half shows layer/key-position status.
- The OLED on the offhand half shows QMK Raw HID host labeling for a two-Mac KVM.
- Left encoder controls volume up/down/mute. Right encoder PGUP/PGDOWN.

## Two-Mac KVM host labeling

The offhand OLED displays one of two states:

```text
Personal Mac
Work Mac
```

On boot, USB reconnect, or USB wake, the keyboard reads the last persisted host from EEPROM and immediately predicts the opposite host, which matches a normal KVM switch. It then starts a hidden 2500 ms correction window: if the personal Mac helper sends the Raw HID claim packet, the OLED switches to `Personal Mac` and persists that state; if no personal claim arrives, the OLED switches to `Work Mac` and persists that state. A late personal claim still switches the OLED to `Personal Mac`.

An EEPROM reset defaults the persisted host to `Work Mac`.

The firmware defaults are:

- Vendor ID: `0xFC32`
- Product ID: `0x0287`
- Raw HID usage page: `0xFF60`
- Raw HID usage: `0x61`
- Claim payload: `42 01 01` followed by zero padding to 32 bytes

Build the firmware with:

```sh
qmk compile -kb sofle/rev1 -km benji -e CONVERT_TO=rp2040_ce
```

Manual personal-claim test:

```sh
/opt/homebrew/bin/python3 -m pip install hidapi
python3 util/qmk_kvm_host_claim.py --once
```

Personal Mac LaunchAgent install:

```sh
mkdir -p ~/Library/LaunchAgents
cp util/com.benjiyork.qmk-kvm-host-claim.plist ~/Library/LaunchAgents/
launchctl bootstrap "gui/$(id -u)" ~/Library/LaunchAgents/com.benjiyork.qmk-kvm-host-claim.plist
launchctl enable "gui/$(id -u)/com.benjiyork.qmk-kvm-host-claim"
```
