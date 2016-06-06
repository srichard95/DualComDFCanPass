/*
 * console.c
 *
 *  Created on: 2015 jún. 12
 *      Author: Richárd
 *
 *      DualCom Shell COde
 */

#include <string.h>
#include <stdlib.h>
#include <time.h>

#include "ch.h"
#include "hal.h"
#include "test.h"

#include "shell.h"
#include "chprintf.h"

#include "console.h"
#include "EspUart.h"

/*===========================================================================*/
/* Command line related.                                                     */
/*===========================================================================*/

#define SHELL_WA_SIZE   THD_WORKING_AREA_SIZE(2048)
#define TEST_WA_SIZE    THD_WORKING_AREA_SIZE(256)

static void cmd_mem(BaseSequentialStream *chp, int argc, char *argv[]) {
  size_t n, size;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: mem\r\n");
    return;
  }
  n = chHeapStatus(NULL, &size);
  chprintf(chp, "core free memory : %u bytes\r\n", chCoreGetStatusX());
  chprintf(chp, "heap fragments   : %u\r\n", n);
  chprintf(chp, "heap free total  : %u bytes\r\n", size);
}

static void cmd_threads(BaseSequentialStream *chp, int argc, char *argv[]) {
  static const char *states[] = {CH_STATE_NAMES};
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: threads\r\n");
    return;
  }
  chprintf(chp, "    addr    stack prio refs     state\r\n");
  tp = chRegFirstThread();
  do {
    chprintf(chp, "%08lx %08lx %4lu %4lu %9s %lu\r\n",
             (uint32_t)tp, (uint32_t)tp->p_ctx.r13,
             (uint32_t)tp->p_prio, (uint32_t)(tp->p_refs - 1),
             states[tp->p_state]);
    tp = chRegNextThread(tp);
  } while (tp != NULL);
}

static void cmd_test(BaseSequentialStream *chp, int argc, char *argv[]) {
  thread_t *tp;

  (void)argv;
  if (argc > 0) {
    chprintf(chp, "Usage: test\r\n");
    return;
  }
  tp = chThdCreateFromHeap(NULL, TEST_WA_SIZE, chThdGetPriorityX(),
                           TestThread, chp);
  if (tp == NULL) {
    chprintf(chp, "out of memory\r\n");
    return;
  }
  chThdWait(tp);
}

static const ShellCommand commands[] = {
  {"mem", cmd_mem},
  {"threads", cmd_threads},
  {"test", cmd_test},
  {"esp_send", esp_send},
  {"start_driver_test", start_driver_test},
  {"getdllstats", GetDllStats},
  {NULL, NULL}
};

static const ShellConfig shCfg = {
    (BaseSequentialStream *)&SD2,
    commands
};


static SerialConfig ser_cfg = {
  115200,
  0,
  0,
  0,
};

thread_t *consoleThread;

void consoleInit(void){
  /* Shell initialization.*/
  sdStart(&SD2, &ser_cfg);
  shellInit();
  consoleThread = NULL;
}

void consoleStart(void){
  if (!consoleThread) {
    consoleThread = shellCreate(&shCfg, SHELL_WA_SIZE, NORMALPRIO);
  }
  else if (chThdTerminatedX(consoleThread)) {
    chThdRelease(consoleThread);    /* Recovers memory of the previous shell.   */
    consoleThread = NULL;           /* Triggers spawning of a new shell.        */
  }
}



