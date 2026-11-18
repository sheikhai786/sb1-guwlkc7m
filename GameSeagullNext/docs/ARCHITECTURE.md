# Architecture and engineering limits

## Native path (implemented, not device tested)

The user starts ReplayKit through RPSystemBroadcastPickerView. The broadcast upload extension receives CMSampleBuffers independently of the host's foreground state. Its serial worker holds at most one accepted frame at a time, rejects excess work and throttles to 4/8/12 frames per second. Audio is discarded. Core Image applies the orientation attachment and resizes the longest side to 640 pixels into a reusable BGRA buffer. The extension invokes the C++ detector, geometric predictor and candidate optimizer.

Only small geometry/configuration JSON packets cross a localhost TCP connection on 127.0.0.1:49382. The listener binds specifically to loopback, not the LAN. There is no App Group requirement, external service, image upload, microphone use or disk recording. Framing has a 64 KiB payload limit and one outgoing message in flight. Reconnection is rate limited. **This prototype's loopback peers are not cryptographically authenticated**; another app on the same device could potentially interfere with the session. No captured images or message text cross this transport, but geometry is not treated as a secure channel. Production deployment would require stronger peer validation or a provisioned App Group mechanism.

The host creates a sample-buffer-backed AVPictureInPictureController and renders a schematic table. The actual game screenshot is never rendered into PiP, avoiding recursive copies of the screen. Purple felt distinguishes the schematic from the detector's green/teal input model. The host declares the background audio mode used for PiP video playback; no silent audio loop or invented background entitlement is supplied.

The receiver suppresses frames older than 750 ms **while its rendering timer executes**. If iOS suspends the entire renderer, the last PiP frame could freeze; a timer cannot protect against its own suspension. Testing foreground/background transitions is therefore a release gate, not a cosmetic check.

## Vision

The independent detector combines largest connected green/teal felt-region segmentation, aspect/coverage checks, cloth-color estimation, foreground morphological opening, ball-shaped components and a radial contrast search to recover touching objects. It verifies dark regions at the six expected pocket anchors. It classifies cue/eight/solid/stripe from brightness and peripheral white area; it does not read ball numbers. It estimates the existing thin white aiming line with a polar search and rejects ambiguous directions. It reports no observed-aim path when no guide is visible; suggested aim is kept separate.

Coordinates are pixel positions in the oriented analysis image. Circle radius and cushion boundaries are estimates, not GamePigeon's internal coordinates. A single fixed-green/teal table model does not cover every skin, perspective, animation, rack, light effect, occlusion or future UI. The table must fit within the frame. Contacts, close clusters, near-rail balls and PiP covering part of the table can produce missed or misclassified balls. Validity/confidence numbers are heuristic quality gates, not calibrated probabilities. The detector cannot reliably know that every ball was observed.

Motion suppression compares nearest positions between successive frames and waits for displacement to fall below a threshold. It is a simple settle check, not persistent numbered-ball tracking. No device-training dataset or held-out benchmark was available. The one upstream image was also used during tuning, so its good result must not be presented as an independent accuracy estimate.

## Geometry and optimization

The geometry uses finite ball radii, earliest ray/circle contact, cushion inset by ball radius and equal-mass no-spin impact normals. The cue's post-impact direction is the tangential residual. It traces a bounded number of rail reflections and stops each post-impact branch on its next ball contact. It does not solve every subsequent collision in that analytic path.

The candidate optimizer constructs ghost-ball contact points for direct shots, mirrored-pocket one-cushion banks and two-object-ball combinations. Segment clearance tests exclude blocked paths. Rankings penalize distance, severe cut angles, banks/combinations and estimated scratches. The score is a heuristic rank, **not pocketing probability**. The ranking does not yet optimize a measured next-shot position or the complete rules state. Users select their group and explicitly permit the eight ball; the app does not infer whose turn it is, legal eight-ball status, fouls or an opponent's intentions.

`Dynamics.cpp` separately implements adaptive substeps, friction, rail restitution, equal-mass ball impulses and pocket removal. It is suitable for calibration experiments. Uncalibrated dynamics are not used to claim a power setting or final-position accuracy in the UI. No spin, throw, cushion compression, pocket jaw model or proprietary game physics was measured.

The `relativeEffort` value in core output is a distance/cut heuristic. The UI deliberately does not label it as a power percentage. Power recommendations become meaningful only after measuring cue-control displacement versus initial speed and fitting rolling/rail/collision parameters against actual GamePigeon trajectories.

## Calibration work still required

Use a successful user-authorized broadcast to collect short, on-device trajectory samples in a controlled test session. Establish table/ball coordinate scale, rail contact positions, pocket capture behavior and shot-control-to-speed mapping across several power levels. Fit deceleration only on unobstructed rolling intervals; estimate rail restitution from pre/post-impact velocities; compare measured and predicted impacts on independent shots. Keep measured values separate by game version and table layout. No such data was captured in this session, and no calibration coefficients are claimed.

## Build/configuration

The iOS targets use public iOS 16+ capture/video APIs and C++17, so the minimum deployment target does not require iOS 27. The project has a host app and one broadcast upload extension. Privacy manifests declare no data collection/tracking and explain elapsed-time API use. There is no shared group, push notification, iCloud or privileged entitlement.

`scripts/generate_project.py` deterministically generates the Xcode project, scheme, plists and privacy manifests without XcodeGen, CocoaPods or paid dependencies. `build-ios.sh` requests an unsigned generic-device release build and packages it as an explicitly UNSIGNED IPA. An unsigned build is useful for compilation checks and later legitimate signing; it is not a directly installable product.
