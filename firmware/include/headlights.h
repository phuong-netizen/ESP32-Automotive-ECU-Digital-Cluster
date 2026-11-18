#ifndef HEADLIGHTS_H
#define HEADLIGHTS_H

enum class HeadlightMode {
    OFF,
    LOW_BEAM,   // Cốt
    HIGH_BEAM   // Pha
};

void initHeadlights();
void updateHeadlights();

// Trạng thái đang được phần mềm ra lệnh.
// Không phải phản hồi từ tiếp điểm relay thực tế.
HeadlightMode getHeadlightMode();

#endif