#ifndef AUX_SENSORS_H
#define AUX_SENSORS_H

void initAuxSensors();
void updateAuxSensors();

int getAux2Raw();
int getAux3Raw();

int getAux2Percent();
int getAux3Percent();

#endif