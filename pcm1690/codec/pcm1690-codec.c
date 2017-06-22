/*
 * pcm1690 ASoC codec driver
 * Copyright (c) 2017
 * Author: change_one_day
 *
 */

#define DEBUG 1

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <linux/delay.h>

#include <sound/soc-dapm.h>

#include "pcm1690.h"
#include "pcm1690-i2c.c"

// pcm1690 driver private data 
struct pcm1690_private { 
	struct device *dev;
	struct regmap *regmap;
	unsigned int format;
	unsigned int deemph; 	// Current deemphasis status 
	unsigned int rate;	// Current rate for deemphasis control
	
};

static const struct reg_default pcm1690_reg_defaults[] = {
	{ 0x41,	0x86}, // I2S TDM format 0b10000110=0x86, set default to 0x80, power save disabled
	{ 0x42,	0x80}, // sharp roll off, dac7/8 off 0b10000000=0x80
	{ 0x43,	0x00 }, //normal outputs
	{ 0x44,	0xc0 }, //soft mutes 7 and 8
	{ 0x46,	0x00 }, //
//	{ 0x47,	0x00 }, //not allowed
	{ 0x48,	0x4D }, //volume to 30% of max on first time, alsamixer will remember last settings
	{ 0x49,	0x4D },
	{ 0x4a,	0x4D },
	{ 0x4b,	0x4D },
	{ 0x4c,	0x4D },
	{ 0x4d,	0x4D },
	{ 0x4e,	0x00 }, //mute
	{ 0x4f,	0x00 }, //mute
	{ 0x40,	0b11000000},//on auto rate //set to dual rate 0x02
};


static const int pcm1690_deemph[] = {0, 48000, 44100, 32000 };

static int pcm1690_set_deemph(struct snd_soc_codec *codec)
{
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);
	int i = 0, val = -1, enable = 0;

	if (priv->deemph)
		for (i = 0; i < ARRAY_SIZE(pcm1690_deemph); i++)
			if (pcm1690_deemph[i] == priv->rate)
				val = i;

	if (val != -1) {
		regmap_update_bits(priv->regmap, PCM1690_DEEMPH_CONTROL,
				   PCM1690_DEEMPH_RATE_MASK, val << 4);
		enable = 1;
	} else
		enable = 0;

	/* enable/disable deemphasis functionality */
	return regmap_update_bits(priv->regmap, PCM1690_DEEMPH_CONTROL,
					PCM1690_DEEMPH_MASK, enable);
}

static int pcm1690_get_deemph(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);

	ucontrol->value.integer.value[0] = priv->deemph;

	return 0;
}

static int pcm1690_put_deemph(struct snd_kcontrol *kcontrol,
			      struct snd_ctl_elem_value *ucontrol)
{
	struct snd_soc_codec *codec = snd_soc_kcontrol_codec(kcontrol);
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);

	priv->deemph = ucontrol->value.integer.value[0];

	return pcm1690_set_deemph(codec);
}


/* ---------------------------------------------------------------------
 * ALSA controls ok
 */

static const DECLARE_TLV_DB_SCALE(pcm1690_dac_tlv, -6350, 50, 1);

