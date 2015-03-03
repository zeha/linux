/* arch/arm/mach-msm/clock.c
 *
 * Copyright (C) 2007 Google, Inc.
 * Copyright (c) 2007-2012, The Linux Foundation. All rights reserved.
 *
 * This software is licensed under the terms of the GNU General Public
 * License version 2, as published by the Free Software Foundation, and
 * may be copied, distributed, and modified under those terms.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/err.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/module.h>
#include <linux/clk.h>
#include <linux/clkdev.h>
#include <linux/list.h>
#include <trace/events/power.h>

#include "clock.h"

struct handoff_clk {
	struct list_head list;
	struct clk *clk;
};
static LIST_HEAD(handoff_list);

/* Find the voltage level required for a given rate. */
static int find_vdd_level(struct clk *clk, unsigned long rate)
{
	int level;

	for (level = 0; level < ARRAY_SIZE(clk->fmax); level++)
		if (rate <= clk->fmax[level])
			break;

	if (level == ARRAY_SIZE(clk->fmax)) {
		pr_err("Rate %lu for %s is greater than highest Fmax\n", rate,
			clk->dbg_name);
		return -EINVAL;
	}

	return level;
}

/* Update voltage level given the current votes. */
static int update_vdd(struct clk_vdd_class *vdd_class)
{
	int level, rc;

	for (level = ARRAY_SIZE(vdd_class->level_votes)-1; level > 0; level--)
		if (vdd_class->level_votes[level])
			break;

	if (level == vdd_class->cur_level)
		return 0;

	rc = vdd_class->set_vdd(vdd_class, level);
	if (!rc)
		vdd_class->cur_level = level;

	return rc;
}

/* Vote for a voltage level. */
int vote_vdd_level(struct clk_vdd_class *vdd_class, int level)
{
	unsigned long flags;
	int rc;

	spin_lock_irqsave(&vdd_class->lock, flags);
	vdd_class->level_votes[level]++;
	rc = update_vdd(vdd_class);
	if (rc)
		vdd_class->level_votes[level]--;
	spin_unlock_irqrestore(&vdd_class->lock, flags);

	return rc;
}

/* Remove vote for a voltage level. */
int unvote_vdd_level(struct clk_vdd_class *vdd_class, int level)
{
	unsigned long flags;
	int rc = 0;

	spin_lock_irqsave(&vdd_class->lock, flags);
	if (WARN(!vdd_class->level_votes[level],
			"Reference counts are incorrect for %s level %d\n",
			vdd_class->class_name, level))
		goto out;
	vdd_class->level_votes[level]--;
	rc = update_vdd(vdd_class);
	if (rc)
		vdd_class->level_votes[level]++;
out:
	spin_unlock_irqrestore(&vdd_class->lock, flags);
	return rc;
}

/* Vote for a voltage level corresponding to a clock's rate. */
static int vote_rate_vdd(struct clk *clk, unsigned long rate)
{
	int level;

	if (!clk->vdd_class)
		return 0;

	level = find_vdd_level(clk, rate);
	if (level < 0)
		return level;

	return vote_vdd_level(clk->vdd_class, level);
}

/* Remove vote for a voltage level corresponding to a clock's rate. */
static void unvote_rate_vdd(struct clk *clk, unsigned long rate)
{
	int level;

	if (!clk->vdd_class)
		return;

	level = find_vdd_level(clk, rate);
	if (level < 0)
		return;

	unvote_vdd_level(clk->vdd_class, level);
}

int clk_reset(struct clk *clk, enum clk_reset_action action)
{
	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	if (!clk->ops->reset)
		return -ENOSYS;

	return clk->ops->reset(clk, action);
}
EXPORT_SYMBOL(clk_reset);

int clk_set_max_rate(struct clk *clk, unsigned long rate)
{
	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;

	if (!clk->ops->set_max_rate)
		return -ENOSYS;

	return clk->ops->set_max_rate(clk, rate);
}
EXPORT_SYMBOL(clk_set_max_rate);


int clk_set_flags(struct clk *clk, unsigned long flags)
{
	if (IS_ERR_OR_NULL(clk))
		return -EINVAL;
	if (!clk->ops->set_flags)
		return -ENOSYS;

	return clk->ops->set_flags(clk, flags);
}
EXPORT_SYMBOL(clk_set_flags);

