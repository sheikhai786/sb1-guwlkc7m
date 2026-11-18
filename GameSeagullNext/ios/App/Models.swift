import Foundation
struct BallView: Decodable { let p: [Double]; let r: Double; let kind: Int }
struct LineView: Decodable { let a: [Double]; let b: [Double]; let role: Int }
struct ShotView: Decodable { let type: String; let score: Double; let effort: Double; let scratch: Bool; let lines: [LineView] }
struct Frame: Decodable {
    let valid: Bool
    let reason: String
    let table: [Double]?
    let pockets: [[Double]]?
    let balls: [BallView]?
    let prediction: [LineView]?
    let shots: [ShotView]?
    let capturedAt: Double?
    let analysisMS: Double?
    let aimConfidence: Double?
    let scratch: Bool?
}
