# Verification and milestone status

**The user's definition of success has not been met.** There is no signed install link, compiled iOS app, physical-device capture test, or verified live gameplay integration. This deliverable is an engineering prototype with tested portable components.

## Completed evidence

| Check | Outcome | What it does not establish |
|---|---|---|
| C++17 compile with g++, `-Wall -Wextra -Werror` | Passed | Apple toolchain / iOS bridge compilation |
| Detection, geometry, optimization and dynamics tests | 40 named checks passed | General accuracy across current GamePigeon games |
| AddressSanitizer + UndefinedBehaviorSanitizer | 40 checks passed with leak detection disabled | Leak freedom; LeakSanitizer cannot run in this host's ptrace environment |
| Synthetic continuous input | 36/36 frames accepted, 36/36 aim directions detected, 33 distinct predicted outputs | ReplayKit capture, live UI, real-device FPS |
| Old GamePigeon reference image | 15 detected balls, six pockets, unique cue and observed guide | Held-out accuracy: the same image was used while tuning |
| Approximate manual ball-center labels | Mean error about 1.79 px, worst about 5.28 px in a 351×640 analysis image | Subpixel accuracy or exact internal game geometry |
| Swift files | Five files parsed without syntax errors | Swift SDK type checking, linking, extension/PiP correctness |
| Xcode project and configuration | Project object graph, plists and privacy manifests parsed | Successful xcodebuild, signing or install |
| Build pipeline | Source workflow and unsigned build script supplied | Workflow was not uploaded or run |

Exact host timings and raw results are in `artifacts/`. Host runtimes vary and must not be extrapolated to iPhone latency, battery consumption or extension memory use.

The reference image came from the pool-tool repository at the recorded commit. Its original image and the inspected TrollStore IPA are excluded from distribution. The GIF uses only generated test graphics and is labeled as a synthetic lab demonstration in every frame.

## Milestones

| Milestone | Status |
|---|---|
| 1. Feasibility | Research completed to the limits of public documentation, source/IPA inspection and available account access; A/B are documented, C is unsupported for a separate app, D/E/F retain explicit limitations. |
| 2. Central live proof | **Not achieved on iPhone.** Generated live-frame processing works on the host; that is not a substitute. |
| 3. Vision | Initial implementation and limited development-fixture tests complete; reliability on current gameplay unverified. |
| 4. Shot prediction | Geometry/heuristic optimizer and separate dynamics implemented and tested; GamePigeon calibration and measured success rate not done. |
| 5. Live integration | Swift capture/transport/PiP source implemented; not compiled or device verified. |
| 6. Installation | Xcode project and free cloud-build configuration prepared; GitHub access and legitimate signing/device installation are blockers. |
| 7. Verification | Portable-core tests and source/config checks complete; end-to-end acceptance incomplete. |

## Required physical-device acceptance test

This is an engineering test plan for the agent/developer after a successful signed build. It is not a request for the user to learn Xcode or repeatedly upload screenshots.

1. Confirm app and broadcast extension both install and appear under normal signing on the reported OS.
2. Start the receiver and system broadcast using explicit user permission. Verify continuing video sample delivery while Messages and GamePigeon are foregrounded.
3. Start sample-buffer PiP and verify it remains animated for at least ten minutes of gameplay. Measure capture-to-render delay (target under 250 ms at eight analyzed frames per second), memory and frame drops. The target is not an achieved result.
4. Verify loopback connection survives foreground/background transitions, pause/resume, PiP resize/movement, app interruptions and network changes. It should require no external hardware or internet during the match.
5. Verify buffers arriving with portrait/landscape orientation are mapped correctly, and that moving PiP does not produce recursive detections or misleading paths through hidden balls.
6. Verify stale/disconnected frames are suppressed when the renderer runs; detect frozen rendering rather than assuming stale timestamps can fix a suspended process.
7. Validate detector/geometry on independent layouts, touching balls, rails, racks, skins and hard mode. Verify classification and eight-ball selection rules. Measure per-ball recall, false detections, center error and pocketing outcomes before using words such as “reliable.”
8. Calibrate friction, impact response, rails, pockets and cue power. Test independent shots and report errors. Do not advertise a power percentage from the current distance heuristic.
9. Close the host, stop/restart broadcasting, lock/unlock and receive a call; the assistant must recover or visibly stop. Verify audio is not recorded or transmitted.
10. Test free-signing renewal with the extension retained. Confirm the actual laptop dependency before describing installation as low effort.

Failure of steps 2–4 means the central proposed phone-only live presentation route has not worked. Do not buy a developer membership or tell the user to rely on it until that failure is understood.
