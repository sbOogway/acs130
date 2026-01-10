#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <errno.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define SLAVE_ID 1
#define BAUDRATE 19200
#define PARITY 'N'
#define DATA_BITS 8
#define STOP_BITS 1
#define TIMEOUT_SEC 2

int read_register(modbus_t *ctx, int addr, uint16_t *value) {
    int rc = modbus_read_registers(ctx, addr, 1, value);
    if (rc == -1) {
        fprintf(stderr, "Failed to read register %d: %s\n", addr, modbus_strerror(errno));
        return -1;
    }
    return 0;
}

modbus_t* get_client(void) {
    modbus_t *ctx = modbus_new_rtu(SERIAL_PORT, BAUDRATE, PARITY, DATA_BITS, STOP_BITS);
    if (!ctx) {
        fprintf(stderr, "Failed to create modbus context\n");
        return NULL;
    }
    modbus_set_slave(ctx, SLAVE_ID);
    modbus_set_response_timeout(ctx, TIMEOUT_SEC, 0);
    return ctx;
}

void check_faults(modbus_t *ctx) {
    uint16_t regs[2];
    int rc;

    rc = modbus_read_registers(ctx, 50, 1, &regs[0]);
    if (rc == -1) {
        printf("Unable to read status word\n");
        return;
    }

    rc = modbus_read_registers(ctx, 102, 1, &regs[1]);
    if (rc == -1) {
        printf("Unable to read fault code\n");
        return;
    }

    uint16_t sw = regs[0];
    uint16_t last_fault = regs[1];
    int is_faulted = sw & 0x0008;

    printf("\n--- DIAGNOSTIC STATUS ---\n");
    if (is_faulted) {
        printf("STATUS: [!] IN FAULT (Code: %d)\n", last_fault);
        if (last_fault == 0) printf("Note: Fault detected but code unavailable.\n");
    } else {
        printf("STATUS: OK (No active fault)\n");
    }

    if (sw & 0x0080) {
        printf("WARNING: Alarm/Warning active.\n");
    }
}

void test_connection(void) {
    modbus_t *ctx = get_client();
    if (!ctx) return;

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }

    printf("\n========================================\n");
    printf("BASIC CONNECTION TEST\n");
    printf("========================================\n");

    uint16_t regs[4];
    int rc = modbus_read_registers(ctx, 0, 4, regs);
    if (rc == -1) {
        printf("Modbus error: %s\n", modbus_strerror(errno));
    } else {
        printf("Frequency: %.2f Hz\n", regs[0] / 100.0);
        printf("Current: %.2f A\n", regs[1] / 10.0);
        check_faults(ctx);
    }

    modbus_close(ctx);
    modbus_free(ctx);
}

void continuous_monitoring(void) {
    modbus_t *ctx = get_client();
    if (!ctx) return;

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }

    printf("\n========================================\n");
    printf("MONITORING (Ctrl+C to exit)\n");
    printf("========================================\n");

    uint16_t regs[3];
    while (1) {
        int rc_data = modbus_read_registers(ctx, 0, 2, &regs[0]);
        int rc_sw = modbus_read_registers(ctx, 50, 1, &regs[2]);

        if (rc_data != -1 && rc_sw != -1) {
            double f = regs[0] / 100.0;
            double a = regs[1] / 10.0;
            uint16_t sw = regs[2];

            const char *status = "FAULT";
            if (sw & 0x0004) status = "RUNNING";
            else if (!(sw & 0x0008)) status = "READY";

            printf("\rFreq: %6.2fHz | Amp: %5.2fA | Status: %-8s", f, a, status);
            fflush(stdout);
        }
        usleep(500000);
    }

    modbus_close(ctx);
    modbus_free(ctx);
}

void read_all_registers(void) {
    modbus_t *ctx = get_client();
    if (!ctx) return;

    if (modbus_connect(ctx) == -1) {
        fprintf(stderr, "Connection failed: %s\n", modbus_strerror(errno));
        modbus_free(ctx);
        return;
    }

    for (int i = 0; i < 59; i++) {
        uint16_t reg_data, sw;
        int rc_data = modbus_read_registers(ctx, i, 1, &reg_data);
        int rc_sw = modbus_read_registers(ctx, 50, 1, &sw);

        if (rc_data != -1 && rc_sw != -1) {
            printf("========================================\n");
            printf("register\t-> %d\n", i);
            printf("data\t\t-> %d\n", reg_data);

            const char *status = "FAULT";
            if (sw & 0x0004) status = "RUNNING";
            else if (!(sw & 0x0008)) status = "READY";

            printf("%s\n", status);

            if (strcmp(status, "FAULT") == 0) {
                modbus_close(ctx);
                modbus_free(ctx);
                return;
            }
        }
        usleep(500000);
    }

    modbus_close(ctx);
    modbus_free(ctx);
}
