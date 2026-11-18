#!/usr/bin/env python3
"""Reject incomplete device packages and emit a reproducible build manifest."""
import hashlib
import json
import pathlib
import plistlib
import struct
import sys
import zipfile

ipa = pathlib.Path(sys.argv[1])
with zipfile.ZipFile(ipa) as archive:
    names = set(archive.namelist())
    app = "Payload/GameSeagullNext.app/"
    extension = app + "PlugIns/Broadcast.appex/"
    bundles = []
    for path, package in ((app, "APPL"), (extension, "XPC!")):
        info = plistlib.loads(archive.read(path + "Info.plist"))
        assert info["CFBundlePackageType"] == package
        assert "$" not in info["CFBundleIdentifier"]
        executable = path + info["CFBundleExecutable"]
        binary = archive.read(executable)
        magic, cpu, _, filetype = struct.unpack_from("<IIII", binary)
        assert magic == 0xFEEDFACF and cpu == 0x0100000C, "Expected arm64 Mach-O"
        assert filetype == 2, "Expected executable Mach-O"
        assert path + "PrivacyInfo.xcprivacy" in names
        bundles.append({"id": info["CFBundleIdentifier"], "version": info["CFBundleShortVersionString"],
                        "minimumOS": info["MinimumOSVersion"], "executable": executable,
                        "architecture": "arm64", "bytes": len(binary)})
    ext = plistlib.loads(archive.read(extension + "Info.plist"))["NSExtension"]
    assert ext["NSExtensionPointIdentifier"] == "com.apple.broadcast-services-upload"
    assert ext["RPBroadcastProcessMode"] == "RPBroadcastProcessModeSampleBuffer"
    assert not any(n.endswith("embedded.mobileprovision") for n in names)
manifest = {"file": ipa.name, "bytes": ipa.stat().st_size,
            "sha256": hashlib.sha256(ipa.read_bytes()).hexdigest(), "bundles": bundles,
            "signing": "unsigned: personal signing required", "deviceTested": False}
output = ipa.with_suffix(".manifest.json")
output.write_text(json.dumps(manifest, indent=2) + "\n")
print(output.read_text())
