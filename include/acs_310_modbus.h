#ifndef MODBUS_H
#define MODBUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct _modbus modbus_t;

int read_register(modbus_t *ctx, int addr, uint16_t *value);
int write_register(modbus_t *ctx, int addr, uint16_t value);
modbus_t* get_client(void);
void check_faults(modbus_t *ctx);
void test_connection(void);
void continuous_monitoring(void);
void read_all_registers(void);

#ifdef __cplusplus
}
#endif

#endif
