# Feasibility assessment

Assessment for the user's reported iOS 26.4.2, U.S./Illinois location, and $0 budget. Research checked during the November 17, 2026 Chicago session. Device version, Apple Account region and exact GamePigeon version were not inspected on the device.

## Mandatory capability matrix

| Requirement | Platform evidence | This project / verdict |
|---|---|---|
| A. Continuous screen capture while GamePigeon is foregrounded | ReplayKit system broadcasting delivers sample buffers to a broadcast upload extension after explicit user activation. This differs from app-local RPScreenRecorder capture. | Documented platform capability; GamePigeon-specific capture on this phone untested. Protected content may be omitted. |
| B. Analyze frames while Messages is foregrounded | RPBroadcastSampleHandler receives buffers in its own extension process. Processing must respect its lifecycle and memory budget. | Implementation supplied; no iPhone measurement. Ordinary host-app background execution is not being assumed for the CV engine. |
| C. Draw aligned lines over the actual game | Third-party apps have sandboxed UI; no documented public arbitrary cross-app transparent overlay/compositor entitlement was found. PiP is a system-controlled video surface, not such an overlay. | Unsupported for the independent companion app. This conclusion concerns supported APIs, not every conceivable future exploit. |
| D. Install a modified GamePigeon extension | Signed app extensions can be installed inside a containing app. That does NOT authorize replacing or modifying another developer's installed extension. A separately re-signed, lawfully modifiable copy is not inherently impossible. | Original app identity, entitlements, dependencies, current game protocol and functionality remain unverified. Do not equate “can sign an IPA” with “works with existing GamePigeon conversations.” |
| E. Install the finished assistant using only the iPhone | TestFlight/App Store can provide phone-only installation after developer-side signing/distribution. No such signed build or distribution account is available here. U.S. eligibility does not match the alternative-marketplace regions documented by Apple. | No verified $0 phone-only installation path for this new app. A web app does not solve the required capture and foreground presentation on this target OS. |
| F. Continue during normal gameplay without another computer | Broadcast extension execution and PiP are documented separately. The host renderer, loopback IPC, capture, interruptions and PiP must be tested together. | Plausible experimental route, not proven. Phone would do all gameplay processing if the test passes. Free signing may still require a computer for periodic renewal. |

## Repository findings

### donato-fiore/GameSeagull

Inspected main commit `40543762420adf6a6607c6dc1c804396db21ba34`. Latest listed release: 2.1, February 1, 2024. The source is a Theos/Logos Objective-C tweak. Its bundle filter targets `com.gamerdelights.gamepigeon.ext`; it hooks the game's `PoolScene`, `PoolScene2`, `PoolScene3` and `PoolBall` classes. The makefiles target iOS 14+, with separate rootful/rootless package builds, Cephei preferences, private Preferences.framework and substrate/libhooker support.

For extended lines, the constructor searches `mMove` implementations for a four-byte ARM64 instruction pattern and writes a replacement instruction through HookMemory. `Utils.m` resolves libhooker's patch function or falls back to MSHookMemory. It also changes game state/method results and colors the game's existing SpriteKit canvas. **It reuses the game's own aim rendering; it is not a standalone computer-vision engine or independently calibrated physics predictor.** The source does not establish modern GamePigeon binary compatibility.

This process-injection/runtime-patching architecture cannot simply be copied into a normally signed companion app. Merely changing a deployment target does not grant process access or executable-memory patching privileges.

