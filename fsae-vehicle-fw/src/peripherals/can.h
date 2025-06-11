// Anteater Electric Racing, 2025

#ifndef CAN_H
#define CAN_H

void CAN_Init();
void CAN_SendVCU1Message(float torqueValue);
void CAN_ReceivePCCMessage();


#endif
