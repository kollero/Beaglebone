/*
 * pcm1690-i2c.c  --  pcm1690DAC driver module - I2C
 *
 * Copyright 2017 
 * Author: change_one_day
 *
 */

#include <linux/module.h>
#include <linux/i2c.h>
#include <linux/regmap.h>

#include <sound/soc.h>

static int pcm1690_i2c_probe(struct i2c_client *i2c, const struct i2c_device_id *id)
{
	struct regmap_config config;
	
	config = pcm1690_regmap_config;
	
	return pcm1690_codec_probe(&i2c->dev, devm_regmap_init_i2c(i2c, &config));
}

static int pcm1690_i2c_remove(struct i2c_client *i2c)
{
	pcm1690_codec_remove(&i2c->dev);
	dev_info(&i2c->dev,"pcm1690_i2c removed %d\n", 1);
	return 0;
}

static const struct i2c_device_id pcm1690_i2c_id[] = {
	{"pcm1690", }, //0x4c + 0-4
	{}
};
MODULE_DEVICE_TABLE(i2c, pcm1690_i2c_id);

static const struct of_device_id pcm1690_of_match[] = {
	{ .compatible = "ti,pcm1690", },
	{ }
};
MODULE_DEVICE_TABLE(of, pcm1690_of_match);

static struct i2c_driver pcm1690_i2c_driver = {
	.driver = {
		.name	= "pcm1690",
		.of_match_table = pcm1690_of_match,
		//.pm 		= &pcm1690_pm_ops,
	},
	.probe		= pcm1690_i2c_probe,
	.remove		= pcm1690_i2c_remove,
	.id_table	= pcm1690_i2c_id,
	
};
module_i2c_driver(pcm1690_i2c_driver); 

MODULE_AUTHOR("add_one_day <kollero@hotmail.com> ");
MODULE_DESCRIPTION("ASoC pcm1690 codec driver - I2C");
MODULE_LICENSE("GPL v2");
