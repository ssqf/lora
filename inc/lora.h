#ifndef __LORA_H
#define __LORA_H
#include "stm8l15x_conf.h"
#include "stm8l15x_flash.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "config.h"
#include "task.h"
#include "uart.h"
#include "init.h"
#include "clock.h"
#include "protocol.h"
#include "at_cmd.h"

//#define NULL ((void *)0)

#define __DEBUG__

#ifdef __DEBUG__
#define Debug(fmt, ...) printf(#fmt "\r\n", ##__VA_ARGS__)
#else
#define Debug(fmt, ...)
#endif // !__DEBUG__

#endif //__LORA_H