import QtQuick
import QtQuick.Controls
ApplicationWindow {
    id: window
    // Chua co du lieu km/h.
    // Quy đổi mô phỏng: bánh xe 0.6 m, tỉ số truyền 1:1.
    readonly property real canSpeed:
        canRpm * Math.PI * 0.6 * 60 / 1000

    property real canRpm: 0
    property int canThrottle: 0
    property int canDirection: 1

    property real canFuel: 0
    property real canCoolant: 0

    property int canTurnSignal: 0
    property int canLampMode: 0

    property bool canMotionValid: false
    property bool canSensorsValid: false
    property bool canLightsValid: false

    readonly property bool canAllValid:
        canMotionValid && canSensorsValid && canLightsValid
    
    Button {
        anchors.top: parent.top
        anchors.right: parent.right
        anchors.margins: 12
        text: "Thoát"
        z: 1000
        onClicked: Qt.quit()
    }

    width: 1024
    height: 600
    minimumWidth: 1024
    minimumHeight: 600

    visible: true
    visibility: Window.FullScreen
    title: "ECU Digital Cluster"
    color: "#020407"

    // Vùng thiết kế theo kích thước màn HMI.
    // Màn hình chào phủ lên giao diện chính.
    Rectangle {
        id: welcomeScreen
        anchors.fill: parent
        color: "#020407"
        z: 900

        property bool ready: false
        property bool leaving: false

        Item {
            id: welcomeContent
            width: 1024
            height: 600
            anchors.centerIn: parent
            scale: Math.min(welcomeScreen.width / width,
                            welcomeScreen.height / height)

            // Các lớp viền xanh hai bên.
            Item {
                id: welcomeWings
                anchors.fill: parent
                opacity: 0

                // Cánh trái: viền ngoài + 4 dải phía trong.
                Repeater {
                    model: 5

                    delegate: ClusterWing {
                        required property int index

                        x: 40 + index * 24
                        y: 125 + index * 24

                        width: 340 - index * 24
                        height: 340 - index * 48

                        opacity: [1.0, 0.75, 0.55, 0.38, 0.25][index]
                    }
                }

                // Cánh phải đối xứng với cánh trái.
                Repeater {
                    model: 5

                    delegate: ClusterWing {
                        required property int index

                        x: 644
                        y: 125 + index * 24

                        width: 340 - index * 24
                        height: 340 - index * 48

                        mirrored: true
                        opacity: [1.0, 0.75, 0.55, 0.38, 0.25][index]
                    }
                }
            }
            // Logo chữ của đồ án.
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 142

                text: "KP"
                color: "#E1ECF5"
                font.family: "Orbitron"
                font.pixelSize: 58
                font.weight: Font.DemiBold
                font.italic: false
                font.letterSpacing: 6

                opacity: welcomeWings.opacity
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 213

                text: "DIGITAL CLUSTER"
                color: "#000000"
                font.family: "Orbitron"
                font.pixelSize: 11
                font.weight: Font.DemiBold
                font.italic: false
                font.letterSpacing: 4

                opacity: welcomeWings.opacity
            }
            Text {
                id: welcomeTitle
                anchors.horizontalCenter: parent.horizontalCenter
                y: 260
                text: "WELCOME"
                color: "#F0FAFF"
                font.pixelSize: 18
                font.bold: true
                font.letterSpacing: 5
                opacity: 0
            }

            Text {
                id: welcomeSubtitle
                anchors.horizontalCenter: parent.horizontalCenter
                y: 314
                text: "Ready to Drive"
                color: "#00b3ff"
                font.bold: true
                font.pixelSize: 13
                font.letterSpacing: 1
                opacity: 0
            }
            // Đường sáng mở rộng từ giữa ra hai phía.
            Item {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 363
                width: 260
                height: 12

                // Ánh sáng mờ bao quanh.
                Rectangle {
                    anchors.centerIn: parent
                    width: welcomeLine.width
                    height: 8
                    radius: 4
                    color: "#159EDF"
                    opacity: 0.12
                }

                Rectangle {
                    id: welcomeLine
                    anchors.centerIn: parent
                    width: 0
                    height: 2
                    radius: 1

                    gradient: Gradient {
                        orientation: Gradient.Horizontal

                        GradientStop {
                            position: 0.0
                            color: "#002ACBFF"
                        }
                        GradientStop {
                            position: 0.5
                            color: "#A6F3FF"
                        }
                        GradientStop {
                            position: 1.0
                            color: "#002ACBFF"
                        }
                    }
                }

                NumberAnimation {
                    target: welcomeLine
                    property: "width"
                    from: 0
                    to: 260
                    duration: 800
                    easing.type: Easing.OutCubic

                    running: welcomeScreen.ready
                }
            }
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 490
                text: "CHẠM MÀN HÌNH ĐỂ BẮT ĐẦU"
                color: "#7798AB"
                font.pixelSize: 13
                font.letterSpacing: 2
                opacity: welcomeScreen.ready ? 1 : 0

                Behavior on opacity {
                    NumberAnimation { duration: 500 }
                }
            }
        }

        // Hiện lần lượt viền, tiêu đề rồi dòng phụ.
        SequentialAnimation {
            running: true

            PauseAnimation { duration: 200 }

            NumberAnimation {
                target: welcomeWings
                property: "opacity"
                from: 0
                to: 1
                duration: 900
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target: welcomeTitle
                property: "opacity"
                from: 0
                to: 1
                duration: 900
                easing.type: Easing.InOutQuad
            }

            NumberAnimation {
                target: welcomeSubtitle
                property: "opacity"
                from: 0
                to: 1
                duration: 700
                easing.type: Easing.InOutQuad
            }

            ScriptAction {
                script: welcomeScreen.ready = true
            }
        }

        // Nhận cả thao tác chạm và click chuột.
        MouseArea {
            anchors.fill: parent

            onClicked: {
                if (welcomeScreen.ready && !welcomeScreen.leaving) {
                    welcomeScreen.leaving = true
                    welcomeFadeOut.start()
                }
            }
        }

        NumberAnimation {
            id: welcomeFadeOut
            target: welcomeScreen
            property: "opacity"
            to: 0
            duration: 600
            easing.type: Easing.InOutQuad

            onFinished: welcomeScreen.visible = false
        }
    }

    Item {
        id: dashboard
        property real fuelLevel: window.canFuel
        property real coolantTemp: window.canCoolant

        property bool leftSignal:
            window.canLightsValid && window.canTurnSignal === 2

        property bool rightSignal:
            window.canLightsValid && window.canTurnSignal === 1

        property bool blinkOn: true

        property int lightMode: window.canLampMode

        // Chua co tin hieu CAN cho ba muc nay.
        property bool parkingBrake: false
        property bool seatBeltWarning: false
        property string gearPosition: "--"

        // --- BẮT ĐẦU PHẦN THÊM BIẾN VÀ KỊCH BẢN QUÉT KIM ---
        property bool isSweeping: false
        property real sweepSpeed: 0
        property real sweepRpm: 0
        property real sweepFuel: 0
        property real sweepCoolant: 20

        SequentialAnimation {
            id: startupSweepAnim
            
            ScriptAction { script: dashboard.isSweeping = true }
            
            // Quét lên Max
            ParallelAnimation {
                NumberAnimation { target: dashboard; property: "sweepSpeed"; to: 180; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepRpm"; to: 8000; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepFuel"; to: 100; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepCoolant"; to: 120; duration: 700; easing.type: Easing.InOutCubic }
            }
            
            // Quét ngược về 0
            ParallelAnimation {
                NumberAnimation { target: dashboard; property: "sweepSpeed"; to: 0; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepRpm"; to: 0; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepFuel"; to: 0; duration: 700; easing.type: Easing.InOutCubic }
                NumberAnimation { target: dashboard; property: "sweepCoolant"; to: 20; duration: 700; easing.type: Easing.InOutCubic }
            }
            
            ScriptAction { script: dashboard.isSweeping = false }
        }

        Connections {
            target: welcomeScreen
            function onLeavingChanged() {
                if (welcomeScreen.leaving) {
                    startupSweepAnim.start()
                }
            }
        }
        // --- KẾT THÚC PHẦN THÊM ---

        width: 1024
        height: 600
        anchors.centerIn: parent

        scale: Math.min(window.width / width,
                        window.height / height)

        // Nền cụm đồng hồ.
        Rectangle {
            anchors.fill: parent
            anchors.margins: 24

            radius: 28
            color: "#03080E"
            border.color: "#14212C"
            border.width: 1
        }

        Text {
            x: 48
            y: 42

            text: "ECU DIGITAL CLUSTER"
            color: "#A7C8DC"
            font.pixelSize: 12
            font.letterSpacing: 2
        }

        Item {
            id: speedGauge

            x: 60
            y: 145
            width: 340
            height: 340

            // --- ĐÃ SỬA CHỖ NÀY ĐỂ KÉO THEO HIỆU ỨNG ---
            property real speed: dashboard.isSweeping ? dashboard.sweepSpeed : window.canSpeed
            property real maximumSpeed: 180

            // Đổi từ độ sang radian để vẽ vòng tròn.
            function radians(degrees) {
                return degrees * Math.PI / 180
            }
            ClusterWing {
                anchors.fill: parent
            }
            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -5

                // --- ĐÃ SỬA CHỖ NÀY ---
                text: dashboard.isSweeping 
                      ? Math.round(speedGauge.speed).toString()
                      : (window.canMotionValid ? Math.round(speedGauge.speed).toString() : "--")
                
                color: "#F0F7FF"
                font.pixelSize: 76
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 212

                text: "km/h"
                color: "#8BA8C2"
                font.pixelSize: 18
            }
        }
        
        Item {
            id: rpmGauge

            x: 624
            y: 145
            width: 340
            height: 340

            // --- ĐÃ SỬA CHỖ NÀY ĐỂ KÉO THEO HIỆU ỨNG ---
            property real rpm: dashboard.isSweeping ? dashboard.sweepRpm : window.canRpm
            property real maximumRpm: 8000

            // Đổi từ độ sang radian để vẽ vòng tròn.
            function radians(degrees) {
                return degrees * Math.PI / 180
            }

            ClusterWing {
                anchors.fill: parent
                mirrored: true
            }

            Text {
                anchors.centerIn: parent
                anchors.verticalCenterOffset: -5

                // --- ĐÃ SỬA CHỖ NÀY ---
                text: dashboard.isSweeping 
                      ? Math.round(rpmGauge.rpm).toString()
                      : (window.canMotionValid ? Math.round(rpmGauge.rpm).toString() : "--")
                
                color: "#F0F7FF"
                font.pixelSize: 76
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 212

                text: "RPM TRỤC RA"
                color: "#8BA8C2"
                font.pixelSize: 18
            }
        }
        
        Item {
            id: infoPanel

            x: 412
            y: 170
            width: 200
            height: 320

            // Thời gian của máy đang chạy ứng dụng.
            property string clockText: Qt.formatTime(new Date(), "HH:mm")

            Timer {
                interval: 1000
                running: true
                repeat: true

                onTriggered: {
                    infoPanel.clockText =
                            Qt.formatTime(new Date(), "HH:mm")
                }
            }

            // Nền hòa vào nền cụm đồng hồ, không có khung viền.
            Rectangle {
                anchors.fill: parent
                color: "#03080E"
            }

            // Chân trời và mặt đường phối cảnh.
            Canvas {
                anchors.fill: parent

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()
                    ctx.scale(width / 200, height / 320)

                    // Quầng xanh tại chân trời.
                    const horizon = ctx.createLinearGradient(
                        0, 82, 0, 150)

                    horizon.addColorStop(0.00, "#03080E")
                    horizon.addColorStop(0.45, "#072153")
                    horizon.addColorStop(0.80, "#075EB0")
                    horizon.addColorStop(1.00, "#03080E")

                    ctx.fillStyle = horizon
                    ctx.fillRect(0, 82, 200, 68)

                    // Mặt đường mở rộng về phía người nhìn.
                    const road = ctx.createLinearGradient(
                        0, 132, 0, 300)

                    road.addColorStop(0.00, "#123A57")
                    road.addColorStop(0.40, "#0C3552")
                    road.addColorStop(0.75, "#071D2E")
                    road.addColorStop(1.00, "#03080E")

                    ctx.beginPath()
                    ctx.moveTo(96, 132)
                    ctx.lineTo(104, 132)
                    ctx.lineTo(200, 254)
                    ctx.lineTo(200, 300)
                    ctx.lineTo(0, 300)
                    ctx.lineTo(0, 254)
                    ctx.closePath()

                    ctx.fillStyle = road
                    ctx.fill()

                    // Hai mép đường sáng dần về chân trời.
                    const edge = ctx.createLinearGradient(
                        0, 132, 0, 280)

                    edge.addColorStop(0.00, "#5DDCFF")
                    edge.addColorStop(0.35, "#1674AB")
                    edge.addColorStop(1.00, "#03080E")

                    ctx.strokeStyle = edge
                    ctx.lineWidth = 1.2

                    ctx.beginPath()
                    ctx.moveTo(96, 132)
                    ctx.lineTo(0, 254)

                    ctx.moveTo(104, 132)
                    ctx.lineTo(200, 254)
                    ctx.stroke()
                }
            }
            // Đèn phanh đỗ: vòng tròn P và hai cung bên ngoài.
            Item {
                x: 8
                y: 3
                width: 28
                height: 26

                opacity: dashboard.parkingBrake ? 1.0 : 0.15

                Canvas {
                    anchors.fill: parent

                    onPaint: {
                        const ctx = getContext("2d")
                        ctx.reset()

                        ctx.strokeStyle = "#FF4048"
                        ctx.lineWidth = 1.7
                        ctx.lineCap = "round"

                        // Vòng tròn giữa.
                        ctx.beginPath()
                        ctx.arc(14, 13, 8, 0, Math.PI * 2)
                        ctx.stroke()

                        // Hai cung ngoài.
                        ctx.beginPath()
                        ctx.arc(14, 13, 12,
                                Math.PI * 0.75, Math.PI * 1.25)
                        ctx.stroke()

                        ctx.beginPath()
                        ctx.arc(14, 13, 12,
                                -Math.PI * 0.25, Math.PI * 0.25)
                        ctx.stroke()
                    }
                }

                Text {
                    anchors.centerIn: parent
                    text: "P"
                    color: "#FF4048"
                    font.pixelSize: 13
                    font.bold: true
                }
            }

            // Cảnh báo dây an toàn.
            Canvas {
                x: 10
                y: 244
                width: 28
                height: 30

                opacity: dashboard.seatBeltWarning ? 1.0 : 0.15

                onPaint: {
                    const ctx = getContext("2d")
                    ctx.reset()

                    ctx.fillStyle = "#FF4048"
                    ctx.strokeStyle = "#FF4048"
                    ctx.lineWidth = 2.5
                    ctx.lineCap = "round"
                    ctx.lineJoin = "round"

                    // Đầu.
                    ctx.beginPath()
                    ctx.arc(14, 5, 3, 0, Math.PI * 2)
                    ctx.fill()

                    // Vai và thân.
                    ctx.beginPath()
                    ctx.moveTo(8, 19)
                    ctx.lineTo(9, 12)
                    ctx.quadraticCurveTo(14, 8, 19, 12)
                    ctx.lineTo(20, 19)
                    ctx.stroke()

                    // Hai chân.
                    ctx.beginPath()
                    ctx.moveTo(11, 20)
                    ctx.lineTo(10, 28)
                    ctx.moveTo(17, 20)
                    ctx.lineTo(18, 28)
                    ctx.stroke()

                    // Dây chéo qua người.
                    ctx.strokeStyle = "#FF8A8F"
                    ctx.lineWidth = 2

                    ctx.beginPath()
                    ctx.moveTo(20, 10)
                    ctx.lineTo(9, 22)
                    ctx.lineTo(20, 22)
                    ctx.stroke()
                }
            }
            // Danh sách số và số đang chọn.
            Item {
                x: 145
                y: 229
                width: 49
                height: 48

                Column {
                    spacing: 1

                    Repeater {
                        model: ["P", "R", "N", "D"]

                        delegate: Text {
                            required property string modelData

                            text: modelData
                            font.pixelSize: 10
                            font.bold: dashboard.gearPosition === modelData

                            color: dashboard.gearPosition === modelData
                                   ? "#F0FAFF" : "#526878"
                        }
                    }
                }

                Text {
                    anchors.right: parent.right
                    anchors.verticalCenter: parent.verticalCenter

                    text: dashboard.gearPosition
                    color: "#F0FAFF"
                    font.pixelSize: 29
                    font.bold: true
                }
            }
            // Góc trên trái: nhiệt độ nước làm mát.
            Text {
                x: 8
                y: 32

                text: "NƯỚC"
                color: "#849BAA"
                font.pixelSize: 9
            }

            Text {
                x: 8
                y: 44

                text: window.canSensorsValid
                  ? Math.round(dashboard.coolantTemp) + " °C"
                  : "-- °C"
                color: dashboard.coolantTemp >= 105
                       ? "#FF755F" : "#E1F2FF"
                font.pixelSize: 14
            }

            // Góc trên phải: đồng hồ.
            Text {
                anchors.right: parent.right
                anchors.rightMargin: 8
                y: 15

                text: infoPanel.clockText
                color: "#E1F2FF"
                font.pixelSize: 15
            }

            // Tốc độ nhỏ ở đầu màn hình trung tâm.
            Row {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 45
                spacing: 6

                Text {
                    text: window.canMotionValid
                          ? Math.round(window.canSpeed).toString()
                          : "--"
                    color: "#F0F8FF"
                    font.pixelSize: 36
                    font.italic: true
                }

                Text {
                    anchors.bottom: parent.bottom
                    anchors.bottomMargin: 7

                    text: "km/h (mô phỏng)"
                    color: "#B4CBDC"
                    font.pixelSize: 12
                }
            }

            Rectangle {
                x: 15
                y: 91
                width: 170
                height: 1
                color: "#688897"
            }

            // Nội dung giữa màn hình.
            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 158

                text: "LỊCH BẢO DƯỠNG"
                color: "#E1F2FF"
                font.pixelSize: 12
                font.bold: true
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 191

                text: "Chưa có dữ liệu"
                color: "#B7D0E2"
                font.pixelSize: 13
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 215

                text: "Cài đặt quãng đường / thời gian"
                color: "#8DAABD"
                font.pixelSize: 10
            }

            // Trạng thái nguồn dữ liệu.
            Rectangle {
                x: 8
                y: 280
                width: 184
                height: 1
                color: "#183547"
            }

            Text {
                anchors.horizontalCenter: parent.horizontalCenter
                y: 293

                text: window.canAllValid
                  ? "CAN ĐANG NHẬN"
                  : "CAN THIẾU / MẤT DỮ LIỆU"

                color: window.canAllValid ? "#48EB98" : "#FF755F"
                font.pixelSize: 10
                font.letterSpacing: 1
            }
        }
        
        SegmentBar {
            x: 110
            y: 443
            width: 170
            height: 66

            fuelMode: true
            
            // --- ĐÃ SỬA CHỖ NÀY ĐỂ MỨC XĂNG QUÉT THEO HIỆU ỨNG ---
            level: dashboard.isSweeping 
                   ? Math.max(0, Math.min(1, dashboard.sweepFuel / 100))
                   : (window.canSensorsValid ? Math.max(0, Math.min(1, dashboard.fuelLevel / 100)) : 0)
                   
            opacity: (dashboard.isSweeping || window.canSensorsValid) ? 1.0 : 0.25
            warning: !dashboard.isSweeping && window.canSensorsValid && dashboard.fuelLevel < 15
        }

        // Thanh nhiệt độ nước làm mát dưới đồng hồ vòng tua.
        SegmentBar {
            x: 754
            y: 443
            width: 170
            height: 66

            fuelMode: false

            // Thang mô phỏng: C = 40 °C, H = 120 °C.
            // --- ĐÃ SỬA CHỖ NÀY ĐỂ MỨC NHIỆT ĐỘ QUÉT THEO HIỆU ỨNG ---
            level: dashboard.isSweeping
                   ? Math.max(0, Math.min(1, (dashboard.sweepCoolant - 20) / 100))
                   : (window.canSensorsValid ? Math.max(0, Math.min(1, (dashboard.coolantTemp - 20) / 100)) : 0)

            opacity: (dashboard.isSweeping || window.canSensorsValid) ? 1.0 : 0.25
            warning: !dashboard.isSweeping && window.canSensorsValid && dashboard.coolantTemp >= 105
        }

        // Nhịp nhấp nháy chung cho hai xi-nhan.
        Timer {
            interval: 500
            repeat: true
            running: dashboard.leftSignal || dashboard.rightSignal

            onTriggered: dashboard.blinkOn = !dashboard.blinkOn

            // Bật sáng ngay khi bắt đầu; trả nhịp về ban đầu khi tắt.
            onRunningChanged: dashboard.blinkOn = true
        }

        // Xi-nhan trái, phía trên đồng hồ tốc độ.
        Text {
            x: 200
            y: 112
            width: 60

            text: "\u2190"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 40
            font.bold: true

            color: dashboard.leftSignal && dashboard.blinkOn
                   ? "#48EB98" : "#263C45"
        }

        // Xi-nhan phải, phía trên đồng hồ vòng tua.
        Text {
            x: 764
            y: 112
            width: 60

            text: "\u2192"
            horizontalAlignment: Text.AlignHCenter
            font.pixelSize: 40
            font.bold: true

            color: dashboard.rightSignal && dashboard.blinkOn
                   ? "#48EB98" : "#263C45"
        }

        // Trạng thái đèn, phía trên khung thông tin.
        Rectangle {
            x: 442
            y: 127
            width: 140
            height: 34
            radius: 8

            color: "#111E2E"
            border.color: dashboard.lightMode === 2 ? "#35B8FF"
                          : dashboard.lightMode === 1 ? "#48EB98"
                          : "#28415B"
            opacity: window.canLightsValid ? 1.0 : 0.3
            Text {
                anchors.centerIn: parent

                text: !window.canLightsValid ? "ĐÈN: --"
                      : dashboard.lightMode === 2 ? "ĐÈN PHA"
                      : dashboard.lightMode === 1 ? "ĐÈN CỐT"
                      : "ĐÈN TẮT"

                color: dashboard.lightMode === 2 ? "#35B8FF"
                       : dashboard.lightMode === 1 ? "#48EB98"
                       : "#71869B"

                font.pixelSize: 14
                font.bold: true
            }
        }

        // Phím thử nghiệm trên máy tính.
        Text {
            anchors.bottom: parent.bottom
            anchors.bottomMargin: 46
            anchors.horizontalCenter: parent.horizontalCenter

            text: "RPM: " + (window.canMotionValid ? "OK" : "--")
                  + "  •  CẢM BIẾN: " + (window.canSensorsValid ? "OK" : "--")
                  + "  •  ĐÈN: " + (window.canLightsValid ? "OK" : "--")
        }
    }
}