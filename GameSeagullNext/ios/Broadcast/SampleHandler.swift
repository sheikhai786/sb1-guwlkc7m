import ReplayKit
import CoreImage
import ImageIO
import Network

final class SampleHandler: RPBroadcastSampleHandler {
    private let worker = DispatchQueue(label: "seagull.analysis", qos: .userInitiated)
    private let gate = DispatchSemaphore(value: 1)
    private let context = CIContext(options: [.cacheIntermediates: false])
    private var link: FramedConnection?
    private var config = Configuration()
    private var last = 0.0
    private var reconnectAt = 0.0
    private var active = false
    private var output: CVPixelBuffer?
    private var previous: [[Double]] = []

    override func broadcastStarted(withSetupInfo setupInfo: [String : NSObject]?) {
        worker.async { self.active = true; self.connect() }
    }
    override func broadcastPaused() { worker.async { self.active = false; self.sendStatus("Capture paused") } }
    override func broadcastResumed() { worker.async { self.active = true; self.previous = [] } }
    override func broadcastFinished() {
        worker.async { self.active = false; self.sendStatus("Capture stopped"); self.link?.cancel(); self.link = nil; self.output = nil }
    }
    private func connect() {
        let pipe = FramedConnection(NWConnection(host: "127.0.0.1", port: seagullPort, using: .tcp))
        link = pipe
        pipe.onMessage = { [weak self] data in
            if let c = try? JSONDecoder().decode(Configuration.self, from: data) { self?.config = c }
        }
        pipe.onClose = { [weak self] in self?.link = nil }
        pipe.start(queue: worker)
    }
    private func sendStatus(_ reason: String) {
        let object: [String: Any] = ["valid": false, "reason": reason, "capturedAt": ProcessInfo.processInfo.systemUptime]
        if let data = try? JSONSerialization.data(withJSONObject: object) { link?.send(data) }
    }
    override func processSampleBuffer(_ sampleBuffer: CMSampleBuffer, with sampleBufferType: RPSampleBufferType) {
        guard sampleBufferType == .video, gate.wait(timeout: .now()) == .success else { return }
        worker.async {
            defer { self.gate.signal() }
            autoreleasepool { self.process(sampleBuffer) }
        }
    }
    private func process(_ sample: CMSampleBuffer) {
        let now = ProcessInfo.processInfo.systemUptime
        guard active, now - last >= 1 / Double(max(2, min(12, config.fps))) else { return }
        last = now
        if link == nil && now >= reconnectAt { reconnectAt = now + 1; connect() }
        guard config.enabled else { sendStatus("Assistance disabled"); return }
        guard let input = CMSampleBufferGetImageBuffer(sample) else { return }
        var image = CIImage(cvPixelBuffer: input)
        if let number = CMGetAttachment(sample, key: RPVideoSampleOrientationKey as CFString, attachmentModeOut: nil) as? NSNumber,
           let orientation = CGImagePropertyOrientation(rawValue: number.uint32Value) { image = image.oriented(orientation) }
        let extent = image.extent
        let scale = min(1, 640 / max(extent.width, extent.height))
        image = image.transformed(by: CGAffineTransform(translationX: -extent.minX, y: -extent.minY)).transformed(by: CGAffineTransform(scaleX: scale, y: scale))
        let width = max(64, Int(image.extent.width)), height = max(64, Int(image.extent.height))
        if output == nil || CVPixelBufferGetWidth(output!) != width || CVPixelBufferGetHeight(output!) != height {
            output = nil
            let attrs: [CFString: Any] = [kCVPixelBufferIOSurfacePropertiesKey: [:]]
            guard CVPixelBufferCreate(kCFAllocatorDefault, width, height, kCVPixelFormatType_32BGRA, attrs as CFDictionary, &output) == kCVReturnSuccess else { sendStatus("Frame allocation failed"); return }
        }
        guard let output = output else { return }
        context.render(image, to: output, bounds: CGRect(x: 0, y: 0, width: width, height: height), colorSpace: CGColorSpaceCreateDeviceRGB())
        let text = SGBridge.analyze(output, group: config.group, advanced: config.advanced, eightAllowed: config.eightAllowed)
        guard let raw = text.data(using: .utf8), var state = (try? JSONSerialization.jsonObject(with: raw)) as? [String: Any] else { return }
        let balls = (state["balls"] as? [[String: Any]] ?? []).compactMap { $0["p"] as? [Double] }
        // Suppress recommendations while balls move; nearest-neighbor displacement, not unstable component IDs.
        let moving = balls.count != previous.count || balls.contains { p in
            guard p.count == 2 else { return true }
            let nearest = previous.filter { $0.count == 2 }.map { hypot($0[0]-p[0], $0[1]-p[1]) }.min() ?? 999
            return nearest > 2
        }
        previous = balls
        if moving { state["prediction"] = []; state["shots"] = []; state["reason"] = "Waiting for balls to settle" }
        state["capturedAt"] = now
        state["analysisMS"] = (ProcessInfo.processInfo.systemUptime-now)*1000
        if let data = try? JSONSerialization.data(withJSONObject: state) { link?.send(data) }
    }
}
