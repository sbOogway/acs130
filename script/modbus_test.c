#include "modbus.h"
#include <stdio.h>

int main(void) {
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

    return 0;
}
