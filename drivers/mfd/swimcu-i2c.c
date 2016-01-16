/*
 * swimcu-i2c.c  --  Generic I2C driver for Sierra Wireless WPx5 MCU
 *
 * adapted from:
 * wm8350-i2c.c  --  Generic I2C driver for Wolfson WM8350 PMIC
 *
 * Copyright (c) 2016 Sierra Wireless, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/err.h>
#include <linux/init.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/mfd/swimcu/core.h>
#include <linux/regmap.h>
#include <linux/slab.h>

static int swimcu_i2c_probe(struct i2c_client *i2c,
			    const struct i2c_device_id *id)
{
	struct swimcu *swimcu;

	swimcu_log(INIT, "%s: start %lu\n", __func__, id->driver_data);
	swimcu = devm_kzalloc(&i2c->dev, sizeof(struct swimcu), GFP_KERNEL);
	if (swimcu == NULL)
		return -ENOMEM;

	i2c_set_clientdata(i2c, swimcu);

	swimcu->dev = &i2c->dev;
	swimcu->client = i2c;
	swimcu->i2c_driver_id = id->driver_data;

	return swimcu_device_init(swimcu);
}

static int swimcu_i2c_remove(struct i2c_client *i2c)
{
	struct swimcu *swimcu = i2c_get_clientdata(i2c);

	swimcu_device_exit(swimcu);

	return 0;
}

static const struct i2c_device_id swimcu_i2c_id[] = {
       { "swimcu", SWIMCU_APPL_I2C_ID },
       { }
};
MODULE_DEVICE_TABLE(i2c, swimcu_i2c_id);


static struct i2c_driver swimcu_appl_i2c_driver = {
	.driver = {
		   .name = "swimcu",
		   .owner = THIS_MODULE,
	},
	.probe = swimcu_i2c_probe,
	.remove = swimcu_i2c_remove,
	.id_table = swimcu_i2c_id,
};

static int __init swimcu_i2c_init(void)
{
	swimcu_log(INIT, "%s: start\n", __func__);

	return i2c_add_driver(&swimcu_appl_i2c_driver);
}
/* init early so consumer devices can complete system boot */
subsys_initcall(swimcu_i2c_init);

static void __exit swimcu_i2c_exit(void)
{
	i2c_del_driver(&swimcu_appl_i2c_driver);
}
module_exit(swimcu_i2c_exit);

MODULE_DESCRIPTION("I2C support for the Sierra Wireless WPx5 MCU");
MODULE_LICENSE("GPL");
