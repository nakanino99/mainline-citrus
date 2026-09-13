// SPDX-License-Identifier: GPL-2.0
/* HACK pribadi — minimal AW87359 enable, profil "drcv" (earpiece/EAR_OUT).
 * Tidak untuk upstream. Register table diambil dari downstream techpack/audio. */
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/delay.h>
#include <linux/of.h>

#define AW87359_REG_CHIPID	0x00
#define AW87359_CHIPID		0x59

static const u8 aw87359_drcv_cfg[] = {
	0x70, 0x80,
	0x01, 0x00,
	0x01, 0x00,
	0x02, 0x00,
	0x03, 0x08,
	0x04, 0x05,
	0x05, 0x00,
	0x06, 0x0F,
	0x07, 0x4E,
	0x08, 0x09,
	0x09, 0x08,
	0x0A, 0x4B,
	0x61, 0xBB,
	0x62, 0x80,
	0x63, 0x29,
	0x64, 0x58,
	0x65, 0xCD,
	0x66, 0x80,
	0x67, 0x2F,
	0x68, 0x07,
	0x69, 0xDB,
	0x01, 0x0D,
};

static int aw87359_min_probe(struct i2c_client *client)
{
	int ret, i;
	u8 chipid;

	ret = i2c_smbus_read_byte_data(client, AW87359_REG_CHIPID);
	if (ret < 0) {
		dev_err(&client->dev, "gagal baca chipid: %d\n", ret);
		return ret;
	}
	chipid = ret;
	if (chipid != AW87359_CHIPID) {
		dev_err(&client->dev, "chipid tidak cocok: 0x%02x (harusnya 0x%02x)\n",
			chipid, AW87359_CHIPID);
		return -ENODEV;
	}
	dev_info(&client->dev, "AW87359 chipid OK: 0x%02x\n", chipid);

	for (i = 0; i < ARRAY_SIZE(aw87359_drcv_cfg); i += 2) {
		ret = i2c_smbus_write_byte_data(client,
				aw87359_drcv_cfg[i], aw87359_drcv_cfg[i + 1]);
		if (ret < 0)
			dev_warn(&client->dev, "write reg 0x%02x gagal: %d\n",
				 aw87359_drcv_cfg[i], ret);
		usleep_range(1000, 1200);
	}

	dev_info(&client->dev, "AW87359 enabled (profil drcv/earpiece, HACK pribadi)\n");
	return 0;
}

static const struct of_device_id aw87359_min_of_match[] = {
	{ .compatible = "awinic,aw87359_pa" },
	{ }
};
MODULE_DEVICE_TABLE(of, aw87359_min_of_match);

static struct i2c_driver aw87359_min_driver = {
	.driver = {
		.name = "aw87359_min",
		.of_match_table = aw87359_min_of_match,
	},
	.probe = aw87359_min_probe,
};
module_i2c_driver(aw87359_min_driver);

MODULE_DESCRIPTION("AW87359 minimal enable hack (personal, not for upstream)");
MODULE_LICENSE("GPL");
