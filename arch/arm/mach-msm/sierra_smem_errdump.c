/* arch/arm/mach-msm/sierra_smem_errdump.c
 *
 * Sierra SMEM utility functions. These functions don't rely on Sierra SMEM driver,
 * and can be used in early kernel start (after paging_init)
 *
 * Copyright (C) 2012 Sierra Wireless, Inc
 * Author: Brad Du <bdu@sierrawireless.com>
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 */

#include <linux/string.h>
#include <linux/kernel.h>
#include <linux/export.h>
#include <linux/mutex.h>
#include <asm/uaccess.h>
#include <mach/sierra_smem.h>

static DEFINE_MUTEX(errdump_lock);

static struct sER_DATA *sierr_smem_get_dump_buf(void)
{
  return (struct sER_DATA *)SIERRA_SMEM_ERR_DUMP_START;
}

void sierra_smem_errdump_save_start(void)
{
  struct sER_DATA *errdatap = sierr_smem_get_dump_buf();

  if(mutex_trylock(&errdump_lock))
  {
    /* note that the errdatap can only be accessible after paging_init at kernel start
     * If there is a panic before paging_init, the following line will likely cause another panic
     */
    memset((void *)errdatap, 0x00, sizeof(struct sER_DATA));
    mutex_unlock(&errdump_lock);
  }
  /* else, reentry, don't save */
}
EXPORT_SYMBOL(sierra_smem_errdump_save_start);

void sierra_smem_errdump_save_timestamp(uint32_t time_stamp)
{
  struct sER_DATA *errdatap = sierr_smem_get_dump_buf();

  if(mutex_trylock(&errdump_lock))
  {
    if(errdatap->time_stamp == 0)
    {
      errdatap->time_stamp = time_stamp;
    }
    /* else time_stamp has sth, should not happen since
     * it should be cleared at sierra_smem_errdump_save_start
     */

    mutex_unlock(&errdump_lock);
  }
  /* else, reentry, don't save */
}
EXPORT_SYMBOL(sierra_smem_errdump_save_timestamp);

void sierra_smem_errdump_save_errstr(char *errstrp)
{
  struct sER_DATA *errdatap = sierr_smem_get_dump_buf();

  if(mutex_trylock(&errdump_lock))
  {
    if(errdatap->error_string[0] == 0x00)
    {
      errdatap->start_marker = ERROR_START_MARKER;
      errdatap->error_source = ERROR_FATAL_ERROR;

      strncpy(errdatap->error_string, errstrp, ERROR_STRING_LEN);
      errdatap->error_string[ERROR_STRING_LEN - 1] = 0x00;

      errdatap->end_marker = ERROR_END_MARKER;
    }
    /* else error_string has sth, should not happen since
     * it should be cleared at sierra_smem_errdump_save_start
     */
    
    mutex_unlock(&errdump_lock);
  }
  /* else, reentry, don't save */
}
EXPORT_SYMBOL(sierra_smem_errdump_save_errstr);

void sierra_smem_errdump_save_fmtstr(char *errstrp)
{
  struct sER_DATA *errdatap = sierr_smem_get_dump_buf();

  if(mutex_trylock(&errdump_lock))
  {
    if(errdatap->format_string[0] == 0x00)
    {
      strncpy(errdatap->format_string, errstrp, ERROR_STRING_LEN);
      errdatap->format_string[ERROR_STRING_LEN - 1] = 0x00;

      errdatap->ext_marker = ERROR_END_MARKER;
      errdatap->format_marker = ERROR_END_MARKER;
    }
    /* else format_string has sth, should not happen since
     * it should be cleared at sierra_smem_errdump_save_start
     */

    mutex_unlock(&errdump_lock);
  }
  /* else, reentry, don't save */
}
EXPORT_SYMBOL(sierra_smem_errdump_save_fmtstr);

void sierra_smem_errdump_save_frame(void *taskp, struct stackframe *framep)
{
  struct sER_DATA *errdatap = sierr_smem_get_dump_buf();
  unsigned long *stackp, stack_index;

  if(mutex_trylock(&errdump_lock))
  {
    if(errdatap->program_counter == 0)
    {
      errdatap->start_marker = ERROR_START_MARKER;
  
      errdatap->program_counter = framep->pc;
      errdatap->registers[11] = framep->fp; 
      errdatap->registers[13] = framep->sp; 
      errdatap->registers[14] = framep->lr;
  
      /* use frame pointer which is one step closer than stack pointer */
      /* taskp != 0: kernel space stack processing : */
      if(taskp && framep->fp)
      {
        stackp = (unsigned long *)framep->fp;
        /* match mpss side pattern */
        for (stack_index = 0; stack_index < MAX_STACK_DATA; stack_index ++)
        {
          errdatap->stack_data[MAX_STACK_DATA - stack_index - 1] = stackp[stack_index];
        }
      }
      /* taskp == 0: user space stack processing: */
      else if(taskp == 0 && access_ok(VERIFY_READ, (char __user *)framep->fp, MAX_STACK_DATA * sizeof(long)))
      {
        /* match mpss side pattern */
        for (stack_index = 0; stack_index < MAX_STACK_DATA; stack_index ++)
        {
          get_user(errdatap->stack_data[MAX_STACK_DATA - stack_index - 1],
                   (unsigned long __user *)(framep->fp + (sizeof(long) * stack_index)));
        }
      }
 
      sprintf(errdatap->task_name, "%08X", (unsigned int)taskp);

      errdatap->end_marker = ERROR_END_MARKER;
    }
    /* else pc has sth, should not happen since
     * it should be cleared at sierra_smem_errdump_save_start
     */

    mutex_unlock(&errdump_lock);
  }
  /* else, reentry, don't save */
}
EXPORT_SYMBOL(sierra_smem_errdump_save_frame);

int sierra_smem_get_download_mode(void)
{
  struct bcboottoappmsg *b2amsgp = (struct bcboottoappmsg *)BS_BOOT_APP_MSG_START;
  int download_mode = 0;

  if(b2amsgp->bcstartmarker == b2amsgp->bcendmarker &&
     (b2amsgp->bcstartmarker & BC_MSG_MARKER_M) == (BC_VALID_BOOT_MSG_MARKER & BC_MSG_MARKER_M))
  {
    if(b2amsgp->flags & BCBOOTAPPFLAG_DLOAD_MODE_M)
    {
      download_mode = 1;
    }
  }

  return download_mode;
}
EXPORT_SYMBOL(sierra_smem_get_download_mode);
