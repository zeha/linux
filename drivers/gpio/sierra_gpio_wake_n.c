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
#include <linux/err.h>
#include <linux/pm.h>

#include <mach/gpio.h>

#define NUM_WAKE_GPIOS 1
#define WAKEN_GNUMBER 77
#define DRIVER_NAME	"wake-n_gpio"
struct wake_n_pdata {
	int gpio;
	char name[64];
	int irq;
	struct wakeup_source ws;
} wake_n_pdata = {
	.gpio = WAKEN_GNUMBER,
};

static inline int gpio_check_and_wake(struct wake_n_pdata *w)
{
	int gpioval = gpio_get_value(w->gpio);

	pr_info("%s: %s state %s\n", __func__, w->name,
		(gpioval ? "SLEEP" : "WAKEUP"));
	if (gpioval)
		__pm_relax(&w->ws);
	else
		__pm_stay_awake(&w->ws);

	return gpioval;
}

static irqreturn_t gpio_wake_input_irq_handler(int irq, void *dev_id)
{
	struct wake_n_pdata *w = (struct wake_n_pdata*)dev_id;
	gpio_check_and_wake(w);
	return IRQ_HANDLED;
}

static int __init wake_n_probe(struct platform_device *pdev)
{
	int ret = 0;

	dev_info(&pdev->dev, "wake_n probe\n");

	ret = gpio_request(wake_n_pdata.gpio, "WAKE_N_GPIO");
	if (ret) {
		pr_err("%s: failed to get GPIO%d\n", __func__,
			wake_n_pdata.gpio);
		return ret;
	}

	snprintf(wake_n_pdata.name, sizeof(wake_n_pdata.name), "wake-n_gpio%d",
		wake_n_pdata.gpio);
	if (gpio_direction_input(wake_n_pdata.gpio)) {
		pr_err("%s: failed to set GPIO%d to input\n", __func__,
			wake_n_pdata.gpio);
		goto release_gpio;
	}

	wake_n_pdata.irq = gpio_to_irq(wake_n_pdata.gpio);
	if(wake_n_pdata.irq < 0){
		pr_err("%s: no IRQ associated with GPIO%d\n", __func__,
			wake_n_pdata.gpio);
		goto release_gpio;
	}

	ret = request_irq(wake_n_pdata.irq, gpio_wake_input_irq_handler,
                      IRQF_TRIGGER_RISING | IRQF_TRIGGER_FALLING,
                      wake_n_pdata.name, &wake_n_pdata);
	if (ret) {
		pr_err("%s: request_irq failed for GPIO%d (IRQ%d)\n", __func__,
			wake_n_pdata.gpio, wake_n_pdata.irq);
		goto release_gpio;
	}

	ret = enable_irq_wake(wake_n_pdata.irq);
	if (ret) {
		pr_err("%s: enable_irq failed for GPIO%d\n", __func__,
			wake_n_pdata.gpio);
		goto free_irq;
	}

	wakeup_source_init(&wake_n_pdata.ws, "wake-n_GPIO");
	gpio_check_and_wake(&wake_n_pdata);

	return 0;

free_irq:
	free_irq(wake_n_pdata.irq, NULL);
release_gpio:
	gpio_free(wake_n_pdata.gpio);
	return ret;
}

static int __devexit wake_n_remove(struct platform_device *pdev)
{
	pr_info("wake_n_remove");
	gpio_free(wake_n_pdata.gpio);
	wakeup_source_trash(&wake_n_pdata.ws);
  return 0;
}

static struct platform_driver wake_n_driver = {
    .probe = wake_n_probe,
    .remove = __devexit_p(wake_n_remove),
    .driver = {
        .name = DRIVER_NAME,
        .owner = THIS_MODULE,
    },
};

static int __init wake_n_init(void)
{
    return platform_driver_register(&wake_n_driver);
}

static void __exit wake_n_exit(void)
{
    platform_driver_unregister(&wake_n_driver);
}

module_init(wake_n_init);
module_exit(wake_n_exit);

MODULE_LICENSE("GPL v2");
MODULE_DESCRIPTION("GPIO wake_n pin driver");
