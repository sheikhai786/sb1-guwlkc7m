# Install the personal-test prototype

The app and ReplayKit extension have compiled successfully on GitHub. Download `GameSeagullNext-UNSIGNED.ipa` from this repository's **Releases** page. The manifest beside it records package checks and its SHA-256. This is an unsigned experimental app, not a one-tap Safari install or a tested GamePigeon overlay.

No money was spent. Do not buy an Apple Developer membership or a certificate for this test.

## If you already have AltStore Classic

1. Download the IPA in Safari and save it to Files. Use the IPA asset, not GitHub's Source code ZIP.
2. Open AltStore Classic, select My Apps, tap +, and select the IPA from Files. Use your configured AltServer or Remote AltServer.
3. **Keep the app extensions** if asked. `Seagull Next Capture` is required.
4. Complete any Apple-account verification in the installer. Never share Apple passwords or codes in chat.

## If you do not have AltStore yet

For this US iPhone / $0 setup, no verified entirely phone-only initial installation route was established. Use your Windows laptop for initial AltStore installation and pairing:

1. Follow the [official Windows installation guide](https://faq.altstore.io/altstore-classic/how-to-install-altstore-windows). Install its linked Apple dependencies and the latest official AltServer. Windows AltServer 1.7.4 fixed iOS 26.4 signing/launch issues; use that or later.
2. Connect the unlocked iPhone by USB, tap Trust, and enable Wi-Fi sync as the guide directs.
3. In the Windows tray, select AltServer > Install AltStore > your iPhone, and sign in with your own Apple Account.
4. On iPhone, trust your own developer identity in Settings > General > VPN & Device Management. Enable Settings > Privacy & Security > Developer Mode and restart if requested.
5. Install the IPA using the steps above. No Mac or Xcode is needed.

Do not disable antivirus or install unrelated profiles to get past an error. If signing fails, keep the actual error for diagnosis rather than purchasing a certificate.

## Reduce future laptop use

AltStore Classic 2.3 and later provide Remote AltServer. In AltStore Settings, choose **Set up Remote AltServer**, complete its PC-pairing step, and install LocalDevVPN through that official setup flow. The official guide then supports installing and renewing apps on Wi-Fi with LocalDevVPN enabled, without your computer connected. Cellular alone does not work for that flow. This is a documented installation option, not a test performed on your iPhone.

Free signing normally lasts seven days. Refresh AltStore and Seagull Next before expiration. If Remote AltServer is unavailable, use the paired laptop as the fallback. The laptop is not intended to process gameplay.

Sources: [Remote AltServer setup](https://github.com/altstoreio/FAQ/blob/main/altstore-classic/remote-altservers.md), [AltStore releases](https://github.com/altstoreio/FAQ/blob/main/release-notes/altstore.md), [AltServer releases](https://github.com/altstoreio/FAQ/blob/main/release-notes/altserver.md).

## First live test

1. Open Seagull Next and tap **Start receiver**.
2. Tap **Start screen broadcast** and select **Seagull Next Capture**. Leave the microphone off.
3. Tap **Open floating view**, then open GamePigeon in Messages.
4. Keep the floating window away from the pool table. It should show a continuously changing miniature table as you adjust the cue.
5. Stop screen broadcasting in Control Center when finished.

This draws a miniature table in Picture in Picture, not transparent lines on GamePigeon itself. Screen capture may include visible messages; frames are processed locally and audio is discarded.

If the window freezes, PiP fails to start, or the broadcast extension is missing, the central live route has not passed its physical-device test. A successful compile does not prove it works on iPhone 12 Pro / iOS 26.4.2. No calibrated shot-power percentage or guaranteed wins are claimed.
