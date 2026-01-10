#include "acs_130_modbus.h"
#include <modbus/modbus.h>
#include <stdio.h>

int main(void) {
    modbus_t *ctx = get_client();
    if (!ctx) return 1;

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed\n");
        modbus_free(ctx);
        return 1;
    }

    uint16_t value;
    int addr = 0;
    if (read_register(ctx, addr, &value) == 0) {
        printf("Register %d value: %u\n", addr, value);
    }

    char choice[10];

    while (1) {
        printf("\n1. Test Connection and Fault\n2. Continuous Monitoring\n3. Read All Registers\n0. Exit\n");
        printf("Choice: ");
        fflush(stdout);

        if (fgets(choice, sizeof(choice), stdin) == NULL) break;

        if (choice[0] == '1') {
            test_connection();
        } else if (choice[0] == '2') {
            continuous_monitoring();
        } else if (choice[0] == '3') {
            read_all_registers();
        } else if (choice[0] == '0') {
            break;
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return 0;
}
