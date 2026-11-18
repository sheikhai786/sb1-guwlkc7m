import UIKit
import SwiftUI
import AVKit
import Network

final class LiveController: NSObject, ObservableObject, AVPictureInPictureSampleBufferPlaybackDelegate, AVPictureInPictureControllerDelegate {
    let layer = AVSampleBufferDisplayLayer()
    @Published var status = "Start the receiver, then screen broadcast"
    @Published var pipPossible = false
    @Published var running = false
    @Published var config = Configuration()
    @Published var assisted = true
    @Published var brightLines = true
    @Published var frame: Frame?
    private var pip: AVPictureInPictureController?
    private var listener: NWListener?
    private var peer: FramedConnection?
    private var timer: Timer?
    private var observation: NSKeyValueObservation?
    private var paused = false
    private var lastSettings = 0.0

    override init() {
        super.init()
        layer.videoGravity = .resizeAspect
        var timebase: CMTimebase?
        CMTimebaseCreateWithSourceClock(allocator: kCFAllocatorDefault, sourceClock: CMClockGetHostTimeClock(), timebaseOut: &timebase)
        if let timebase = timebase {
            CMTimebaseSetTime(timebase, time: CMClockGetTime(CMClockGetHostTimeClock()))
            CMTimebaseSetRate(timebase, rate: 1)
            layer.controlTimebase = timebase
        }
        if AVPictureInPictureController.isPictureInPictureSupported() {
            pip = AVPictureInPictureController(contentSource: .init(sampleBufferDisplayLayer: layer, playbackDelegate: self))
            pip?.delegate = self
            pip?.requiresLinearPlayback = true
            observation = pip?.observe(\.isPictureInPicturePossible, options: [.initial, .new]) { [weak self] controller, _ in
                DispatchQueue.main.async { self?.pipPossible = controller.isPictureInPicturePossible }
            }
        } else { status = "Picture in Picture unavailable on this device" }
    }
    func start() {
        guard !running else { return }
        do {
            try AVAudioSession.sharedInstance().setCategory(.playback, mode: .moviePlayback, options: [.mixWithOthers])
            try AVAudioSession.sharedInstance().setActive(true)
            let params = NWParameters.tcp
            params.requiredLocalEndpoint = .hostPort(host: "127.0.0.1", port: seagullPort)
            let server = try NWListener(using: params)
            server.newConnectionHandler = { [weak self] connection in
                guard let self = self else { connection.cancel(); return }
                guard self.peer == nil else { connection.cancel(); return }
                let peer = FramedConnection(connection)
                self.peer = peer
                peer.onMessage = { [weak self] data in
                    guard let self = self, let incoming = try? JSONDecoder().decode(Frame.self, from: data) else { return }
                    self.frame = incoming
                    self.status = incoming.reason
                }
                peer.onClose = { [weak self] in self?.peer = nil; self?.frame = nil; self?.status = "Capture disconnected" }
                peer.start(queue: .main)
            }
            server.stateUpdateHandler = { [weak self] state in
                if case .failed(let error) = state { self?.stop(); self?.status = "Receiver: \(error.localizedDescription)" }
            }
            listener = server
            server.start(queue: .main)
            running = true
            status = "Receiver ready. Start screen broadcast below."
            timer = Timer.scheduledTimer(withTimeInterval: 1.0/8, repeats: true) { [weak self] _ in self?.tick() }
        } catch { stop(); status = error.localizedDescription }
    }
    func stop() {
        timer?.invalidate(); timer = nil
        peer?.cancel(); peer = nil; listener?.cancel(); listener = nil
        pip?.stopPictureInPicture(); layer.flushAndRemoveImage()
        frame = nil; running = false
        try? AVAudioSession.sharedInstance().setActive(false, options: .notifyOthersOnDeactivation)
        status = "Receiver stopped. Stop screen broadcast in Control Center too."
    }
    func floatWindow() {
        guard pipPossible else { status = "Wait for video to become ready"; return }
        pip?.startPictureInPicture()
    }
    private func tick() {
        let now = ProcessInfo.processInfo.systemUptime
        if now-lastSettings > 0.5 {
            lastSettings = now
            if let data = try? JSONEncoder().encode(config) { peer?.send(data) }
        }
        guard !paused else { return }
        if layer.status == .failed { layer.flush() }
        guard layer.isReadyForMoreMediaData else { return }
        let stale = now-(frame?.capturedAt ?? 0) > 0.75
        let image = draw(frame: stale ? nil : frame)
        var pixel: CVPixelBuffer?
        let attrs: [CFString: Any] = [kCVPixelBufferCGImageCompatibilityKey: true, kCVPixelBufferCGBitmapContextCompatibilityKey: true, kCVPixelBufferIOSurfacePropertiesKey: [:]]
        guard CVPixelBufferCreate(kCFAllocatorDefault, 360, 640, kCVPixelFormatType_32BGRA, attrs as CFDictionary, &pixel) == kCVReturnSuccess, let pixel = pixel else { return }
        CVPixelBufferLockBaseAddress(pixel, [])
        if let context = CGContext(data: CVPixelBufferGetBaseAddress(pixel), width: 360, height: 640, bitsPerComponent: 8, bytesPerRow: CVPixelBufferGetBytesPerRow(pixel), space: CGColorSpaceCreateDeviceRGB(), bitmapInfo: CGImageAlphaInfo.premultipliedFirst.rawValue | CGBitmapInfo.byteOrder32Little.rawValue), let cg = image.cgImage {
            context.translateBy(x: 0, y: 640); context.scaleBy(x: 1, y: -1)
            context.draw(cg, in: CGRect(x: 0, y: 0, width: 360, height: 640))
        }
        CVPixelBufferUnlockBaseAddress(pixel, [])
        var format: CMVideoFormatDescription?
        guard CMVideoFormatDescriptionCreateForImageBuffer(allocator: kCFAllocatorDefault, imageBuffer: pixel, formatDescriptionOut: &format) == noErr, let format = format else { return }
        var timing = CMSampleTimingInfo(duration: CMTime(value: 1, timescale: 8), presentationTimeStamp: CMClockGetTime(CMClockGetHostTimeClock()), decodeTimeStamp: .invalid)
        var sample: CMSampleBuffer?
        if CMSampleBufferCreateReadyWithImageBuffer(allocator: kCFAllocatorDefault, imageBuffer: pixel, formatDescription: format, sampleTiming: &timing, sampleBufferOut: &sample) == noErr, let sample = sample { layer.enqueue(sample) }
    }
    private func draw(frame: Frame?) -> UIImage {
        let format = UIGraphicsImageRendererFormat(); format.scale = 1
        return UIGraphicsImageRenderer(size: CGSize(width: 360, height: 640), format: format).image { renderer in
            let c = renderer.cgContext
            UIColor(red: 0.06, green: 0.07, blue: 0.12, alpha: 1).setFill(); c.fill(CGRect(x: 0, y: 0, width: 360, height: 640))
            func text(_ value: String, _ y: CGFloat, _ size: CGFloat = 15, _ color: UIColor = .white) {
                (value as NSString).draw(in: CGRect(x: 16, y: y, width: 328, height: 45), withAttributes: [.font: UIFont.systemFont(ofSize: size, weight: .medium), .foregroundColor: color])
            }
            text("SEAGULL NEXT · LIVE PROTOTYPE", 16, 17)
            guard let frame = frame, frame.valid, let t = frame.table, t.count == 4, t[2]>t[0], t[3]>t[1] else {
                text(frame?.reason ?? "No fresh capture — guidance hidden", 90, 20)
                text("Open GamePigeon after starting broadcast. Keep this window off the table.", 160)
                return
            }
            let scale = min(320/(t[2]-t[0]), 470/(t[3]-t[1]))
            let ox = (360-(t[2]-t[0])*scale)/2, oy = 74.0
            func point(_ p: [Double]) -> CGPoint { CGPoint(x: ox+(p[0]-t[0])*scale, y: oy+(p[1]-t[1])*scale) }
            UIColor(red: 0.14, green: 0.10, blue: 0.24, alpha: 1).setFill()
            c.fill(CGRect(x: ox, y: oy, width: (t[2]-t[0])*scale, height: (t[3]-t[1])*scale))
            for p in frame.pockets ?? [] where p.count == 2 { let q = point(p); UIColor.black.setFill();c.fillEllipse(in: CGRect(x:q.x-8,y:q.y-8,width:16,height:16)) }
            for b in frame.balls ?? [] where b.p.count == 2 {
                let q = point(b.p), r = max(3,b.r*scale)
                let color: UIColor = b.kind == 1 ? .white : b.kind == 4 ? .black : b.kind == 3 ? .systemOrange : .systemBlue
                color.setFill(); c.fillEllipse(in: CGRect(x:q.x-r,y:q.y-r,width:2*r,height:2*r))
                UIColor.white.setStroke();c.setLineWidth(1);c.strokeEllipse(in: CGRect(x:q.x-r,y:q.y-r,width:2*r,height:2*r))
            }
            var lines = frame.prediction ?? []
            if assisted, let shot = frame.shots?.first { lines += shot.lines }
            for line in lines where line.a.count == 2 && line.b.count == 2 {
                let colors: [UIColor] = [.white,.cyan,.systemOrange,.systemGreen]
                colors[max(0,min(3,line.role))].withAlphaComponent(brightLines ? 1 : 0.65).setStroke()
                c.setLineWidth(brightLines ? 2.5 : 1.5); c.setLineDash(phase: 0, lengths: line.role == 3 ? [6,4] : [])
                c.move(to: point(line.a));c.addLine(to:point(line.b));c.strokePath()
            }
            c.setLineDash(phase: 0, lengths: [])
            let shot = assisted ? frame.shots?.first : nil
            text(shot.map { "\($0.type.capitalized) · heuristic rank \(Int($0.score))/100" } ?? "Observed aim only", 554)
            text(frame.scratch == true || shot?.scratch == true ? "SCRATCH RISK · approximate" : "White: cue  Cyan: object  Green: suggested", 580, 13, .systemOrange)
            text("Uncalibrated · no game power percentage", 608, 12, .lightGray)
        }
    }
    func pictureInPictureController(_ pictureInPictureController: AVPictureInPictureController, setPlaying playing: Bool) { paused = !playing }
    func pictureInPictureControllerTimeRangeForPlayback(_ pictureInPictureController: AVPictureInPictureController) -> CMTimeRange { CMTimeRange(start: .zero, duration: .positiveInfinity) }
    func pictureInPictureControllerIsPlaybackPaused(_ pictureInPictureController: AVPictureInPictureController) -> Bool { paused }
    func pictureInPictureController(_ pictureInPictureController: AVPictureInPictureController, didTransitionToRenderSize newRenderSize: CMVideoDimensions) {}
    func pictureInPictureController(_ pictureInPictureController: AVPictureInPictureController, skipByInterval skipInterval: CMTime, completion: @escaping () -> Void) { completion() }
    func pictureInPictureController(_ pictureInPictureController: AVPictureInPictureController, failedToStartPictureInPictureWithError error: Error) { status = "PiP failed: \(error.localizedDescription)" }
    func pictureInPictureControllerDidStartPictureInPicture(_ pictureInPictureController: AVPictureInPictureController) { status = "Floating view started. Open Messages." }
    func pictureInPictureController(_ pictureInPictureController: AVPictureInPictureController, restoreUserInterfaceForPictureInPictureStopWithCompletionHandler completionHandler: @escaping (Bool) -> Void) { completionHandler(true) }
}

final class VideoView: UIView {
    var videoLayer: AVSampleBufferDisplayLayer?
    override func layoutSubviews() { super.layoutSubviews(); videoLayer?.frame = bounds }
}
struct LivePreview: UIViewRepresentable {
    let controller: LiveController
    func makeUIView(context: Context) -> VideoView {
        let view = VideoView(); view.videoLayer = controller.layer; view.layer.addSublayer(controller.layer);return view
    }
    func updateUIView(_ uiView: VideoView, context: Context) {}
}
