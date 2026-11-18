#ifndef TURN_SIGNALS_H
#define TURN_SIGNALS_H

void initTurnSignals();
void updateTurnSignals();
// 0 = tắt, 1 = trái, 2 = phải; trạng thái đã chống dội.
unsigned char getTurnSignalState(); 
#endif