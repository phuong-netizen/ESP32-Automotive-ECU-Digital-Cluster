import QtQuick

Item {
    id: bar

    width: 220
    height: 66

    // Giá trị chuẩn hóa từ 0 đến 1.
    property real level: 0.72

    // true: F bên trái, E bên phải.
    // false: C bên trái, H bên phải.
    property bool fuelMode: false

    property bool warning: false
    property string caption: ""

    readonly property real boundedLevel:
        Math.max(0, Math.min(1, level))

    onBoundedLevelChanged: strip.requestPaint()
    onFuelModeChanged: strip.requestPaint()
    onWarningChanged: strip.requestPaint()

    Text {
        x: 0
        y: 0
        text: bar.fuelMode ? "F" : "C"
        color: "#E5F5FF"
        font.pixelSize: 14
        font.bold: true
    }

    Text {
        anchors.right: parent.right
        y: 0
        text: bar.fuelMode ? "E" : "H"
        color: "#E5F5FF"
        font.pixelSize: 14
        font.bold: true
    }

    Canvas {
        id: strip

        x: 10
        y: 18
        width: parent.width - 20
        height: 26

        onWidthChanged: requestPaint()
        onHeightChanged: requestPaint()

        onPaint: {
            const ctx = getContext("2d")
            ctx.reset()
            ctx.scale(width / 200, height / 26)

            // Dốc cong đổi hướng giữa hai bên.
            function upperY(x) {
                const t = bar.fuelMode
                        ? 1 - x / 200
                        : x / 200

                return 13 - 10 * t * t
            }

            function lowerY(x) {
                const t = bar.fuelMode
                        ? 1 - x / 200
                        : x / 200

                return upperY(x) + 6 + 5 * t
            }

            // Nền của toàn bộ thanh.
            ctx.beginPath()
            ctx.moveTo(0, upperY(0) - 2)

            for (let x = 4; x <= 200; x += 4)
                ctx.lineTo(x, upperY(x) - 2)

            for (let x = 200; x >= 0; x -= 4)
                ctx.lineTo(x, lowerY(x) + 2)

            ctx.closePath()
            ctx.fillStyle = "#0D202D"
            ctx.fill()

            // Dải màu sáng cho từng đốt.
            const light = ctx.createLinearGradient(0, 0, 0, 26)
            light.addColorStop(0, "#FAFFFF")
            light.addColorStop(0.55, "#C6F6FF")
            light.addColorStop(1, "#57C9EA")

            const count = 12
            const pitch = 200 / count
            const activeCount = Math.round(
                bar.boundedLevel * count)

            for (let i = 0; i < count; ++i) {
                const left = i * pitch + 1.5
                const right = (i + 1) * pitch - 1.5

                // Nhiên liệu tăng từ E về F.
                // Nhiệt độ tăng từ C về H.
                const active = bar.fuelMode
                        ? i >= count - activeCount
                        : i < activeCount

                // Đốt hình tứ giác nghiêng.
                ctx.beginPath()
                ctx.moveTo(left, upperY(left))
                ctx.lineTo(right - 3, upperY(right))
                ctx.lineTo(right, lowerY(right))
                ctx.lineTo(left + 3, lowerY(left))
                ctx.closePath()

                if (active) {
                    ctx.fillStyle = bar.warning
                            ? "#FF845C"
                            : light
                } else {
                    ctx.fillStyle = "#18303F"
                }

                ctx.fill()
            }

            // Đường sáng mảnh dưới chân thanh.
            ctx.beginPath()
            ctx.moveTo(0, lowerY(0) + 3)

            for (let x = 4; x <= 200; x += 4)
                ctx.lineTo(x, lowerY(x) + 3)

            ctx.lineWidth = 0.8
            ctx.strokeStyle = "#6597AA"
            ctx.stroke()
        }
    }

    Text {
        anchors.horizontalCenter: parent.horizontalCenter
        y: 48

        Canvas {
            id: statusIcon
            parent: bar

            width: 26
            height: 26

            x: bar.fuelMode
               ? strip.x
               : strip.x + strip.width - width

            y: strip.y + strip.height + 2
            Connections {
                target: bar

                function onFuelModeChanged() {
                    statusIcon.requestPaint()
                }

                function onWarningChanged() {
                    statusIcon.requestPaint()
                }
            }

            onPaint: {
                const ctx = getContext("2d")
                ctx.reset()

                const iconColor = bar.warning ? "#FF845C" : "#D8EDF5"

                ctx.strokeStyle = iconColor
                ctx.fillStyle = iconColor
                ctx.lineWidth = 1.7
                ctx.lineCap = "round"
                ctx.lineJoin = "round"

                if (bar.fuelMode) {
                    // Thân cây xăng.
                    ctx.strokeRect(4, 4, 11, 17)

                    // Ô màn hình.
                    ctx.strokeRect(6.5, 6.5, 6, 5)

                    // Chân đế.
                    ctx.beginPath()
                    ctx.moveTo(2, 22)
                    ctx.lineTo(17, 22)
                    ctx.stroke()

                    // Vòi và dây bơm.
                    ctx.beginPath()
                    ctx.moveTo(15, 12)
                    ctx.lineTo(18, 12)
                    ctx.lineTo(18, 18)
                    ctx.bezierCurveTo(18, 22, 23, 22, 23, 18)
                    ctx.lineTo(23, 9)
                    ctx.lineTo(19, 5)
                    ctx.stroke()

                    // Đầu vòi.
                    ctx.beginPath()
                    ctx.moveTo(20, 6)
                    ctx.lineTo(20, 10)
                    ctx.lineTo(23, 11)
                    ctx.stroke()
                } else {
                    // Ống nhiệt kế.
                    ctx.beginPath()
                    ctx.moveTo(10, 15)
                    ctx.lineTo(10, 4)
                    ctx.bezierCurveTo(10, 1, 14, 1, 14, 4)
                    ctx.lineTo(14, 15)
                    ctx.stroke()

                    // Bầu nhiệt kế.
                    ctx.beginPath()
                    ctx.arc(12, 17, 3.5, 0, Math.PI * 2)
                    ctx.fill()

                    // Cột nhiệt.
                    ctx.beginPath()
                    ctx.moveTo(12, 7)
                    ctx.lineTo(12, 16)
                    ctx.stroke()

                    // Vạch chia.
                    for (let i = 0; i < 3; ++i) {
                        const tickY = 5 + i * 4
                        ctx.beginPath()
                        ctx.moveTo(15, tickY)
                        ctx.lineTo(18, tickY)
                        ctx.stroke()
                    }

                    // Hai hàng sóng nước.
                    for (let row = 0; row < 2; ++row) {
                        const waveY = 21 + row * 3

                        ctx.beginPath()
                        ctx.moveTo(2, waveY)

                        for (let j = 0; j < 3; ++j) {
                            const waveX = 2 + j * 7

                            ctx.bezierCurveTo(
                                waveX + 2, waveY - 2,
                                waveX + 5, waveY + 2,
                                waveX + 7, waveY
                            )
                        }

                        ctx.stroke()
                    }
                }
            }
        }
    }
}