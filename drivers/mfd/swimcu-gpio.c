/*
 * swimcu-gpio.c  --  Device access for Sierra Wireless WPx5 MCU.
 *
 * adapted from:
 * wm8350-core.c  --  Device access for Wolfson WM8350
 *
 * Copyright (c) 2016 Sierra Wireless, Inc.
 *
 *  This program is free software; you can redistribute  it and/or modify it
 *  under  the terms of  the GNU General  Public License as published by the
 *  Free Software Foundation;  either version 2 of the  License, or (at your
 *  option) any later version.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/errno.h>
#include<linux/string.h>

#include <linux/mfd/swimcu/core.h>
#include <linux/mfd/swimcu/gpio.h>
#include <linux/mfd/swimcu/mciprotocol.h>
#include <linux/mfd/swimcu/mcidefs.h>

static enum mci_pin_irqc_type_e gpio_irqc[SWIMCU_NUM_GPIO_IRQ] = {
	MCI_PIN_IRQ_DISABLED,
	MCI_PIN_IRQ_DISABLED,
	MCI_PIN_IRQ_DISABLED
};

/* WPx5 GPIOs provided by MCU occur in the range 34 - 41
 * external gpio number is the offset from GPIO34 */
static const struct {
	int port;
	int pin;
	enum mci_pin_irqc_type_e *irqc_type;
} gpio_map[] = {
	{ 0, 4, NULL },   /* GPIO 34 = PTA4 */
	{ 0, 3, NULL },   /* GPIO 35 = PTA3 */
	{ 0, 0, &gpio_irqc[SWIMCU_GPIO_PTA0_IRQ] }, /* GPIO 36 = PTA0 */
	{ 0, 2, NULL },   /* GPIO 37 = PTA2 */
	{ 1, 0, &gpio_irqc[SWIMCU_GPIO_PTB0_IRQ] }, /* GPIO 38 = PTB0 */
	{ 0, 7, &gpio_irqc[SWIMCU_GPIO_PTA7_IRQ] }, /* GPIO 39 = PTA7 */
	{ 0, 6, NULL },   /* GPIO 40 = PTA6 */
	{ 0, 5, NULL }    /* GPIO 41 = PTA5 */
};

/************
 *
 * Name:     swimcu_get_gpio_from_port_pin
 *
 * Purpose:  return index into gpio_map given port and pin
 *
 * Parms:    port - 0 or 1 corresponding to PTA or PTB
 *           pin - 0 to 7 for pin # on that port
 *
 * Return:   gpio index - 0 to 7 on success
 *           8 on failure
 *
 * Abort:    none
 *
 ************/
enum swimcu_gpio_index swimcu_get_gpio_from_port_pin(int port, int pin)
{
	enum swimcu_gpio_index gpio;

	for (gpio = SWIMCU_GPIO_FIRST; gpio <= SWIMCU_GPIO_LAST; gpio++) {
		if((port == gpio_map[gpio].port) && (pin == gpio_map[gpio].pin))
			break;
	}
	return gpio;
}

/************
 *
 * Name:     swimcu_gpio_callback
 *
 * Purpose:  callback for gpio irq event (stub for now)
 *
 * Parms:    swimcu - device data struct
 *           port - 0 or 1 corresponding to PTA or PTB
 *           pin - 0 to 7 for pin # on that port
 *           level - 0 or 1
 *
 * Return:   nothing
 *
 * Abort:    none
 *
 ************/
void swimcu_gpio_callback(struct swimcu *swimcu, int port, int pin, int level)
{
	enum swimcu_gpio_index gpio = swimcu_get_gpio_from_port_pin(port, pin);

	swimcu_log(GPIO, "%s: gpio %d level = %d\n", __func__, gpio, level);
}

/************
 *
 * Name:     swimcu_gpio_set_trigger
 *
 * Purpose:  set the irq trigger for a particular gpio
 *
 * Parms:    gpio index - 0 to 7
 *           irq control type (rising, falling, etc).
 *
 * Return:   nothing
 *
 * Abort:    none
 *
 ************/
void swimcu_gpio_set_trigger(int gpio, enum mci_pin_irqc_type_e irqc_type)
{
	if (gpio < SWIMCU_NUM_GPIO) {
		if( gpio_map[gpio].irqc_type != NULL ) {
			*gpio_map[gpio].irqc_type = irqc_type;
			swimcu_log(GPIO, "%s: gpio %d irq = %d\n", __func__, gpio, irqc_type);
		}
	}
	else {
		pr_err("%s: gpio %d out of range\n", __func__, gpio);
	}
}

/************
 *
 * Name:     swimcu_gpio_get_trigger
 *
 * Purpose:  get the irq trigger for a particular gpio
 *
 * Parms:    gpio index - 0 to 7
 *
 * Return:   irq control type (rising, falling, etc).
 *
 * Abort:    none
 *
 ************/
enum mci_pin_irqc_type_e swimcu_gpio_get_trigger(int gpio)
{
	enum mci_pin_irqc_type_e irqc_type = MCI_PIN_IRQ_DISABLED;

