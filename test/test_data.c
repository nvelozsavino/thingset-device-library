/*
 * Copyright (c) 2020 Martin Jäger / Libre Solar
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#include "test.h"

/* Provide for inclusion in C++ code */
#ifdef __cplusplus
extern "C" {
#endif

// info
char manufacturer[] = "Libre Solar";
static uint32_t timestamp = 12345678;
static char device_id[] = "ABCD1234";

// conf
static float bat_charging_voltage = 14.4;
static float load_disconnect_voltage = 10.8;

// input
static bool enable_switch = false;

// meas
static float battery_voltage = 14.1;
static float battery_current = 5.13;
static int16_t ambient_temp = 22;

// rec
static float bat_energy_hour = 32.2;
static float bat_energy_day = 123;
static int16_t ambient_temp_max_day = 28;

// pub
bool pub_report_enable = false;
uint16_t pub_report_interval = 1000;
bool pub_info_enable = true;

// exec
void reset_function(void);
void auth_function(void);
char auth_password[11];

char strbuf[300];

float f32;

static uint64_t ui64;
static int64_t i64;

static uint32_t ui32;
int32_t i32;

static uint16_t ui16;
static int16_t i16;

bool b;

int32_t A[100] = {4, 2, 8, 4};
struct ts_array_info int32_array = {A, sizeof(A)/sizeof(int32_t), 4, TS_T_INT32};

float B[100] = {2.27, 3.44};
struct ts_array_info float32_array = {B, sizeof(B)/sizeof(float), 2, TS_T_FLOAT32};

uint8_t bytes[300] = {};
struct ts_bytes_buffer bytes_buf = { bytes, 0 };

void dummy(void);
void conf_callback(void);


struct ts_data_node data_nodes[] = {

    // DEVICE INFORMATION /////////////////////////////////////////////////////

    TS_NODE_PATH(ID_INFO, "info", 0, NULL),

    TS_NODE_STRING(0x19, "Manufacturer", manufacturer, 0, ID_INFO, TS_ANY_R, 0),
    TS_NODE_UINT32(0x1A, "Timestamp_s", &timestamp, ID_INFO, TS_ANY_RW, PUB_REPORT),
    TS_NODE_STRING(0x1B, "DeviceID", device_id, sizeof(device_id), ID_INFO, TS_ANY_R | TS_MKR_W, 0),

    // CONFIGURATION //////////////////////////////////////////////////////////

    TS_NODE_PATH(ID_CONF, "conf", 0, &conf_callback),

    TS_NODE_FLOAT(0x31, "BatCharging_V", &bat_charging_voltage, 2, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_FLOAT(0x32, "LoadDisconnect_V", &load_disconnect_voltage, 2, ID_CONF, TS_ANY_RW, 0),

    // INPUT DATA /////////////////////////////////////////////////////////////

    TS_NODE_PATH(ID_INPUT, "input", 0, NULL),

    TS_NODE_BOOL(0x61, "EnableCharging", &enable_switch, ID_INPUT, TS_ANY_RW, 0),

    // MEASUREMENT DATA ///////////////////////////////////////////////////////

    TS_NODE_PATH(ID_MEAS, "meas", 0, NULL),

    TS_NODE_FLOAT(0x71, "Bat_V", &battery_voltage, 2, ID_MEAS, TS_ANY_R, PUB_REPORT | PUB_CAN),
    TS_NODE_FLOAT(0x72, "Bat_A", &battery_current, 2, ID_MEAS, TS_ANY_R, PUB_REPORT | PUB_CAN),
    TS_NODE_INT16(0x73, "Ambient_degC", &ambient_temp, ID_MEAS, TS_ANY_R, PUB_REPORT),

    // RECORDED DATA //////////////////////////////////////////////////////////

    TS_NODE_PATH(ID_REC, "rec", 0, NULL),

    TS_NODE_FLOAT(0xA1, "BatHour_kWh", &bat_energy_hour, 2, ID_REC, TS_ANY_R, 0),
    TS_NODE_FLOAT(0xA2, "BatDay_kWh", &bat_energy_day, 2, ID_REC, TS_ANY_R, 0),
    TS_NODE_INT16(0xA3, "AmbientMaxDay_degC", &ambient_temp_max_day, ID_REC, TS_ANY_R, 0),

    // REMOTE PROCEDURE CALLS /////////////////////////////////////////////////

    TS_NODE_PATH(ID_RPC, "rpc", 0, NULL),

    TS_NODE_EXEC(0xE1, "x-reset", &reset_function, ID_RPC, TS_ANY_RW),
    TS_NODE_EXEC(0xE2, "x-auth", &auth_function, ID_RPC, TS_ANY_RW),
    TS_NODE_STRING(0xE3, "Password", auth_password, sizeof(auth_password), 0xE2, TS_ANY_RW, 0),

    // REPORTS ////////////////////////////////////////////////////////////////

    TS_NODE_PUBSUB(0xF4, "report", PUB_REPORT, 0, TS_ANY_RW, 0),

    // PUBLICATION DATA ///////////////////////////////////////////////////////

    TS_NODE_PATH(ID_PUB, ".pub", 0, NULL),

    TS_NODE_PATH(0xF1, "report", ID_PUB, NULL),
    TS_NODE_BOOL(0xF2, "Enable", &pub_report_enable, 0xF1, TS_ANY_RW, 0),
    TS_NODE_UINT16(0xF3, "Interval_ms", &pub_report_interval, 0xF1, TS_ANY_RW, 0),

    TS_NODE_PATH(0xF5, "info", ID_PUB, NULL),
    TS_NODE_BOOL(0xF6, "OnChange", &pub_info_enable, 0xF5, TS_ANY_RW, 0),

    // UNIT TEST DATA /////////////////////////////////////////////////////////
    // using IDs >= 0x1000

    TS_NODE_PATH(0x1000, "test", 0, NULL),

    TS_NODE_INT32(0x4001, "i32_readonly", &i32, 0x1000, TS_ANY_R, 0),

    TS_NODE_EXEC(0x5001, "x-dummy", &dummy, ID_RPC, TS_ANY_RW),

    TS_NODE_UINT64(0x6001, "ui64", &ui64, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_INT64(0x6002, "i64", &i64, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_UINT32(0x6003, "ui32", &ui32, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_INT32(0x6004, "i32", &i32, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_UINT16(0x6005, "ui16", &ui16, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_INT16(0x6006, "i16", &i16, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_FLOAT(0x6007, "f32", &f32, 2, ID_CONF, TS_ANY_RW, 0),
    TS_NODE_BOOL(0x6008, "bool", &b, ID_CONF, TS_ANY_RW, 0),

    TS_NODE_STRING(0x6009, "strbuf", strbuf, sizeof(strbuf), ID_CONF, TS_ANY_RW, 0),

    TS_NODE_FLOAT(0x600A, "f32_rounded", &f32, 0, ID_CONF, TS_ANY_RW, 0),

    TS_NODE_UINT32(0x7001, "secret_expert", &ui32, ID_CONF, TS_ANY_R | TS_EXP_W | TS_MKR_W, 0),
    TS_NODE_UINT32(0x7002, "secret_maker", &ui32, ID_CONF, TS_ANY_R | TS_MKR_W, 0),
    TS_NODE_ARRAY(0x7003, "arrayi32", &int32_array, 0, ID_CONF, TS_ANY_RW, 0),
    // data_node->detail will specify the number of decimal places for float
    TS_NODE_ARRAY(0x7004, "arrayfloat", &float32_array, 2, ID_CONF, TS_ANY_RW, 0),

    TS_NODE_BYTES(0x8000, "bytesbuf", &bytes_buf, sizeof(bytes), ID_CONF, TS_ANY_RW, 0),
};

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array) (sizeof(array) / sizeof((array)[0]))
#endif

size_t data_nodes_size = ARRAY_SIZE(data_nodes);

#ifdef __cplusplus
} /** extern "C" */
#endif
