#!/usr/bin/env python3
"""Send a one-shot QMK Raw HID claim for the personal Mac."""

from __future__ import annotations

import argparse
import sys
import time
from dataclasses import dataclass

DEFAULT_VENDOR_ID = 0xFC32
DEFAULT_PRODUCT_ID = 0x0287
DEFAULT_USAGE_PAGE = 0xFF60
DEFAULT_USAGE = 0x61

HOST_MAGIC = 0x42
HOST_MSG_CLAIM = 0x01
HOST_ID_PERSONAL = 0x01
RAW_HID_REPORT_SIZE = 32


@dataclass(frozen=True)
class HidFilter:
    vendor_id: int
    product_id: int
    usage_page: int
    usage: int


def parse_int(value: str) -> int:
    return int(value, 0)


def hid_api():
    try:
        import hid
    except ImportError as exc:
        raise SystemExit(
            "Missing Python package 'hid'. Install it on the personal Mac with: "
            "/opt/homebrew/bin/python3 -m pip install hidapi"
        ) from exc
    return hid


def hex_or_unknown(value: int | None, width: int = 4) -> str:
    if value is None:
        return "unknown"
    return f"0x{value:0{width}X}"


def decoded_path(path: bytes | str | None) -> str:
    if isinstance(path, bytes):
        return path.decode(errors="replace")
    if path is None:
        return ""
    return path


def enumerate_vid_pid(filter_: HidFilter) -> list[dict]:
    return list(hid_api().enumerate(filter_.vendor_id, filter_.product_id))


def raw_hid_devices(filter_: HidFilter) -> list[dict]:
    return [
        device
        for device in enumerate_vid_pid(filter_)
        if device.get("usage_page") == filter_.usage_page and device.get("usage") == filter_.usage
    ]


def find_raw_hid_path(filter_: HidFilter) -> bytes | str | None:
    devices = raw_hid_devices(filter_)
    if not devices:
        return None
    return devices[0]["path"]


def print_device(device: dict, file=sys.stdout) -> None:
    print(
        "VID={vid} PID={pid} usage_page={usage_page} usage={usage} "
        "interface={interface} manufacturer={manufacturer!r} product={product!r} "
        "path={path!r}".format(
            vid=hex_or_unknown(device.get("vendor_id")),
            pid=hex_or_unknown(device.get("product_id")),
            usage_page=hex_or_unknown(device.get("usage_page")),
            usage=hex_or_unknown(device.get("usage"), width=2),
            interface=device.get("interface_number", "unknown"),
            manufacturer=device.get("manufacturer_string"),
            product=device.get("product_string"),
            path=decoded_path(device.get("path")),
        ),
        file=file,
    )


def list_devices(filter_: HidFilter, all_devices: bool) -> int:
    if all_devices:
        devices = list(hid_api().enumerate())
    else:
        devices = enumerate_vid_pid(filter_)

    if not devices:
        if all_devices:
            print("No HID devices reported by hidapi.")
        else:
            print(f"No HID devices found for VID={hex_or_unknown(filter_.vendor_id)} PID={hex_or_unknown(filter_.product_id)}.")
        return 1

    for device in devices:
        print_device(device)
    return 0


def explain_no_match(filter_: HidFilter) -> None:
    devices = enumerate_vid_pid(filter_)
    if not devices:
        print(
            "No HID interfaces matched "
            f"VID={hex_or_unknown(filter_.vendor_id)} PID={hex_or_unknown(filter_.product_id)}.",
            file=sys.stderr,
        )
        print("Run with --list --all-devices to see the VID/PID macOS reports through the KVM.", file=sys.stderr)
        return

    print(
        "Found the keyboard VID/PID, but not the QMK Raw HID interface "
        f"(usage_page={hex_or_unknown(filter_.usage_page)} usage={hex_or_unknown(filter_.usage, width=2)}).",
        file=sys.stderr,
    )
    print("Interfaces found for that VID/PID:", file=sys.stderr)
    for device in devices:
        print_device(device, file=sys.stderr)
    print(
        "That usually means the keyboard is running firmware without RAW_ENABLE, "
        "the KVM is hiding vendor-defined HID interfaces, or the usage page/usage differs.",
        file=sys.stderr,
    )


def claim_payload(with_report_id: bool) -> list[int]:
    payload = [0] * RAW_HID_REPORT_SIZE
    payload[0] = HOST_MAGIC
    payload[1] = HOST_MSG_CLAIM
    payload[2] = HOST_ID_PERSONAL
    if with_report_id:
        return [0x00] + payload
    return payload


def send_claim_burst(path: bytes | str, with_report_id: bool) -> None:
    packet = claim_payload(with_report_id)
    device = hid_api().device()
    try:
        device.open_path(path)
        for delay in (0.0, 0.5, 1.0):
            if delay:
                time.sleep(delay)
            device.write(packet)
    finally:
        device.close()


def claim_once(filter_: HidFilter, with_report_id: bool, verbose: bool) -> int:
    path = find_raw_hid_path(filter_)
    if path is None:
        if verbose:
            explain_no_match(filter_)
        return 1

    if verbose:
        print(f"Sending personal host claim to {decoded_path(path)!r}.")
    send_claim_burst(path, with_report_id)
    return 0


def monitor(filter_: HidFilter, with_report_id: bool, poll_interval: float, verbose: bool) -> int:
    claimed_path = None

    while True:
        path = find_raw_hid_path(filter_)

        if path is None:
            if verbose and claimed_path is not None:
                print("Raw HID interface disappeared.")
            claimed_path = None
        elif path != claimed_path:
            if verbose:
                print(f"Sending personal host claim to {decoded_path(path)!r}.")
            send_claim_burst(path, with_report_id)
            claimed_path = path

        time.sleep(poll_interval)


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--vendor-id", type=parse_int, default=DEFAULT_VENDOR_ID)
    parser.add_argument("--product-id", type=parse_int, default=DEFAULT_PRODUCT_ID)
    parser.add_argument("--usage-page", type=parse_int, default=DEFAULT_USAGE_PAGE)
    parser.add_argument("--usage", type=parse_int, default=DEFAULT_USAGE)
    parser.add_argument("--poll-interval", type=float, default=1.0)
    parser.add_argument("--payload-only", action="store_true", help="omit the leading 0x00 report ID byte")
    parser.add_argument("--once", action="store_true", help="scan once, send one burst if the keyboard is present, then exit")
    parser.add_argument("--list", action="store_true", help="list HID interfaces for the configured VID/PID and exit")
    parser.add_argument("--all-devices", action="store_true", help="with --list, show every HID device macOS reports")
    parser.add_argument("--verbose", action="store_true", help="print matching and claim diagnostics")
    args = parser.parse_args()

    if args.poll_interval <= 0:
        parser.error("--poll-interval must be positive")

    filter_ = HidFilter(
        vendor_id=args.vendor_id,
        product_id=args.product_id,
        usage_page=args.usage_page,
        usage=args.usage,
    )

    if args.list:
        return list_devices(filter_, all_devices=args.all_devices)

    with_report_id = not args.payload_only

    if args.once:
        return claim_once(filter_, with_report_id, verbose=True)

    return monitor(
        filter_=filter_,
        with_report_id=with_report_id,
        poll_interval=args.poll_interval,
        verbose=args.verbose,
    )


if __name__ == "__main__":
    sys.exit(main())
