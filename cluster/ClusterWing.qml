import QtQuick

Canvas {
    id: wing

    width: 340
    height: 340

    property bool mirrored: false

    onWidthChanged: requestPaint()
    onHeightChanged: requestPaint()
    onMirroredChanged: requestPaint()

    onPaint: {
        const ctx = getContext("2d")
        ctx.reset()
        ctx.scale(width / 340, height / 340)

        if (mirrored) {
            ctx.translate(340, 0)
            ctx.scale(-1, 1)
        }

        // Đường ngoài đi từ đầu trên đến đầu dưới.
        // Đường trong quay ngược lại, tạo thành một dải kín.
        function ribbon() {
            ctx.beginPath()

            ctx.moveTo(330, 48)

            // Mép ngoài phía trên.
            ctx.bezierCurveTo(236, 44, 145, 47, 101, 75)

            // Mép ngoài bên trái.
            ctx.bezierCurveTo(62, 100, 34, 157, 28, 208)
            ctx.bezierCurveTo(22, 245, 36, 261, 79, 273)

            // Mép ngoài phía dưới, thu về đầu nhọn.
            ctx.bezierCurveTo(149, 293, 254, 298, 330, 284)

            // Mép trong phía dưới.
            ctx.bezierCurveTo(241, 288, 149, 284, 84, 263)

            // Mép trong bên trái.
            ctx.bezierCurveTo(49, 252, 38, 241, 43, 209)
            ctx.bezierCurveTo(49, 162, 72, 108, 108, 84)

            // Mép trong phía trên, trở về đầu nhọn.
            ctx.bezierCurveTo(153, 56, 241, 50, 330, 48)

            ctx.closePath()
        }

        const blue = ctx.createLinearGradient(0, 48, 0, 292)
        blue.addColorStop(0.00, "#0753DA")
        blue.addColorStop(0.24, "#009EF5")
        blue.addColorStop(0.55, "#66EDFF")
        blue.addColorStop(0.72, "#B6FFFF")
        blue.addColorStop(0.87, "#13BCF8")
        blue.addColorStop(1.00, "#0762E8")

        ribbon()
        ctx.fillStyle = blue
        ctx.fill()

        // Chỉ xóa khe bên trong dải xanh.
        // Vì vậy không còn vạch đen chìa ra hai bên.
        ctx.save()
        ribbon()
        ctx.clip()

        ctx.globalCompositeOperation = "destination-out"
        ctx.strokeStyle = "#000000"
        ctx.lineWidth = 2.5
        ctx.lineCap = "butt"

        ctx.beginPath()

        // Khe gần đầu trên.
        ctx.moveTo(204, 39)
        ctx.lineTo(219, 63)

        // Khe tại góc cong phía trên.
        ctx.moveTo(86, 67)
        ctx.lineTo(119, 93)

        // Khe ở cạnh ngoài.
        ctx.moveTo(20, 184)
        ctx.lineTo(56, 185)

        // Khe phía dưới.
        ctx.moveTo(126, 267)
        ctx.lineTo(107, 293)

        ctx.stroke()
        ctx.restore()
    }
}