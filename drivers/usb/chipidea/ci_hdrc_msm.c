/* Copyright (c) 2010, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/usb/msm_hsusb_hw.h>
#include <linux/usb/ulpi.h>
#include <linux/usb/gadget.h>
#include <linux/usb/chipidea.h>

#include "ci.h"

#define MSM_USB_BASE	(ci->hw_bank.abs)

#define MDM9X15_WAKE_GPIO_WORKAROUND
#ifdef MDM9X15_WAKE_GPIO_WORKAROUND
#include <linux/gpio.h>
struct ci_hdrc_msm_context {
	int wake_gpio;
	int irq;
	bool wake_irq_state;
};

static void ci_hdrc_msm_suspend(struct ci_hdrc *ci)
{
	struct ci_hdrc_msm_context *c;

	c = (struct ci_hdrc_msm_context*)ci->platdata->context;
	if (c && c->irq && !c->wake_irq_state) {
	        enable_irq_wake(c->irq);
	        enable_irq(c->irq);
	        c->wake_irq_state = true;
	}
	if (ci->transceiver)
		usb_phy_set_suspend(ci->transceiver, 1);
}

static void ci_hdrc_msm_resume(struct ci_hdrc *ci)
{
	struct ci_hdrc_msm_context *c;

	c = (struct ci_hdrc_msm_context*)ci->platdata->context;
	if (c && c->irq && c->wake_irq_state) {
	        disable_irq_wake(c->irq);
#ifndef CONFIG_SIERRA_USB_OTG
	        disable_irq(c->irq);
#else
	        disable_irq_nosync(c->irq);
#endif
	        c->wake_irq_state = false;
	}
	if (ci->transceiver)
		usb_phy_set_suspend(ci->transceiver, 0);
}

static irqreturn_t ci_hdrc_msm_resume_irq(int irq, void *data)
{
	struct ci_hdrc *ci = (struct ci_hdrc *)data;

	if (ci->transceiver && ci->vbus_active && ci->suspended)
	        usb_phy_set_suspend(ci->transceiver, 0);
	else if (!ci->suspended)
	        ci_hdrc_msm_resume(ci);

	return IRQ_HANDLED;
}

#ifndef CONFIG_SIERRA_USB_OTG
#define WAKE_GPIO_IRQFLAGS (IRQF_TRIGGER_HIGH | IRQF_ONESHOT)
#else
#define WAKE_GPIO_IRQFLAGS (IRQF_TRIGGER_RISING | IRQF_ONESHOT)
#endif
static int ci_hdrc_msm_install_wake_gpio(struct ci_hdrc *ci)
{
	struct resource *res;
	struct ci_hdrc_msm_context *c;
	int ret = 0;

	c = (struct ci_hdrc_msm_context*)kmalloc(sizeof(*c), GFP_KERNEL);
	if (!c) {
		pr_err("%s: cannot allocate %d context bytes\n",
			__func__, sizeof(*c));
		return -ENOMEM;
	}

	res = platform_get_resource_byname(to_platform_device(ci->dev),
	                                   IORESOURCE_IO, "USB_RESUME");
	if (!res) {
	        pr_err("%s: no USB_RESUME GPIO\n", __func__);
	        return -ENXIO;
	}
	c->wake_gpio = res->start;
	gpio_request(c->wake_gpio, "USB_RESUME");
	gpio_direction_input(c->wake_gpio);
	c->irq = gpio_to_irq(c->wake_gpio);
	if (c->irq < 0) {
	        dev_err(ci->dev, "no interrupt for GPIO%d\n", c->wake_gpio);
	        return -ENXIO;
	}

	ret = request_irq(c->irq, ci_hdrc_msm_resume_irq,
	        WAKE_GPIO_IRQFLAGS, "usb resume", ci);
	if (ret < 0) {
		gpio_free(c->wake_gpio);
		kfree(c);
		/* just in case... */
		ci->platdata->context = NULL;
	        dev_err(ci->dev, "cannot register USB resume IRQ%d\n", c->irq);
	        return ret;
	}
	disable_irq(c->irq);
	c->wake_irq_state = false;
	ci->platdata->context = c;

	return ret;
}
#else
static inline void ci_hdrc_msm_suspend(struct ci_hdrc *ci) {}
static inline void ci_hdrc_msm_resume(struct ci_hdrc *ci) {}
static inline int ci_hdrc_msm_install_wake_gpio(struct ci_hdrc *ci) {return 0;}
#endif

