// SPDX-License-Identifier: GPL-2.0
/* HACK pribadi — minimal AW87519 enable, profil "kspk" (speaker utama/AUX_OUT).
 * Tidak untuk upstream. Register table diambil dari downstream techpack/audio. */
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/gpio/consumer.h>
#include <linux/of.h>

#define AW87519_REG_CHIPID	0x00
#define AW87519_CHIPID		0x59

static const u8 aw87519_kspk_cfg[] = {
	0x69, 0x80,
	0x69, 0xB7,
	0x01, 0xF0,
	0x02, 0x09,
	0x03, 0xE8,
	0x04, 0x11,
	0x05, 0x10,
	0x06, 0x43,
	0x07, 0x4E,
	0x08, 0x03,
	0x09, 0x08,
	0x0A, 0x4A,
	0x60, 0x16,
	0x61, 0x20,
	0x62, 0x01,
	0x63, 0x0B,
	0x64, 0xC5,
	0x65, 0xA4,
	0x66, 0x78,
	0x67, 0xC4,
	0x68, 0x90,
};

static int aw87519_min_probe(struct i2c_client *client)
{
	struct gpio_desc *reset_gpio;
	int ret, i;
	u8 chipid;

	reset_gpio = devm_gpiod_get_optional(&client->dev, "reset", GPIOD_OUT_LOW);
	if (IS_ERR(reset_gpio))
		return PTR_ERR(reset_gpio);

	if (reset_gpio) {
		gpiod_set_value_cansleep(reset_gpio, 0);
		usleep_range(2000, 2500);
		gpiod_set_value_cansleep(reset_gpio, 1);
		usleep_range(2000, 2500);
	}

	ret = i2c_smbus_read_byte_data(client, AW87519_REG_CHIPID);
	if (ret < 0) {
		dev_err(&client->dev, "gagal baca chipid: %d\n", ret);
		return ret;
	}
	chipid = ret;
	if (chipid != AW87519_CHIPID) {
		dev_err(&client->dev, "chipid tidak cocok: 0x%02x (harusnya 0x%02x)\n",
			chipid, AW87519_CHIPID);
		return -ENODEV;
	}
	dev_info(&client->dev, "AW87519 chipid OK: 0x%02x\n", chipid);

	for (i = 0; i < ARRAY_SIZE(aw87519_kspk_cfg); i += 2) {
		ret = i2c_smbus_write_byte_data(client,
				aw87519_kspk_cfg[i], aw87519_kspk_cfg[i + 1]);
		if (ret < 0)
			dev_warn(&client->dev, "write reg 0x%02x gagal: %d\n",
				 aw87519_kspk_cfg[i], ret);
		usleep_range(1000, 1200);
	}

	dev_info(&client->dev, "AW87519 enabled (profil kspk/speaker utama, HACK pribadi)\n");
	return 0;
}

static const struct of_device_id aw87519_min_of_match[] = {
	{ .compatible = "awinic,aw87519_pa" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw87519_min_of_match);

static struct i2c_driver aw87519_min_driver = {
	.driver = {
		.name = "aw87519_min",
		.of_match_table = aw87519_min_of_match,
	},
	.probe = aw87519_min_probe,
};
module_i2c_driver(aw87519_min_driver);

MODULE_DESCRIPTION("AW87519 minimal enable hack (personal, not for upstream)");
MODULE_LICENSE("GPL");
