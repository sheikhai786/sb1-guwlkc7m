#!/usr/bin/env bash
set -euo pipefail
cd "$(dirname "$0")/.."
python3 scripts/generate_project.py
xcodebuild -version
xcodebuild -project GameSeagullNext.xcodeproj -scheme GameSeagullNext -configuration Release -sdk iphoneos -destination 'generic/platform=iOS' -derivedDataPath build/ios CODE_SIGNING_ALLOWED=NO build
mkdir -p build/package/Payload
cp -R build/ios/Build/Products/Release-iphoneos/GameSeagullNext.app build/package/Payload/
(cd build/package && zip -qr ../GameSeagullNext-UNSIGNED.ipa Payload)
echo 'Unsigned IPA built. It must be legitimately signed before installation.'
