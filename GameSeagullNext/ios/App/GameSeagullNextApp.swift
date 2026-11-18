import SwiftUI
import ReplayKit

@main struct GameSeagullNextApp: App {
    @StateObject private var live = LiveController()
    var body: some Scene {
        WindowGroup {
            NavigationStack {
                ScrollView {
                    VStack(alignment: .leading, spacing: 18) {
                        Text("Live table assistant").font(.largeTitle.bold())
                        Text("Experimental floating view. It does not draw on GamePigeon itself.").foregroundStyle(.secondary)
                        LivePreview(controller: live).frame(height: 300).background(.black).clipShape(RoundedRectangle(cornerRadius: 16))
                        Text(live.status).font(.callout).accessibilityLabel("Status: \(live.status)")
                        Button(live.running ? "Stop receiver" : "1. Start receiver") { live.running ? live.stop() : live.start() }.buttonStyle(.borderedProminent)
                        HStack { Text("2. Start screen broadcast"); BroadcastButton().frame(width: 56, height: 56) }
                        Button("3. Open floating view, then Messages") { live.floatWindow() }.buttonStyle(.bordered).disabled(!live.pipPossible || !live.running)
                        Text("Screen broadcasting can capture anything visible, including messages. Processing stays on this iPhone; audio is discarded. Stop broadcasting in Control Center when finished.").font(.footnote).foregroundStyle(.secondary)
                        Divider()
                        Toggle("Enable assistance", isOn: $live.config.enabled)
                        Toggle("Recommended shot", isOn: $live.assisted)
                        Toggle("Banks and combinations", isOn: $live.config.advanced)
                        Toggle("Bright guidelines", isOn: $live.brightLines)
                        Picker("My group", selection: $live.config.group) { Text("Open / unknown").tag(0); Text("Solids").tag(2); Text("Stripes").tag(3) }
                        Toggle("I may legally target the 8 ball", isOn: $live.config.eightAllowed)
                        Picker("Analysis rate", selection: $live.config.fps) { Text("Battery · 4 fps").tag(4); Text("Balanced · 8 fps").tag(8); Text("Fast · 12 fps").tag(12) }
                        Text("Predictions use geometry without spin or measured GamePigeon physics. Classification and shot legality need your judgment. No win guarantee.").font(.footnote).foregroundStyle(.secondary)
                    }.padding(22)
                }.preferredColorScheme(.dark)
            }
        }
    }
}
struct BroadcastButton: UIViewRepresentable {
    func makeUIView(context: Context) -> RPSystemBroadcastPickerView {
        let picker = RPSystemBroadcastPickerView(frame: CGRect(x:0,y:0,width:56,height:56))
        picker.preferredExtension = (Bundle.main.bundleIdentifier ?? "org.seagull.next") + ".broadcast"
        picker.showsMicrophoneButton = false
        return picker
    }
    func updateUIView(_ uiView: RPSystemBroadcastPickerView, context: Context) {}
}
