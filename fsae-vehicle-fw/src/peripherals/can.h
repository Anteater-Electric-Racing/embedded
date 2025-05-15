// Anteater Electric Racing, 2025

#ifndef CAN_H
#define CAN_H

void CAN_Init();
void CAN_SendVCU1Message(uint32_t id, uint64_t msg);
void CAN_Receive(uint32_t* rx_id, uint64_t* rx_data);


#endif
