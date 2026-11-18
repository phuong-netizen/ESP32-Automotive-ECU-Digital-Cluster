#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QSocketNotifier>
#include <QVariant>
#include <QDebug>
#include <QTimer>
#include <QElapsedTimer>

#include <linux/can.h>
#include <linux/can/raw.h>
#include <net/if.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    const int canSocket = socket(
        PF_CAN, SOCK_RAW | SOCK_NONBLOCK | SOCK_CLOEXEC, CAN_RAW);

    if (canSocket < 0) {
        qCritical() << "Khong mo duoc CAN:" << strerror(errno);
        return 1;
    }

    const unsigned int interfaceIndex = if_nametoindex("can0");

    if (interfaceIndex == 0) {
        qCritical() << "Khong tim thay can0.";
        close(canSocket);
        return 1;
    }

    // Chi nhan ba ID chuan, khong nhan RTR.
    can_filter filters[3]{};

    for (int i = 0; i < 3; ++i) {
        filters[i].can_id = 0x100 + i;
        filters[i].can_mask =
            CAN_SFF_MASK | CAN_EFF_FLAG | CAN_RTR_FLAG;
    }

    if (setsockopt(canSocket, SOL_CAN_RAW, CAN_RAW_FILTER,
                   filters, sizeof(filters)) < 0) {
        qCritical() << "Loi bo loc CAN:" << strerror(errno);
        close(canSocket);
        return 1;
    }

    sockaddr_can address{};
    address.can_family = AF_CAN;
    address.can_ifindex = static_cast<int>(interfaceIndex);

    if (bind(canSocket,
             reinterpret_cast<sockaddr *>(&address),
             sizeof(address)) < 0) {
        qCritical() << "Khong bind duoc can0:" << strerror(errno);
        close(canSocket);
        return 1;
    }

    QQmlApplicationEngine engine;
    engine.loadFromModule("ECU_DigitalCluster", "Main");

    if (engine.rootObjects().isEmpty()) {
        close(canSocket);
        return 1;
    }

    QObject *window = engine.rootObjects().first();

    const char *properties[] = {
        "canRpm", "canThrottle", "canDirection",
        "canFuel", "canCoolant", "canTurnSignal", "canLampMode",
        "canMotionValid", "canSensorsValid", "canLightsValid"
    };

    for (const char *name : properties) {
        if (window->metaObject()->indexOfProperty(name) < 0) {
            qCritical() << "Main.qml thieu property:" << name;
            close(canSocket);
            return 1;
        }
    }

    QElapsedTimer clock;
    clock.start();

    qint64 lastMotion = -1;
    qint64 lastSensors = -1;
    qint64 lastLights = -1;

    QSocketNotifier notifier(canSocket, QSocketNotifier::Read);

    QObject::connect(
        &notifier, &QSocketNotifier::activated, &app,
        [&](QSocketDescriptor, QSocketNotifier::Type) {
            // Gioi han cong viec moi luot de giao dien khong bi dung.
            for (int i = 0; i < 64; ++i) {
                can_frame frame{};
                const ssize_t count =
                    read(canSocket, &frame, sizeof(frame));

                if (count < 0) {
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                        break;

                    if (errno == EINTR)
                        continue;

                    qWarning() << "Loi doc CAN:" << strerror(errno);
                    notifier.setEnabled(false);
                    break;
                }

                if (count != static_cast<ssize_t>(sizeof(frame)))
                    break;

                if (frame.can_id &
                    (CAN_EFF_FLAG | CAN_RTR_FLAG | CAN_ERR_FLAG))
                    continue;

                const qint64 now = clock.elapsed();

                switch (frame.can_id) {
                case 0x100: {
                    if (frame.can_dlc != 4 ||
                        frame.data[2] > 100 ||
                        (frame.data[3] != 0x01 &&
                         frame.data[3] != 0xFF))
                        break;

                    const unsigned int rpmX10 =
                        static_cast<unsigned int>(frame.data[0]) |
                        (static_cast<unsigned int>(frame.data[1]) << 8);

                    window->setProperty("canRpm", rpmX10 / 10.0);
                    window->setProperty("canThrottle",
                                        static_cast<int>(frame.data[2]));
                    window->setProperty("canDirection",
                                        frame.data[3] == 0xFF ? -1 : 1);

                    lastMotion = now;
                    window->setProperty("canMotionValid", true);
                    break;
                }

                case 0x101:
                    if (frame.can_dlc != 2 || frame.data[0] > 100)
                        break;

                    window->setProperty("canFuel",
                                        static_cast<int>(frame.data[0]));
                    window->setProperty("canCoolant",
                                        static_cast<int>(frame.data[1]));

                    lastSensors = now;
                    window->setProperty("canSensorsValid", true);
                    break;

                case 0x102:
                    if (frame.can_dlc != 2 ||
                        frame.data[0] > 2 ||
                        frame.data[1] > 2)
                        break;

                    window->setProperty("canTurnSignal",
                                        static_cast<int>(frame.data[0]));
                    window->setProperty("canLampMode",
                                        static_cast<int>(frame.data[1]));

                    lastLights = now;
                    window->setProperty("canLightsValid", true);
                    break;
                }
            }
        });

    // Kiem tra rieng tung nhom, tranh giu du lieu cu nhu dang con song.
    QTimer watchdog;

    QObject::connect(&watchdog, &QTimer::timeout, &app, [&]() {
        const qint64 now = clock.elapsed();

        window->setProperty(
            "canMotionValid",
            lastMotion >= 0 && now - lastMotion < 1000);

        window->setProperty(
            "canSensorsValid",
            lastSensors >= 0 && now - lastSensors < 1000);

        window->setProperty(
            "canLightsValid",
            lastLights >= 0 && now - lastLights < 1000);
    });

    watchdog.start(100);

    qInfo() << "Nhan CAN that: can0, IDs 100 / 101 / 102";

    const int result = app.exec();

    watchdog.stop();
    notifier.setEnabled(false);
    close(canSocket);
    return result;
}