static struct clock_init_data __initdata *clk_init_data;

static enum handoff __init __handoff_clk(struct clk *clk)
{
	enum handoff ret;
	struct handoff_clk *h;
	unsigned long rate;
	int err = 0;

	/*
	 * Tree roots don't have parents, but need to be handed off. So,
	 * terminate recursion by returning "enabled". Also return "enabled"
	 * for clocks with non-zero enable counts since they must have already
	 * been handed off.
	 */
	if (clk == NULL || clk->count)
		return HANDOFF_ENABLED_CLK;

	/* Clocks without handoff functions are assumed to be disabled. */
	if (!clk->ops->handoff || (clk->flags & CLKFLAG_SKIP_HANDOFF))
		return HANDOFF_DISABLED_CLK;

	/*
	 * Handoff functions for children must be called before their parents'
	 * so that the correct parent is returned by the clk_get_parent() below.
	 */
	ret = clk->ops->handoff(clk);
	if (ret == HANDOFF_ENABLED_CLK) {
		ret = __handoff_clk(clk_get_parent(clk));
		if (ret == HANDOFF_ENABLED_CLK) {
			h = kmalloc(sizeof(*h), GFP_KERNEL);
			if (!h) {
				err = -ENOMEM;
				goto out;
			}
			err = clk_prepare_enable(clk);
			if (err)
				goto out;
			rate = clk_get_rate(clk);
			if (rate)
				pr_debug("%s rate=%lu\n", clk->dbg_name, rate);
			h->clk = clk;
			list_add_tail(&h->list, &handoff_list);
		}
	}
out:
	if (err) {
		pr_err("%s handoff failed (%d)\n", clk->dbg_name, err);
		kfree(h);
		ret = HANDOFF_DISABLED_CLK;
	}
	return ret;
}

void __init msm_clock_init(struct clock_init_data *data)
{
	unsigned n;
	struct clk_lookup *clock_tbl;
	size_t num_clocks;
	struct clk *clk;

	clk_init_data = data;
	if (clk_init_data->pre_init)
		clk_init_data->pre_init();

	clock_tbl = data->table;
	num_clocks = data->size;

	for (n = 0; n < num_clocks; n++) {
		struct clk *parent;
		clk = clock_tbl[n].clk;
		parent = clk_get_parent(clk);
		if (parent && list_empty(&clk->siblings))
			list_add(&clk->siblings, &parent->children);
	}

	/*
	 * Detect and preserve initial clock state until clock_late_init() or
	 * a driver explicitly changes it, whichever is first.
	 */
	for (n = 0; n < num_clocks; n++)
		__handoff_clk(clock_tbl[n].clk);

	clkdev_add_table(clock_tbl, num_clocks);

	if (clk_init_data->post_init)
		clk_init_data->post_init();
}

/*
 * The bootloader and/or AMSS may have left various clocks enabled.
 * Disable any clocks that have not been explicitly enabled by a
 * clk_enable() call and don't have the CLKFLAG_SKIP_AUTO_OFF flag.
 */
static int __init clock_late_init(void)
{
	unsigned n, count = 0;
	struct handoff_clk *h, *h_temp;
	unsigned long flags;
	int ret = 0;

	clock_debug_init(clk_init_data);
	for (n = 0; n < clk_init_data->size; n++) {
		struct clk *clk = clk_init_data->table[n].clk;

		clock_debug_add(clk);
		spin_lock_irqsave(&clk->lock, flags);
		if (!(clk->flags & CLKFLAG_SKIP_AUTO_OFF)) {
			if (!clk->count && clk->ops->auto_off) {
				count++;
				clk->ops->auto_off(clk);
			}
		}
		spin_unlock_irqrestore(&clk->lock, flags);
	}
	pr_info("clock_late_init() disabled %d unused clocks\n", count);

	list_for_each_entry_safe(h, h_temp, &handoff_list, list) {
		clk_disable_unprepare(h->clk);
		list_del(&h->list);
		kfree(h);
	}

	if (clk_init_data->late_init)
		ret = clk_init_data->late_init();
	return ret;
}
late_initcall(clock_late_init);
