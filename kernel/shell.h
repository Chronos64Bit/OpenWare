/**
 * OpenWare OS - Shell Header
 * Copyright (c) 2026 Ventryx Inc. All rights reserved.
 */

#ifndef OPENWARE_SHELL_H
#define OPENWARE_SHELL_H

#include "../include/types.h"

/* Shell constants */
#define SHELL_MAX_INPUT     256
#define SHELL_MAX_ARGS      16
#define SHELL_PROMPT        "openware> "

/* Shell functions */
void shell_init(void);
void shell_run(void);

#endif /* OPENWARE_SHELL_H */
