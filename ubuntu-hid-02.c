/*
 *  Sample hotkey driver for imaginary device UBT0002
 *
 *  Copyright (C) 2017 Alex Hung <alex.hung@ubuntu.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/input.h>
#include <linux/input/sparse-keymap.h>
#include <linux/acpi.h>
#include <acpi/acpi_bus.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Alex Hung");
MODULE_ALIAS("acpi*:UBT0002:*");

static struct input_dev *input_dev;

static const struct acpi_device_id ubt0002_ids[] = {
	{"UBT0002", 0},
	{"", 0},
};

static const struct key_entry sample_keymap[] = {
	{ KE_KEY, 0x80, { KEY_VOLUMEUP } },
	{ KE_KEY, 0x81, { KEY_VOLUMEDOWN} },
	{ KE_KEY, 0x82, { KEY_RFKILL } },
	{ KE_END },
};

static int ubt0002_input_setup(struct acpi_device *device)
{
	int ret;

	input_dev = devm_input_allocate_device(&device->dev);
	if (!input_dev)
		return -ENOMEM;

	ret = sparse_keymap_setup(input_dev, sample_keymap, NULL);
	if (ret)
		return ret;

	input_dev->name = "Sample hotkeys";
	input_dev->id.bustype = BUS_HOST;

	return input_register_device(input_dev);
}

static int sample_hotkey_enable(struct acpi_device *device, bool enable)
{
	acpi_status status;

	status = acpi_execute_simple_method(device, "ENAB", enable);
	if (ACPI_FAILURE(status)) {
		dev_warn(&device->dev, "failed to %sable hotkeys\n", enable ? "en" : "dis");
		return -EIO;
	}

	return 0;
}

static void ubt0002_notify(struct acpi_device *acpi_dev, u32 event)
{
	unsigned long long ev_index;
	acpi_status status;

	if (event != 0x80) {
		dev_info(&acpi_dev->dev, "unknown event 0x%x\n", event);
		return;
	}

	status = acpi_evaluate_integer(acpi_dev->handle, "GETK", NULL, &ev_index);
	if (ACPI_FAILURE(status)) {
		dev_warn(&acpi_dev->dev, "failed to get event index\n");
		return;
	}

	if (!sparse_keymap_report_event(input_dev, event, 1, true))
		dev_info(&acpi_dev->dev, "unknown event index 0x%x\n", ev_index);
}

static int ubt0002_add(struct acpi_device *device)
{
	int err;

	err = ubt0002_input_setup(device);
	if (err) {
		pr_err("Failed to setup sample input device\n");
		return err;
	}

	return sample_hotkey_enable(device, true);
}

static int ubt0002_remove(struct acpi_device *device)
{
	return sample_hotkey_enable(device, false);
}

static struct acpi_driver ubt0002_driver = {
	.name	= "sample-hotkey",
	.owner	= THIS_MODULE,
	.ids	= ubt0002_ids,
	.ops	= {
		.add	= ubt0002_add,
		.remove	= ubt0002_remove,
		.notify	= ubt0002_notify,
	},
};

module_acpi_driver(ubt0002_driver);
