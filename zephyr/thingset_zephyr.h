/*
 * Copyright (c) 2021 Bobby Noelte.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef THINGSET_ZEPHYR_H_
#define THINGSET_ZEPHYR_H_

#if !CONFIG_THINGSET_ZEPHYR
#error "You need to define CONGIG_THINGSET_ZEPHYR."
#endif

/* Logging */
#ifndef LOG_MODULE_NAME
#define LOG_MODULE_NAME thingset
#define LOG_LEVEL CONFIG_THINGSET_LOG_LEVEL
#endif

#include <version.h>

#if ZEPHYR_VERSION_CODE >= 0x030100
#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#else
#include <zephyr.h>
#include <logging/log.h>
#endif

#ifdef THINGSET_MAIN
LOG_MODULE_REGISTER(LOG_MODULE_NAME);
#else
LOG_MODULE_DECLARE(LOG_MODULE_NAME);
#endif

#if CONFIG_MINIMAL_LIBC
/*
 * Zephyr's minimal libc is missing some functions.
 * Provide !!sufficient!! replacements here.
 */
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

#define isnan(value) __builtin_isnan(value)
#define isinf(value) __builtin_isinf(value)

long int lroundf(float x);

long long int llroundf(float x);

double ts_strtod(const char * string, char **endPtr);
inline double strtod(const char * string, char **endPtr)
{
    return ts_strtod(string, endPtr);
};

#if ZEPHYR_VERSION_CODE < 0x030100
inline long long strtoll(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (long long)strtol(str, endptr, base);
};

inline unsigned long long strtoull(const char *str, char **endptr, int base)
{
    /* XXX good enough for thingset uses ?*/
    return (unsigned long long)strtoul(str, endptr, base);
};
#endif /* ZEPHYR_VERSION_CODE < 0x030100 */

#endif /* CONFIG_MINIMAL_LIBC */

#if CONFIG_ZTEST
/*
 * Thingset unit tests are basically made for the Unity test framework.
 * Provide some adaptations to make them run under the ztest framework.
 */
#include "ztest/ztest_unity.h"
#endif

#endif /* THINGSET_ZEPHYR_H_ */
