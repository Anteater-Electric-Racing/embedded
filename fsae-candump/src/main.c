#include <device.h>
#include <drivers/can.h>
#include <sys/byteorder.h>
#include <sys/printk.h>
#include <zephyr.h>

#define RX_THREAD_STACK_SIZE 512
#define RX_THREAD_PRIORITY 2

K_THREAD_STACK_DEFINE(rx_thread_stack, RX_THREAD_STACK_SIZE);
struct k_thread rx_thread_data;

const struct device *const can_dev =
    DEVICE_DT_GET(DT_CHOSEN(zephyr_can_primary));
CAN_DEFINE_MSGQ(rx_msgq, 8);

void rx_thread(void *arg1, void *arg2, void *arg3) {
    ARG_UNUSED(arg1);
    ARG_UNUSED(arg2);
    ARG_UNUSED(arg3);

    struct zcan_filter all_filter = {.id_type = CAN_STANDARD_IDENTIFIER,
                                     .rtr = CAN_DATAFRAME,
                                     .id = 0,
                                     .rtr_mask = 0,
                                     .id_mask = 0};

    int filter_id = can_attach_msgq(can_dev, &rx_msgq, &all_filter);
    if (filter_id < 0) {
        printk("Failed to attach CAN filter\n");
        return;
    }
    printk("CAN candump started. Filter id: %d\n", filter_id);

    struct zcan_frame msg;
    while (1) {
        k_msgq_get(&rx_msgq, &msg, K_FOREVER);

        printk("CAN id: 0x%x [%d] data:", msg.id, msg.dlc);
        for (int i = 0; i < msg.dlc; i++) {
            printk(" %02x", msg.data[i]);
        }
        printk("\n");
    }
}

void main(void) {
    if (!device_is_ready(can_dev)) {
        printk("CAN: Device not ready.\n");
        return;
    }

    k_tid_t rx_tid =
        k_thread_create(&rx_thread_data, rx_thread_stack,
                        K_THREAD_STACK_SIZEOF(rx_thread_stack), rx_thread, NULL,
                        NULL, NULL, RX_THREAD_PRIORITY, 0, K_NO_WAIT);
    if (!rx_tid) {
        printk("ERROR spawning rx thread\n");
    }

    printk("candump running...\n");
}
