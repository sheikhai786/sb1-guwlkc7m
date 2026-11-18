# What you can install today

**There is no installable build or TestFlight link yet.** The delivered ZIP is a source project, not an app. Renaming it to `.ipa`, downloading it in Safari, or installing a profile will not make it run.

No money was spent. Do not buy an Apple Developer membership to test whether the live view works.

## Smallest next action

The agent needs access to a dedicated GitHub repository or a GitHub browser sign-in to create one and run the free macOS compile workflow. The connected GitHub account returned no accessible repositories; the browser reached GitHub's sign-in screen. No repository, CI run or release was created. Once access is available, the technical uploading/building work can be handled by the agent. A public repository is the intended $0 standard-runner path; do not add private data or signing keys.

That compiles the prototype but still does not prove it works on the phone. A physical-device test must happen before any paid-distribution decision.

## Cheapest installation candidate after compilation

The best documented free fallback found is **AltStore Classic with your Windows laptop and your own Apple Account**. It does not require owning a Mac. Exact iOS 26.4.2 compatibility and retaining the ReplayKit extension still need to be tested.

Do not follow these steps until there is a successfully compiled IPA and the agent has checked its contents:

1. Install AltServer for Windows from [AltStore's official site](https://altstore.io/) and the Apple dependencies listed in its [Windows guide](https://faq.altstore.io/altstore-classic/how-to-install-altstore-windows).
2. Connect the unlocked iPhone once, approve Trust, and enable Wi-Fi sync as that guide directs.
3. Use AltServer's “Install AltStore” with your own Apple Account. Enter credentials only in the legitimate installation flow, never in this chat.
4. On the iPhone, trust your own developer identity and enable Developer Mode when requested.
5. Open the compiled IPA in AltStore and **retain the broadcast extension**. The companion app alone cannot capture GamePigeon. If signing or extension installation fails, stop and inspect the error; do not buy a certificate to bypass it.

Free provisioning expires after seven days. AltServer can refresh over the same Wi-Fi network, but the laptop is still needed periodically. It should not be required during a match if the on-device prototype passes testing. This is **not** a promise of one-time-only Windows setup.

## Intended use after a successful device test

1. Open Seagull Next and tap **Start receiver**.
2. Tap the screen-broadcast button, select **Seagull Next Capture** if prompted, and start broadcasting.
3. Tap **Open floating view**, then open GamePigeon in Messages.
4. Place the floating window outside the pool table. White/cyan/orange lines show estimated cue/object/post-contact paths; dashed green lines show a proposed shot. You still aim and shoot manually.
5. Stop the broadcast through Control Center when finished. Stopping only the receiver does not stop ReplayKit's system broadcast.

If the preview freezes in Messages, PiP cannot start, the extension disappears, or the detector cannot see the table, the central hypothesis failed on that device. Do not interpret a frozen diagram as live guidance.

## If you require the exact original experience

The prototype's miniature table will not satisfy a requirement for transparent lines directly on the actual GamePigeon table. No verified supported, free, phone-only installation and overlay solution was established. A separately modified game copy remains unverified and is not included. Paying for signing does not remove the platform UI restriction.
