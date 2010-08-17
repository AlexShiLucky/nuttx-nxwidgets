/****************************************************************************
 *  arch/arm/src/src/up_prefetchabort.c
 *
 *   Copyright (C) 2007-2010 Gregory Nutt. All rights reserved.
 *   Author: Gregory Nutt <spudmonkey@racsa.co.cr>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 * 3. Neither the name NuttX nor the names of its contributors may be
 *    used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 ****************************************************************************/

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include <nuttx/config.h>

#include <stdint.h>
#include <debug.h>

#include <nuttx/irq.h>
#ifdef CONFIG_PAGING
#  include <nuttx/page.h>
#endif

#include "os_internal.h"
#include "up_internal.h"

/****************************************************************************
 * Pre-processor Definitions
 ****************************************************************************/

/* Configuration ************************************************************/

#ifdef CONFIG_PAGING
#  ifndef CONFIG_PAGING_PAGESIZE
#    error "CONFIG_PAGING_PAGESIZE is not defined in your .config file"
#  endif
#  ifndef CONFIG_PAGING_NLOCKED
#    error "CONFIG_PAGING_NLOCKED is not defined in your .config file"
#  endif
#  ifndef CONFIG_PAGING_NPAGES
#    error "CONFIG_PAGING_NPAGES is not defined in your .config file"
#  endif
#endif

/* Output debug info if stack dump is selected -- even if 
 * debug is not selected.
 */

#ifdef CONFIG_ARCH_STACKDUMP
# undef  lldbg
# define lldbg lib_lowprintf
#endif

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Private Functions
 ****************************************************************************/

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/****************************************************************************
 * Name: up_prefetchabort
 ****************************************************************************/

void up_prefetchabort(uint32_t *regs)
{
  /* Save the saved processor context in current_regs where it can be accessed
   * for register dumps and possibly context switching.
   */

  current_regs = regs;

#ifdef CONFIG_PAGING
  /* Get the (virtual) address of instruction that caused the prefetch abort.
   * When the exception occurred, this address was provide in the lr register
   * and this value was saved in the context save area as the PC at the
   * REG_R15 index.
   *
   * Check to see if this miss address is within the configured range of
   * virtual addresses.
   */

  if (regs[REG_R15] >= CONFIG_PAGING_PAGEDBASE &&
      regs[REG_R15] <  CONFIG_PAGING_PAGEDEND)
    {
      /* Call pg_miss() to schedule the page fill.  A consequences of this
       * call are:
       *
       * (1) The currently executing task will be blocked and saved on
       *     on the g_waitingforfill task list.
       * (2) An interrupt-level context switch will occur so that when
       *     this function returns, it will return to a different task,
       *     most likely the page fill worker thread.
       * (3) The page fill worker task has been signalled and should
       *     execute immediately when we return from this exception.
       */

      pg_miss();
    }
  else
#endif
    {
      lldbg("Prefetch abort at 0x%x\n", regs[REG_PC]);
      PANIC(OSERR_ERREXCEPTION);
    }
}
