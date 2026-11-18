import Foundation
import Network

// Loopback geometry only. No screen images, cloud server, App Group or LAN listener.
// Prototype IPC: same-device third-party processes are not authenticated. Do not send private content.
final class FramedConnection {
    let connection: NWConnection
    var onMessage: ((Data) -> Void)?
    var onClose: (() -> Void)?
    private var buffered = Data()
    private var sending = false
    private var ready = false
    init(_ connection: NWConnection) { self.connection = connection }
    func start(queue: DispatchQueue) {
        connection.stateUpdateHandler = { [weak self] state in
            guard let self = self else { return }
            switch state {
            case .ready: self.ready = true; self.receive()
            case .failed, .cancelled: self.ready = false; self.onClose?()
            default: break
            }
        }
        connection.start(queue: queue)
    }
    func send(_ data: Data) {
        guard ready, !sending, data.count <= 65536 else { return }
        sending = true
        let n = UInt32(data.count)
        var packet = Data([UInt8((n >> 24) & 255), UInt8((n >> 16) & 255), UInt8((n >> 8) & 255), UInt8(n & 255)])
        packet.append(data)
        connection.send(content: packet, completion: .contentProcessed { [weak self] error in
            self?.sending = false
            if error != nil { self?.connection.cancel() }
        })
    }
    private func receive() {
        connection.receive(minimumIncompleteLength: 1, maximumLength: 65540) { [weak self] data, _, done, error in
            guard let self = self else { return }
            if let data = data { self.buffered.append(data) }
            while self.buffered.count >= 4 {
                let header = [UInt8](self.buffered.prefix(4))
                let n = Int(header.reduce(UInt32(0)) { ($0 << 8) | UInt32($1) })
                guard n > 0 && n <= 65536 else { self.connection.cancel(); return }
                if self.buffered.count < n + 4 { break }
                let payload = Data(self.buffered.dropFirst(4).prefix(n))
                self.buffered.removeFirst(n + 4)
                self.onMessage?(payload)
            }
            if done || error != nil { self.connection.cancel() }
            else { self.receive() }
        }
    }
    func cancel() { onClose = nil; connection.cancel() }
}
struct Configuration: Codable {
    var enabled = true
    var advanced = true
    var group = 0
    var eightAllowed = false
    var fps = 8
}
let seagullPort: NWEndpoint.Port = 49382