	if (gpio < SWIMCU_NUM_GPIO) {
		if( gpio_map[gpio].irqc_type != NULL ) {
			irqc_type = *gpio_map[gpio].irqc_type;
			swimcu_log(GPIO, "%s: gpio %d irq = %d\n", __func__, gpio, irqc_type);
		}
	}
	return irqc_type;
}

/************
 *
 * Name:     swimcu_gpio_get
 *
 * Purpose:  get the gpio property
 *
 * Parms:    swimcu - device data struct
 *           action - get value is only option
 *           gpio index - 0 to 7
 *           value ptr - returns value
 *
 * Return:   0 if success
 *           -ve otherwise
 *
 * Abort:    none
 *
 * Notes:    called from gpio driver
 *
 ************/
int swimcu_gpio_get(struct swimcu *swimcu, int action, int gpio, int *value)
{
	struct mci_mcu_pin_state_s pin_state;
	int ret = 0;

	memset((void*) &pin_state, 0, sizeof(struct mci_mcu_pin_state_s));

	*value = 0;

	switch (action) {
		case SWIMCU_GPIO_GET_VAL:
			swimcu_log(GPIO, "%s: GET VAL gpio%d\n", __func__, gpio);
			if (MCI_PROTOCOL_STATUS_CODE_SUCCESS ==
				swimcu_pin_states_get(swimcu, gpio_map[gpio].port, gpio_map[gpio].pin, &pin_state)) {
				*value = (pin_state.level == MCI_MCU_PIN_LEVEL_LOW) ? 0 : 1;
			}
			else {
				ret = -EIO;
			}
			break;

		default:
			ret = -EINVAL;
			pr_err ("%s: unknown %d\n", __func__, gpio);
	}
	return ret;
}

/************
 *
 * Name:     swimcu_gpio_set
 *
 * Purpose:  set the gpio property
 *
 * Parms:    swimcu - device data struct
 *           action - property to set
 *           gpio index - 0 to 7
 *           value - value to set (depends on action)
 *
 * Return:   0 if success
 *           -ve otherwise
 *
 * Abort:    none
 *
 * Notes:    called from gpio driver
 *
 ************/
int swimcu_gpio_set(struct swimcu *swimcu, int action, int gpio, int value)
{
	struct mci_mcu_pin_state_s pin_state;
	enum mci_pin_irqc_type_e irqc_type;
	enum mci_mcu_pin_direction_e direction;
	int ret = 0;

	memset((void*) &pin_state, 0, sizeof(struct mci_mcu_pin_state_s));

	if (gpio_map[gpio].irqc_type != NULL)
		irqc_type = *gpio_map[gpio].irqc_type;
	else
		irqc_type = MCI_PIN_IRQ_DISABLED;

	if (MCI_PROTOCOL_STATUS_CODE_SUCCESS != swimcu_pin_states_get(swimcu, gpio_map[gpio].port, gpio_map[gpio].pin, &pin_state))
		ret = -EIO;

	if (pin_state.mux != MCI_MCU_PIN_FUNCTION_GPIO)
	{
		pr_warn("%s: uninitialized gpio%d\n",__func__, gpio);

		pin_state.mux = MCI_MCU_PIN_FUNCTION_GPIO;
		pin_state.dir = MCI_MCU_PIN_DIRECTION_INPUT;
		pin_state.params.input.pe = false;
		pin_state.params.input.ps = (irqc_type == MCI_PIN_IRQ_LOGIC_ZERO) ? MCI_MCU_PIN_PULL_UP : MCI_MCU_PIN_PULL_DOWN;
		pin_state.params.input.pfe = false;
		pin_state.params.input.irqc_type = irqc_type;
	}

	switch (action) {
		case SWIMCU_GPIO_SET_DIR:
			swimcu_log(GPIO, "%s: SET DIR %d, port %d, pin%d\n", __func__, value, gpio_map[gpio].port, gpio_map[gpio].pin);
			/* value: 0-input, 1-output,low, 2-output,high */
			direction = (value > 0) ? MCI_MCU_PIN_DIRECTION_OUTPUT : MCI_MCU_PIN_DIRECTION_INPUT;
			if ( pin_state.dir != direction) {
				swimcu_log(GPIO, "%s: change DIR %d to %d\n", __func__, pin_state.dir, value);
			}
			else {
				swimcu_log(GPIO, "%s: no change DIR %d\n", __func__, value);
			}

			if (direction == MCI_MCU_PIN_DIRECTION_INPUT) {
				pin_state.params.input.pe = false;
				pin_state.params.input.ps = (irqc_type == MCI_PIN_IRQ_LOGIC_ZERO)
					? MCI_MCU_PIN_PULL_UP : MCI_MCU_PIN_PULL_DOWN;
				pin_state.params.input.pfe = false;
				pin_state.params.input.irqc_type = irqc_type;
			}
			else {
				pin_state.params.output.sre = MCI_MCU_PIN_SLEW_RATE_FAST;
				pin_state.params.output.dse = MCI_MCU_PIN_DRIVE_STRENGTH_HIGH;
			}
			pin_state.dir = direction;
			pin_state.level = (value > 1) ? MCI_MCU_PIN_LEVEL_HIGH : MCI_MCU_PIN_LEVEL_LOW;

			break;

		case SWIMCU_GPIO_SET_VAL:
			if ( pin_state.dir != MCI_MCU_PIN_DIRECTION_OUTPUT ) {
				pr_err ("%s: VAL %d (is input, illegal operation)\n", __func__, value);
				ret = -EPERM;
			}
			else if ( pin_state.level != value ) {
				swimcu_log(GPIO, "%s: change VAL %d to %d\n", __func__, pin_state.level, value);
			}
			else {
				swimcu_log(GPIO, "%s: no change VAL %d\n", __func__, value);
			}
			pin_state.level = (enum mci_mcu_pin_level_e) value;

			break;

		case SWIMCU_GPIO_SET_PULL:
			if ( pin_state.dir != MCI_MCU_PIN_DIRECTION_INPUT ) {
				pr_err ("%s: PULL %d (is out, illegal operation)\n", __func__, value);
				ret = -EPERM;
			}
			else if ( pin_state.params.input.ps != (enum mci_mcu_pin_pull_select_e) value ) {
				swimcu_log(GPIO, "%s: change VAL %d to %d\n", __func__, pin_state.level, value);
			}
			else {
				swimcu_log(GPIO, "%s: no change VAL %d\n", __func__, value);
			}
			pin_state.params.input.pe = true;
			pin_state.params.input.ps = value ? MCI_MCU_PIN_PULL_UP : MCI_MCU_PIN_PULL_DOWN;

			break;

		default:
			ret = -EINVAL;
			pr_err ("%s: unknown %d\n", __func__, value);

	}

	if (0 == ret) {
		if (MCI_PROTOCOL_STATUS_CODE_SUCCESS != swimcu_pin_config_set( swimcu, gpio_map[gpio].port, gpio_map[gpio].pin, &pin_state ))
			ret = -EIO;
	}

	return ret;
}

