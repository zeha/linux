/************
 *
 * $Id$
 *
 * Filename:  gpio_wake_n.c
 *
 * Purpose:    
 *           
 * Copyright: (c) 2014 Sierra Wireless, Inc.
 *            All rights reserved
 *
 ************/
#include <linux/module.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/interrupt.h>
#include <linux/suspend.h>
#include <linux/err.h>
#include <linux/pm.h>
#include <linux/wakelock.h>

#include <mach/gpio.h>
#include <mach/msm_xo.h>

/* should be aligned with bsudefs.h */
enum bspctype
{
	BSPC_NOT_SUPPORT = 0,
	BSPC_XO_SUPPORT,
	BSPC_VDDMIN_SUPPORT,
	BSPC_SUSMEM_SUPPORT,
	BSPC_SUPPORT_MAX
};

extern unsigned char otg_pc;
extern bool flag_usb_connect;
bool wake_n_waked = false;
struct msm_xo_voter *wpin_xohandle;
struct wake_lock waken_wlock;

#define DRIVER_NAME	"wake-n_gpio"
#define WAKEN_GNUMBER 77

static void gpio_suspend_w(struct work_struct *w);
static DECLARE_DELAYED_WORK(gpio_suspend_work, gpio_suspend_w);

static void gpio_suspend_w(struct work_struct *w)
{
    pm_suspend(PM_SUSPEND_MEM);
}

static irqreturn_t gpio_wake_input_irq_handler(int irq, void *dev_id)
{
  int g_invalue = 0xff;
  g_invalue = gpio_get_value(WAKEN_GNUMBER);
  
  pr_info("gpio_wake_input_irq_handler %d, usb connect state  %d", g_invalue, (int)flag_usb_connect);
  
  if(g_invalue==1)
  {
    wake_n_waked = false;

    if(!flag_usb_connect)
    {
      pr_info("gpio_wake_input_irq_handler suspen mem");
      queue_delayed_work(system_nrt_wq, &gpio_suspend_work, 1);
    }
  }
  else
  {
    wake_n_waked = true;
  }

  return IRQ_HANDLED;
}

static int __init wake_n_probe(struct platform_device *pdev)
{
  int ret = 0;
  int wake_irq;
  unsigned long req_flags = IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING;

  dev_info(&pdev->dev, "wake_n probe\n");

  ret = gpio_request(WAKEN_GNUMBER, "WAKE_N_GPIO");
  if (ret){
    pr_err("gpio_request: WAKE_N_GPIO failed\n");
    return ret;
  }
  else
  {
    ret = gpio_direction_input(WAKEN_GNUMBER);
    if (ret){
        pr_err("gpio_direction_input WAKE_N_GPIO fail\n");		
        goto err_gpiowaken_configure_failed;
    }
    
    ret = wake_irq = gpio_to_irq(WAKEN_GNUMBER);
    if(ret < 0){
      pr_err("gpio_to_irq WAKE_N_GPIO fail\n");
      goto err_gpio_get_irq_num_failed;
    }
  
    ret = request_irq(wake_irq, gpio_wake_input_irq_handler, req_flags, "WAKE_N_GPIO", NULL);
    if (ret){
      pr_err("request_irq WAKE_N_GPIO failed \n");
      goto err_request_irq_failed;
    }

    ret = enable_irq_wake(wake_irq);
    if (ret){
      pr_err("enable_irq WAKE_N_GPIO failed \n");
      goto err_enable_irq_wake_failed;
    }

    wpin_xohandle = msm_xo_get(MSM_XO_TCXO_D0, "WAKE_N_GPIO");
    if (IS_ERR(wpin_xohandle)) {
      pr_err("Not able able to get WAKE_N_GPIO XO handle\n");
    }
    else{
      pr_info("WAKE_N_GPIO get XO handle and vote to OFF first");
      ret = msm_xo_mode_vote(wpin_xohandle, MSM_XO_MODE_OFF);
      if (ret){
        pr_err("WAKE_N_GPIO fail to vote for TCXO, free XO handle\n");
        goto free_xo_handle;
      }
    }

    wake_lock_init(&waken_wlock, WAKE_LOCK_SUSPEND, "wake-n_GPIO");
    wake_lock(&waken_wlock);
  }

  return 0;

  free_xo_handle:
    msm_xo_put(wpin_xohandle);
  err_enable_irq_wake_failed:
    free_irq(wake_irq, NULL); 
  err_gpiowaken_configure_failed:
    gpio_free(WAKEN_GNUMBER);
  err_request_irq_failed:
  err_gpio_get_irq_num_failed:
    return ret;
}

static int __devexit wake_n_remove(struct platform_device *pdev)
{ 
  pr_info("wake_n_remove");
  gpio_free(WAKEN_GNUMBER);
  wake_lock_destroy(&waken_wlock);
  return 0;
}

#ifdef CONFIG_PM_RUNTIME
static int wake_n_runtime_idle(struct device *dev)
{
  return 0;
}

static int wake_n_runtime_suspend(struct device *dev)
{
  pr_info("wake_n_runtime_suspend");
    
  if(!wake_n_waked)
  {
    if (!IS_ERR(wpin_xohandle)){
      msm_xo_mode_vote(wpin_xohandle, MSM_XO_MODE_OFF); 
    }  
  }
  else
  {
    if (!IS_ERR(wpin_xohandle)){
      msm_xo_mode_vote(wpin_xohandle, MSM_XO_MODE_ON); 
    }
  }
  
  return 0;
}

static int wake_n_runtime_resume(struct device *dev)
{
    int ret = 0;
    pr_info("wake_n_runtime_resume");

    if(!wake_n_waked)
    {
      if (!IS_ERR(wpin_xohandle)){
        msm_xo_mode_vote(wpin_xohandle, MSM_XO_MODE_ON); 
      }
    }

    return ret;
}
#endif

#ifdef CONFIG_PM_SLEEP
static int wake_n_pm_suspend(struct device *dev)
{
  int ret = 0;

  pr_info("wake_n_pm_suspend");
  
#ifdef CONFIG_SIERRA_VDDMIN
  /* ensure any pending pm_suspend is cancelled */
  if (delayed_work_pending(&gpio_suspend_work))
  {
    pr_info("gpio_suspend_work delay pending");
    cancel_delayed_work(&gpio_suspend_work);
  }
#endif

  wake_unlock(&waken_wlock);

  return ret;
}

static int wake_n_pm_resume(struct device *dev)
{
    int ret = 0;

    pr_info("wake_n_pm_resume");

    wake_lock(&waken_wlock);

    return ret;
}
#endif

#ifdef CONFIG_PM
     static const struct dev_pm_ops wake_n_dev_pm_ops = {
         SET_SYSTEM_SLEEP_PM_OPS(wake_n_pm_suspend, wake_n_pm_resume)
         SET_RUNTIME_PM_OPS(wake_n_runtime_suspend, wake_n_runtime_resume,
                wake_n_runtime_idle)
     };
#endif

static struct platform_driver wake_n_driver = {
    .remove = __devexit_p(wake_n_remove),
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
#ifdef CONFIG_PM
        .pm = &wake_n_dev_pm_ops,
#endif
    },
};

static int __init wake_n_init(void)
{
    return platform_driver_probe(&wake_n_driver, wake_n_probe);
}

static void __exit wake_n_exit(void)
{
    platform_driver_unregister(&wake_n_driver);
}

module_init(wake_n_init);
module_exit(wake_n_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GPIO wake_n pin driver");

