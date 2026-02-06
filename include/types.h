/**
 * OpenWare OS - Common Type Definitions
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_TYPES_H
#define OPENWARE_TYPES_H

/* Unsigned integer types */
typedef unsigned char       uint8_t;
typedef unsigned short      uint16_t;
typedef unsigned int        uint32_t;
typedef unsigned long long  uint64_t;

/* Signed integer types */
typedef signed char         int8_t;
typedef signed short        int16_t;
typedef signed int          int32_t;
typedef signed long long    int64_t;

/* Size types */
typedef uint32_t            size_t;
typedef int32_t             ssize_t;

/* Boolean type */
typedef uint8_t             bool;
#define true                1
#define false               0

/* NULL pointer */
#define NULL                ((void*)0)

#endif /* OPENWARE_TYPES_H */