static void ci_hdrc_msm_notify_event(struct ci_hdrc *ci, unsigned event)
{
	struct device *dev = ci->gadget.dev.parent;

	switch (event) {
	case CI_HDRC_CONTROLLER_RESET_EVENT:
		dev_info(dev, "CI_HDRC_CONTROLLER_RESET_EVENT received\n");
		writel(0, USB_AHBBURST);
		writel(0, USB_AHBMODE);
		usb_phy_init(ci->transceiver);
#ifdef CONFIG_SIERRA_USB_OTG
		/* try wake GPIO IRQ installation here instead at probe time
		 * since hw_device_reset may toggle the GPIO and cause
		 * ci_hdrc_msm_resume_irq as soon as the IRQ is enabled at
		 * ci_hdrc_msm_suspend
		 */
		if (ci->platdata && !ci->platdata->context) {
			/* USB wakeup work-around for MDM9615 */
			ci_hdrc_msm_install_wake_gpio(ci);
		}
#endif
		break;
	case CI_HDRC_CONTROLLER_STOPPED_EVENT:
		dev_info(dev, "CI_HDRC_CONTROLLER_STOPPED_EVENT received\n");
		/*
		 * Put the transceiver in non-driving mode. Otherwise host
		 * may not detect soft-disconnection.
		 */
		usb_phy_notify_disconnect(ci->transceiver, USB_SPEED_UNKNOWN);
		break;
	case CI_HDRC_CONTROLLER_SUSPEND_EVENT:
		dev_info(dev, "CI_HDRC_CONTROLLER_SUSPEND_EVENT received\n");
		ci_hdrc_msm_suspend(ci);
	        break;
	case CI_HDRC_CONTROLLER_RESUME_EVENT:
		dev_info(dev, "CI_HDRC_CONTROLLER_RESUME_EVENT received\n");
		ci_hdrc_msm_resume(ci);
	        break;
	default:
		dev_info(dev, "unknown ci_hdrc event\n");
		break;
	}
}

static struct ci_hdrc_platform_data ci_hdrc_msm_platdata = {
	.name			= "ci_hdrc_msm",
	.capoffset		= DEF_CAPOFFSET,
	.flags			= CI_HDRC_REGS_SHARED |
				  CI_HDRC_REQUIRE_TRANSCEIVER | CI_HDRC_DUAL_ROLE_NOT_OTG |
				  CI_HDRC_DISABLE_STREAMING,
	.dr_mode 			= USB_DR_MODE_PERIPHERAL,
	.notify_event		= ci_hdrc_msm_notify_event,
	.context		= NULL,
};

static int ci_hdrc_msm_probe(struct platform_device *pdev)
{
	struct platform_device *plat_ci;

	dev_dbg(&pdev->dev, "ci_hdrc_msm_probe\n");

	struct ci_hdrc_platform_data_android *pdata_android =
			(struct ci_hdrc_platform_data_android*) pdev->dev.platform_data;
	ci_hdrc_msm_platdata.usb_core_id = pdata_android->usb_core_id;

	plat_ci = ci_hdrc_add_device(&pdev->dev,
				pdev->resource, pdev->num_resources,
				&ci_hdrc_msm_platdata);
	if (IS_ERR(plat_ci)) {
		dev_err(&pdev->dev, "ci_hdrc_add_device failed!\n");
		return PTR_ERR(plat_ci);
	}

	platform_set_drvdata(pdev, plat_ci);

#ifndef CONFIG_SIERRA_USB_OTG
	/* USB wakeup work-around for MDM9615 */
	ci_hdrc_msm_install_wake_gpio(platform_get_drvdata(plat_ci));
#endif

	pm_runtime_no_callbacks(&pdev->dev);
	pm_runtime_enable(&pdev->dev);

	return 0;
}

static int ci_hdrc_msm_remove(struct platform_device *pdev)
{
	struct platform_device *plat_ci = platform_get_drvdata(pdev);

	pm_runtime_disable(&pdev->dev);
	ci_hdrc_remove_device(plat_ci);

	return 0;
}

static struct platform_driver ci_hdrc_msm_driver = {
	.probe = ci_hdrc_msm_probe,
	.remove = ci_hdrc_msm_remove,
	.driver = { .name = "msm_hsusb", },
};

module_platform_driver(ci_hdrc_msm_driver);

MODULE_ALIAS("platform:msm_hsusb");
MODULE_ALIAS("platform:ci13xxx_msm");
MODULE_LICENSE("GPL v2");
