# GameSeagull Next — engineering prototype

**This is source code and tested host-side components, not an installable or device-verified iPhone release. No purchase is needed or recommended at this stage.**

The original goal was a $0, no-jailbreak, iPhone-only live GamePigeon aiming overlay. No verified route satisfying all those conditions was found. Ordinary signing cannot give a separate app the ability to draw transparent, table-aligned lines inside the installed GamePigeon extension.

This project implements the closest plausible independent native alternative: a user-started ReplayKit broadcast extension analyzes screen frames on the iPhone and sends detected geometry to a companion app that renders a continuously updated miniature table in Picture in Picture. **The capture/PiP/background combination is an unverified hypothesis until a signed build runs on a physical iPhone.** It does not place lines directly on the real game or automate taps.

## What exists

- C++17 ball/table/pocket detector and white-guideline direction estimator, shared with the iOS extension.
- Collision geometry, post-impact cue/object directions, up to three rail bounces, scratch warnings, direct/bank/combination candidate ranking and manual solids/stripes selection.
- Separate time-stepped simulation with friction, restitution, ball collisions and pockets. This is tested but is not used to manufacture uncalibrated game-power percentages in the live UI.
- SwiftUI controls, ReplayKit sample handler, bounded frame processing, orientation handling, loopback transport, sample-buffer PiP renderer and stale-frame suppression while the renderer is running.
- Ordinary Xcode project, Info.plists, privacy manifests, project generator and an unsigned iOS build script.
- A GitHub Actions workflow ready to test the core and compile the app on a standard public-repository macOS runner. It has **not** run. Private repository jobs are disabled to avoid runner charges.
- Test logs, reference-image measurements and a clearly labeled synthetic animation.

## What was actually verified

The C++ core compiled and ran on Linux. The verification report in `docs/VERIFICATION.md` contains the exact outcomes. The iOS app and extension have **not** been compiled, signed, installed, or tested. The generated Xcode project is build preparation, not a successful build.

## Where to start

For the non-developer status and smallest next steps, read `docs/INSTALLATION.md`. For the technical evidence, read `docs/FEASIBILITY.md`. For source architecture and limitations, read `docs/ARCHITECTURE.md`.

`artifacts/synthetic-live-demo.gif` demonstrates changing generated frames going through the real detection/geometry engine. It is **not** a recording of GamePigeon, ReplayKit, an iPhone, or PiP.

## Reproduce core tests

On a development runner with g++ and Bash:

```sh
bash scripts/test.sh
```

No external C++ library is required. The optional reference-image and synthetic-animation scripts require Pillow. The reference image is not bundled because its repository supplies no license; `validate_reference.py` accepts a separately obtained copy.

## Cloud iOS build

Once repository access is available, the agent can publish this tree to a dedicated public repository and run its workflow. The resulting `GameSeagullNext-UNSIGNED.ipa` is **not directly installable**. No signing secrets are needed for that compile check. `scripts/build-ios.sh` is also runnable on an authorized macOS build host; the user does not need to own a Mac.

## Provenance

This implementation was written independently. No GamePigeon binary, artwork, original tweak source, CydiaSubstrate, private framework, certificate or provisioning profile is included. The three requested repositories were inspected as research. None exposed a license in the inspected checkout/API metadata, so their code is not incorporated.

The original source here is provided under MIT; see `LICENSE`. The project is unaffiliated with GamePigeon or GameSeagull's authors.