Sources: [Tweak.x](https://github.com/donato-fiore/GameSeagull/blob/main/Tweak.x), [Utils.m](https://github.com/donato-fiore/GameSeagull/blob/main/Utils.m), [Makefile](https://github.com/donato-fiore/GameSeagull/blob/main/Makefile), [releases](https://github.com/donato-fiore/GameSeagull/releases).

### donato-fiore/TrollStoreGameSeagull

The source repository is essentially a README pointing to prepackaged IPAs, not a reproducible modern application source project. Releases v1.0 and v2.1 were inspected; the latter is a 90,278,934-byte IPA dated February 1, 2024.

The IPA was downloaded **for local inspection only**, never executed, installed or redistributed. Its package has a containing `Game Pack.app`, a Messages extension with identifier `com.gamerdelights.gamepigeon.ext`, and an embedded `gameseagull.dylib`. Mach-O inspection shows the extension loading that dylib; the dylib depends on CydiaSubstrate and private Preferences.framework. The extension's code-signature entitlements name the original developer's team, CloudKit containers and production push environment. Both inspected app executables have cryptid 0 in this package. That is an observation about this particular release, not permission to redistribute its contents.

The packaged dylib's symbol inventory differs from the main tweak source: it imports MSHookMessageEx and exposes an “Extended trajectory” setting, but does not expose the same HookMemory symbol. The README does not supply its build source. **The main repository's runtime-patching implementation must not be asserted to be the exact implementation inside this binary.** Metadata and dependency observations are recorded in `ipa-inspection.json` and `ipa-binary-inspection.json`.

TrollStore's official supported-version list excludes iOS 26.4.2. Re-signing could change/remove entitlements and app identity; compatibility is unproved, not categorically ruled out by “extensions cannot be sideloaded.” An independent same-process hook in a lawfully modifiable copy is a research hypothesis. It still does not supply a verified $0 iPhone-only install path, original app identity, rights to distribute the game, or current multiplayer compatibility. No repackaged GamePigeon is delivered.

Sources: [repository](https://github.com/donato-fiore/TrollStoreGameSeagull), [releases](https://github.com/donato-fiore/TrollStoreGameSeagull/releases), [TrollStore](https://github.com/opa334/TrollStore).

### mikee478/pool-tool

Inspected commit `dcf9656afcd740e956ca4f830205d705c46d42c6`; no releases or tags were listed. This is desktop Python/OpenCV/NumPy code. `screenshotter.py` uses Quartz to find a macOS QuickTime “Movie Recording” window and calls the macOS `screencapture` command. Processing uses configurable HSV masks, dilation and contours. Configuration is loaded from pickle. The inspected main loop displays original/filtered/contour images; it does not contain the full shot optimizer the requested product requires. The README reports vibrating lines, occasional crashes and slow capture. A repository screenshot depicts trajectory lines, but that image is not evidence that the checked-in loop supplies a complete live aiming engine.

It is not a native iPhone app, an iOS capture extension or a Windows solution. No upstream code or pickle is executed in this project. One supplied image was used as a development fixture and measured locally, with the input asset excluded from delivery.

Sources: [main.py](https://github.com/mikee478/pool-tool/blob/master/main.py), [image_processor.py](https://github.com/mikee478/pool-tool/blob/master/image_processor.py), [screenshotter.py](https://github.com/mikee478/pool-tool/blob/master/screenshotter.py).

All three repositories returned `license: null`, and no license file appeared in the inspected checkouts. Public visibility is not a license grant. This project's source is an independent implementation.

## Capture, analysis and display are different permissions

ReplayKit can deliver visible-screen frames to an extension. That permission does not grant control of Messages' UI or GamePigeon's SpriteKit scene. Vision/Core ML can analyze pixels, Metal can accelerate compute/rendering, and Accelerate can process buffers; none grants a cross-app UI surface. This prototype uses Core Image to normalize frames and a lightweight independent C++ detector instead of introducing an untrained ML model.

The current ScreenCaptureKit overview describes iOS support. However, Apple's machine-readable documentation metadata lists **iOS 27.0** as its introduction. The checked metadata is saved in `apple-sdk-availability.json`. It is therefore not the capture API for the user's iOS 26.4.2. It would also not itself grant an arbitrary overlay if the phone were upgraded. No OS upgrade is being recommended as a solution.

An iMessage extension we own can render inside its own presentation area. It cannot run simultaneously as an injected layer within GamePigeon's own extension. A separately implemented pool game could have perfect aiming assistance inside itself, but compatibility with GamePigeon's private game payloads and opponents would remain another unimplemented problem; that is not a substitute for this request.

PiP accepts sample-buffer video on supported iPhones. It provides a small opaque movable video window with system-controlled placement/sizes and controls. It cannot be freely stretched to a transparent, touch-through overlay aligned with every pixel of the game. This project renders a purple schematic table instead of replaying the captured screen, reducing recursive screen-within-screen feedback. The window still physically obscures whatever is beneath it; it should sit off the game table. This is a meaningful usability cost.

Supported APIs do not guarantee App Review acceptance for a nontraditional PiP use. Background audio capability is declared for actual PiP playback, not for a silent-audio keepalive trick. No private video-call UI trick, fake VoIP call or private compositor API is used.

## Installation alternatives and cost

| Route | What it would provide | Why it does not currently meet the goal |
|---|---|---|
| Safari / Home Screen web app | Phone-only access with no native signing | No demonstrated iOS 26.4.2 web capture/background/PiP pipeline for this task; adding a website to Home Screen does not grant native ReplayKit or overlay privileges. |
| TestFlight | Simple link-based iPhone installation for testers | Requires a signed developer build, App Store Connect access and distribution prerequisites. External testing may require review. Builds expire after 90 days. No invite exists. |
| App Store | Ordinary phone installation | Developer-side membership, signing, review and successful device testing are required. No approval guarantee. |
| Paid developer / ad hoc distribution | Could allow legitimate phone installation from an appropriately provisioned distribution workflow | $99/year U.S. Apple Developer membership unless an eligible sponsor/waiver applies; device registration/signing also needed. Paying does not solve aligned overlays. Not purchased or recommended before proof. |
| Alternative marketplaces / web distribution | Region-limited installation | Apple's current eligible regions do not include the United States. Some regions/features require later OS versions. Account-region changes alone do not create eligibility. |
| Enterprise certificates | Internal organizational distribution | Not a general-purpose consumer workaround; leaked/shared certificates and untrusted profiles are excluded. |
| On-device signing tools | May sign with credentials they already possess | No verified legitimate zero-cost bootstrap and refresh route on this exact phone was established. A signer cannot conjure Apple's entitlements or a trusted certificate. |
| AltStore Classic + Windows | Free personal signing through the user's own Apple Account | Initial computer setup and periodic renewal; seven-day free provisioning. Extension installation and this exact iOS build need testing. Not a once-ever laptop operation. |
| SideStore-style refresh | Potentially reduces recurring computer use after provisioning | Pairing/bootstrap and current OS/VPN/signing compatibility need verification; no proven simpler route for this project. Not promised as phone-only installation. |
| Cloud macOS build | Can compile while the user stays on the phone | Compilation is not signing or distribution. Public standard GitHub runners are the intended free route; GitHub access is currently blocked. |

The user can be a free TestFlight tester under an existing legitimate developer's distribution, but no such willing distributor is connected. Thus $99 is not a universal tester fee, and spending it now would be unjustified.

Windows as an external live processor would require a continuous iPhone-to-PC capture path and a display to view results. Even if supplied, it would not create an aligned native overlay on the iPhone and would violate the preferred no-external-hardware gameplay experience. The supplied C++ C ABI can accept continuous BGRA frames for that research path; no working Windows receiver or mirroring setup is claimed.

No verified jailbreak for this exact device/OS combination was established. The official TrollStore/Dopamine support information checked does not establish compatibility with iOS 26.4.2. No jailbreak, downgrade or exploit installation is recommended.

## Primary platform/distribution sources

- [Apple runtime sandbox and entitlement restrictions](https://support.apple.com/en-au/guide/security/sec15bfe098e/1/web/1)
- [Apple ReplayKit security architecture](https://support.apple.com/en-gb/guide/security/seca5fc039dd/web)
- [RPBroadcastSampleHandler](https://developer.apple.com/documentation/replaykit/rpbroadcastsamplehandler)
- [ScreenCaptureKit](https://developer.apple.com/documentation/screencapturekit) and [availability metadata](https://developer.apple.com/tutorials/data/documentation/screencapturekit.json)
- [Sample-buffer Picture in Picture](https://developer.apple.com/documentation/avkit/avpictureinpicturecontroller/contentsource-swift.class/init(samplebufferdisplaylayer:playbackdelegate:))
- [Apple alternative app distribution regions](https://support.apple.com/en-gb/118110)
- [Developer account / free provisioning limits](https://developer.apple.com/help/account/basics/about-your-developer-account/)
- [Apple Developer membership pricing](https://developer.apple.com/help/account/membership/program-enrollment)
- [TestFlight overview](https://developer.apple.com/help/app-store-connect/test-a-beta-version/testflight-overview/)
- [App Review guidelines](https://developer.apple.com/app-store/review/guidelines/)
- [AltStore Windows instructions](https://faq.altstore.io/altstore-classic/how-to-install-altstore-windows)
- [AltServer renewal dependency](https://faq.altstore.io/altstore-classic/altserver)
- [GitHub Actions billing](https://docs.github.com/en/actions/concepts/billing-and-usage)