/************
 *
 * Name:     swimcu_gpio_open
 *
 * Purpose:  initialize MCU pin as gpio
 *
 * Parms:    swimcu - device data struct
 *           gpio index - 0 to 7
 *
 * Return:   0 if success
 *           -ve otherwise
 *
 * Abort:    none
 *
 * Notes:    called from gpio driver on export
 *
 ************/
int swimcu_gpio_open( struct swimcu *swimcu, int gpio )
{
	struct mci_mcu_pin_state_s pin_state;
	enum mci_pin_irqc_type_e   irqc_type;
	enum mci_protocol_status_code_e ret;

	memset((void*) &pin_state, 0, sizeof(struct mci_mcu_pin_state_s));

	if (gpio_map[gpio].irqc_type != NULL)
		irqc_type = *gpio_map[gpio].irqc_type;
	else
		irqc_type = MCI_PIN_IRQ_DISABLED;

	pin_state.mux = MCI_MCU_PIN_FUNCTION_GPIO;
	pin_state.dir = MCI_MCU_PIN_DIRECTION_INPUT;
	pin_state.level = MCI_MCU_PIN_LEVEL_LOW;
	pin_state.params.input.pe = false;
	pin_state.params.input.ps = (irqc_type == MCI_PIN_IRQ_LOGIC_ZERO) ? MCI_MCU_PIN_PULL_UP : MCI_MCU_PIN_PULL_DOWN;
	pin_state.params.input.pfe = false;
	pin_state.params.input.irqc_type = irqc_type;

	ret = swimcu_pin_config_set( swimcu, gpio_map[gpio].port, gpio_map[gpio].pin, &pin_state );

	swimcu_log(GPIO, "%s: complete %d\n", __func__, ret);
	if (ret == MCI_PROTOCOL_STATUS_CODE_SUCCESS)
		return 0;
	return -EIO;
}

/************
 *
 * Name:     swimcu_gpio_close
 *
 * Purpose:  return MCU pin to uninitialized
 *
 * Parms:    swimcu - device data struct
 *           gpio index - 0 to 7
 *
 * Return:   nothing
 *
 * Abort:    none
 *
 * Notes:    called from gpio driver on unexport
 *
 ************/
int swimcu_gpio_close( struct swimcu *swimcu, int gpio )
{
	struct mci_mcu_pin_state_s pin_state;
	enum mci_protocol_status_code_e ret;

	memset((void*) &pin_state, 0, sizeof(struct mci_mcu_pin_state_s));

	pin_state.mux = MCI_MCU_PIN_FUNCTION_DISABLED;

	ret = swimcu_pin_config_set( swimcu, gpio_map[gpio].port, gpio_map[gpio].pin, &pin_state );

	swimcu_log(GPIO, "%s: complete %d\n", __func__, ret);
	if (ret == MCI_PROTOCOL_STATUS_CODE_SUCCESS)
		return 0;
	return -EIO;
}

