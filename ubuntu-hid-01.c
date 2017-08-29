/*
 *  Sample hotkey driver for imaginary device UBT0001
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
MODULE_ALIAS("acpi*:UBT0001:*");

static struct input_dev *input_dev;

static const struct acpi_device_id ubt0001_ids[] = {
	{"UBT0001", 0},
	{"", 0},
};

static const struct key_entry sample_keymap[] = {
	{ KE_KEY, 0x80, { KEY_VOLUMEUP } },
	{ KE_KEY, 0x81, { KEY_VOLUMEDOWN} },
	{ KE_KEY, 0x82, { KEY_RFKILL } },
	{ KE_END },
};

static int ubt0001_input_setup(struct acpi_device *device)
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

static void ubt0001_notify(struct acpi_device *acpi_dev, u32 event)
{
	if (!sparse_keymap_report_event(input_dev, event, 1, true))
		dev_info(&acpi_dev->dev, "unknown event index 0x%x\n", event);
}

static int ubt0001_add(struct acpi_device *device)
{
	int err;

	err = ubt0001_input_setup(device);
	if (err) {
		pr_err("Failed to setup sample input device\n");
		return err;
	}

	return sample_hotkey_enable(device, true);
}

static int ubt0001_remove(struct acpi_device *device)
{
	return sample_hotkey_enable(device, false);
}

static struct acpi_driver ubt0001_driver = {
	.name	= "sample-hotkey",
	.owner	= THIS_MODULE,
	.ids	= ubt0001_ids,
	.ops	= {
		.add	= ubt0001_add,
		.remove	= ubt0001_remove,
		.notify	= ubt0001_notify,
	},
};

module_acpi_driver(ubt0001_driver);