static const struct snd_kcontrol_new pcm1690_controls[] = { //independent channel volume controls
	SOC_SINGLE_TLV("C1 Volume",PCM1690_ATT_CONTROL(1),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("C2 Volume",PCM1690_ATT_CONTROL(2),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("C3 Volume",PCM1690_ATT_CONTROL(3),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("C4 Volume",PCM1690_ATT_CONTROL(4),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("C5L Volume",PCM1690_ATT_CONTROL(5),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("C6R Volume",PCM1690_ATT_CONTROL(6),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("NC7 Volume",PCM1690_ATT_CONTROL(7),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_TLV("NC8 Volume",PCM1690_ATT_CONTROL(8),0,0x7f, 0, pcm1690_dac_tlv),
	SOC_SINGLE_BOOL_EXT("De-emphasis Switch", 0,pcm1690_get_deemph, pcm1690_put_deemph),//de-emphasis control	
};
/*
static const struct snd_soc_dapm_widget pcm1690_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("FLOUT"),
	SND_SOC_DAPM_OUTPUT("FROUT"),
	SND_SOC_DAPM_OUTPUT("RLOUT"),
	SND_SOC_DAPM_OUTPUT("RROUT"),
	SND_SOC_DAPM_OUTPUT("VLINE1"),
	SND_SOC_DAPM_OUTPUT("VLINE2"),
	SND_SOC_DAPM_OUTPUT("VOUT7"),
	SND_SOC_DAPM_OUTPUT("VOUT8"),
};

static const struct snd_soc_dapm_route pcm1690_dapm_routes[] = {
	{ "FLOUT", NULL, "Front Left" }, 
	{ "FROUT", NULL, "Front Right" },
	{ "RLOUT", NULL, "Rear Left" },
	{ "RROUT", NULL, "Rear Right" },
	{ "VLINE1", NULL, "LLOUT" },
	{ "VLINE2", NULL, "RLOUT" },
	{ "VOUT7", NULL, "dac7OUT" }, //NC
	{ "VOUT8", NULL, "dac8OUT" }, //NC
};


static const struct snd_soc_dapm_route pcm1690_dapm_routes[] = {
	{ "FLOUT", NULL, "Playback" }, 
	{ "FROUT", NULL,"Playback" },
	{ "RLOUT", NULL, "Playback" },
	{ "RROUT", NULL, "Playback"},
	{ "VLINE1", NULL, "Playback" },
	{ "VLINE2", NULL, "Playback" },
	{ "VOUT7", NULL, "Playback" }, //NC
	{ "VOUT8", NULL, "Playback" }, //NC
};
*/

static const struct snd_soc_dapm_widget pcm1690_dapm_widgets[] = {
	SND_SOC_DAPM_OUTPUT("VOUT1"),
	SND_SOC_DAPM_OUTPUT("VOUT2"),
	SND_SOC_DAPM_OUTPUT("VOUT3"),
	SND_SOC_DAPM_OUTPUT("VOUT4"),
	SND_SOC_DAPM_OUTPUT("VOUT5"),
	SND_SOC_DAPM_OUTPUT("VOUT6"),
	SND_SOC_DAPM_OUTPUT("VOUT7"),
	SND_SOC_DAPM_OUTPUT("VOUT8"),
};

static const struct snd_soc_dapm_route pcm1690_dapm_routes[] = {
	{ "VOUT1", NULL, "Playback" }, 
	{ "VOUT2", NULL, "Playback" },
	{ "VOUT3", NULL, "Playback" },
	{ "VOUT4", NULL, "Playback" },
	{ "VOUT5", NULL, "Playback" },
	{ "VOUT6", NULL, "Playback" },
	{ "VOUT7", NULL, "Playback" }, //NC
	{ "VOUT8", NULL, "Playback" }, //NC
};


 /* ---------------------------------------------------------------------
 * Digital Audio Interface Definition
 */


static bool pcm1690_accessible_reg(struct device *dev, unsigned int reg)
{
	return ((reg <= 0x4F &&  reg >= 0x40) && !(reg == 0x47)); //ok if 64-79, 0x47=71 is not applicaple
}

static bool pcm1690_writeable_reg(struct device *dev, unsigned register reg)
{
	return( pcm1690_accessible_reg(dev, reg) && (reg != PCM1690_ZERO_DETECT_STATUS));
}

	
/* ---------------------------------------------------------------------
 * Digital Audio Interface Operations ok
 */

 static int pcm1690_set_fmt(struct snd_soc_dai *codec_dai,
			      unsigned int format)
{
	struct snd_soc_codec *codec = codec_dai->codec;
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);

	// The pcm1690 can only be slave to all clocks 
	if ((format & SND_SOC_DAIFMT_MASTER_MASK) != SND_SOC_DAIFMT_CBS_CFS) {
		dev_err(codec->dev, "Invalid clocking mode\n");
		return -EINVAL;
	}

	priv->format = format;

	return 0;
}
 
static int pcm1690_hw_params(struct snd_pcm_substream *substream,
			     struct snd_pcm_hw_params *params,
			     struct snd_soc_dai *dai)
{
	
	struct snd_soc_codec *codec = dai->codec;
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);
	int val = 0, ret;

	priv->rate = params_rate(params); //input stream khz
	//priv->format = params_width(params); //input stream in bits
	
	switch (priv->format & SND_SOC_DAIFMT_FORMAT_MASK) {
	case SND_SOC_DAIFMT_I2S:
		switch (params_width(params)) {
			case 24:
				val=0x06; //basic TDM <=48khz
				if((priv->rate) > 48000){
					val=0x08; //fast mode > 48khz	
				}
				break;
			case 16: //only for testing purposes remove after usage
				val=0x06; //basic TDM <=48khz
				if((priv->rate) > 48000){
					val=0x08; //fast mode > 48khz	
				}
				break;	
			default:
				val=0x00; //0000 I2S 16/20/24/32 //set as default, stereo only
		}
		break;
	case SND_SOC_DAIFMT_RIGHT_J: //stereo only
		switch (params_width(params)) {
		case 24:
			val = 2; //0010 right justified 24bit
			break;
		case 16:
			val = 3; //0011 right justified 16bit
			break;
		default:
			dev_err(codec->dev, "Invalid sound output format on right justified\n");
			return -EINVAL;
		}
		break;
	/*case SND_SOC_DAIFMT_LEFT_J: //left justified
		switch (params_width(params)) {
			case 24:
				val=0x07; //basic left justified TDM <=48khz
				if(params_rate(params) > 48000){
					val=0x09; //left justified fast mode > 48khz	
				}
				break;
			default:
				val=0x01; //0001 left justified 16/20/24/32 stereo
		}
		break;*/
	//on TDM mode this and dsp_b are the only modes that pcm1690 supports		
	case SND_SOC_DAIFMT_DSP_A: // dsp_A, L data MSB after FRM LRC drops as in pcm1690 datasheet for TDM
		switch (params_width(params)) {
			case 24:
				//val=0x04; //dsp i2s
				val=0x06; //basic TDM <=48khz
				if(params_rate(params) > 48000){
					val=0x08; //fast mode > 48khz	
				}
				break;
				
			default: 	//testing speaker-test doesn't support 24bit output, 
						//so force 16bit testing in 24bit mode, will sound horrible!!!
				val=0x06; //basic TDM <=48khz
				if(params_rate(params) > 48000){
					val=0x08; //fast mode > 48khz
					//dev_info(codec->dev, "fast mode");
				}
				dev_info(codec->dev, "Used default DPS_A");
				
		}
		break;	
	// dsp_B, chan1 data MSB during FRM LRC drops as in pcm1690 datasheet for left justified TDM		
	case SND_SOC_DAIFMT_DSP_B: 
		if(params_width(params)==24) {
				val=0x07; //basic TDM <=48khz
				if(params_rate(params) > 48000){
					val=0x09; //fast mode > 48khz	
				}
				break;
		}
		break;		

	default:
		dev_err(codec->dev, "Invalid sound output format\n");
		return -EINVAL;
	}
	
	ret = regmap_update_bits(priv->regmap, PCM1690_FMT_CONTROL, 0x0f, val); //write format into pcm1690 register
	if (ret < 0){
		return ret;
		dev_err(codec->dev, "Failed to set format in pcm1690\n");
	}
	return pcm1690_set_deemph(codec);
}

static int pcm1690_digital_mute(struct snd_soc_dai *dai, int mute)
{
	struct snd_soc_codec *codec = dai->codec;
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);
	int val;

	if (mute)
		val = PCM1690_SOFT_MUTE_ALL;
	else
		val = 0;

	return regmap_write(priv->regmap, PCM1690_SOFT_MUTE, val);
}
/*
static int pcm1690_set_tdm_slots(struct snd_soc_dai *dai, unsigned int tx_mask,
			       unsigned int rx_mask, int slots, int width)
{
	
	
	
	return 0;
}	
*/
//dai ops
static const struct snd_soc_dai_ops pcm1690_dai_ops = {
	.hw_params	= pcm1690_hw_params,
	.digital_mute	= pcm1690_digital_mute,
	//.set_tdm_slot = pcm1690_set_tdm_slots,
	.set_fmt	= pcm1690_set_fmt,
};

static struct snd_soc_dai_driver pcm1690_dai = {
	.name = "pcm1690-hifi", //has to be the same as in snd_soc_dai_link .name
	.playback = {
		.stream_name = "Playback", //has to be the same as in snd_soc_dai_link .stream_name "Playback" org
		.channels_min = 2, //2
		.channels_max = 8, //8 outputs in total 
		.rates = PCM1690_PCM_RATES,//SNDRV_PCM_RATE_48000,//PCM1690_PCM_RATES,
		.formats = PCM1690_FORMATS,//SNDRV_PCM_FMTBIT_S24_LE, //in .h file only 24bits supported in TDM mode
	},
	.ops = &pcm1690_dai_ops,
};

static struct snd_soc_codec_driver pcm1690_codec_driver = { //soc_codec_dev_pcm1690
	.idle_bias_off = false,
	//.component_driver = {
	.controls			= pcm1690_controls,
	.num_controls		= ARRAY_SIZE(pcm1690_controls),
	.dapm_widgets		= pcm1690_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(pcm1690_dapm_widgets),
	.dapm_routes		= pcm1690_dapm_routes,
	.num_dapm_routes	= ARRAY_SIZE(pcm1690_dapm_routes),
	//},
};
//codec probe
int pcm1690_codec_probe(struct device *dev, struct regmap *regmap)
{
	struct pcm1690_private *priv;
	int ret=0;// ret2;
	int times=0;
		
	priv = devm_kzalloc(dev, sizeof(*priv), GFP_KERNEL);
	if (priv == NULL){
		dev_err(dev, "Missing device\n");
		return -ENOMEM;
	}
	
	dev_set_drvdata(dev, priv);
	
	priv->dev = dev;
	priv->regmap = regmap;
	
	// Reset the device, verifying I/O in the process for I2C 
	ret = regmap_write_bits(regmap, PCM1690_SYS_RESET_register, 0x40 ,PCM1690_SYS_RESET); //will return 0 if success
	if (ret != 0) {
		//msleep(1);
		while(ret !=0 || times <= 100){
			times++;
			ret = regmap_write_bits(regmap, PCM1690_SYS_RESET_register, 0xC0,0); //try again, may make a pop noise
			msleep(1);	
		}
		if (ret != 0) {
			dev_err(dev, "Failed to reset device: %d\n", ret);
			return ret;
		}
			
	}
	// Internal reset is de-asserted after 3846 SCKI cycles 
	//msleep(DIV_ROUND_UP(3846 * 1000, 11289600)); //wait atleast this long
	msleep(1);
	//pm_runtime_set_active(dev);
	//pm_runtime_enable(dev);
	//pm_runtime_idle(dev);
	
	ret = snd_soc_register_codec(dev, &pcm1690_codec_driver, &pcm1690_dai, 1);
	if (ret != 0) {
		dev_err(dev, "Failed to register CODEC: %d\n", ret);
		return ret;
	}
	dev_info(dev,"successful pcm1690 codec registeration! %d\n", 3);
	return 0;
	
}
EXPORT_SYMBOL_GPL(pcm1690_codec_probe);


void pcm1690_codec_remove(struct device *dev)
{
	//struct snd_soc_codec *codec = codec_dai->codec;
	//struct pcm1690_private *priv =snd_soc_codec_get_drvdata(codec);
	//dev_get_drvdata(dev);
	dev_info(dev,"codec unregistered %d\n", 2);
	snd_soc_unregister_codec(dev);
	//pm_runtime_disable(dev);
	
}
EXPORT_SYMBOL_GPL(pcm1690_codec_remove);

const struct regmap_config pcm1690_regmap_config = {
	.reg_bits		= 8,
	.val_bits		= 8,
	.max_register		= 0x4f, //last register you can write/read to registers start at 0x40
	.writeable_reg		= pcm1690_writeable_reg, //acceccible reqs since no pll can put all writeable reqs
	.readable_reg		= pcm1690_accessible_reg,
	.reg_defaults		= pcm1690_reg_defaults,
	.num_reg_defaults	= ARRAY_SIZE(pcm1690_reg_defaults),
	.cache_type = REGCACHE_FLAT, //???
};
EXPORT_SYMBOL_GPL(pcm1690_regmap_config);

/*
#ifdef CONFIG_PM
static int pcm1690_suspend(struct device *dev,struct snd_soc_codec *codec)
{
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(dai->codec);
	//dev_get_drvdata(dev);
	int ret1;int ret2;int ret3;
	
	ret1=regmap_update_bits(priv->regmap, PCM1690_SOFT_MUTE, PCM1690_SOFT_MUTE_ALL, PCM1690_SOFT_MUTE_ALL); //softmutes on
	ret2=regmap_update_bits(priv->regmap, PCM1690_POWER_SAVE,PCM1690_POWER_SAVE_MASK, 0x00); //powersave
	ret3=regmap_update_bits(priv->regmap, 0x42, 0xf0, 0xf0);//dac's disabled
	
	if (ret1 != 0 || ret2 != 0 || ret3 != 0 ) {
		dev_err(dev, "Failed to request power save: %d\n", 1);
		return 1;
	}
	return 0;
}

static int pcm1690_resume(struct device *dev,struct snd_soc_codec *codec)
{
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(dai->codec);

	//dev_get_drvdata(dev);
	int ret1;int ret2;int ret3;
	
	ret2=regmap_update_bits(priv->regmap, PCM1690_POWER_SAVE,PCM1690_POWER_SAVE_MASK , 0x80); //powersave
	ret3=regmap_update_bits(priv->regmap, 0x42, 0x70, 0x00);//dac's on
	ret1=regmap_update_bits(priv->regmap, PCM1690_SOFT_MUTE, 0x3f, 0x00); //softmutes off except on dac7 and 8
	if (ret1 != 0 || ret2 != 0 || ret3 != 0 ) {
		dev_err(dev, "Failed to request power save off: %d\n", 1);
		return 1; //-EINVAL
	}
	return 0;
}
#endif

static const struct dev_pm_ops pcm1690_pm_ops = {
	SET_RUNTIME_PM_OPS(pcm1690_suspend, pcm1690_resume, NULL)
};
EXPORT_SYMBOL_GPL(pcm1690_pm_ops);
*/

MODULE_AUTHOR("add_one_day <kollero@hotmail.com> ");
MODULE_DESCRIPTION("ASoC pcm1690 codec driver ");
MODULE_LICENSE("GPL v2");

