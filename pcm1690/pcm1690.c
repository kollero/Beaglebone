/*
 *	pcm1690 driver hardware module
 * 	Copyright (c) 2017
 * 	Author: change_one_day
 *
 */

#include <linux/gpio.h>

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/timer.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/of_platform.h>
#include <linux/clk.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>
#include <asm/dma.h>
#include <asm/mach-types.h> 

#include "codec/pcm1690.h"

struct snd_soc_card_drvdata_pcm1690 {
	struct clk *mclk;
	unsigned int sysclk;
	unsigned int codec_clock;
	unsigned int rate; 
};
/*
static const struct snd_soc_dapm_widget pcm1690_dapm_widgets2[] = {
	SND_SOC_DAPM_OUTPUT("VOUT1"),
	SND_SOC_DAPM_OUTPUT("VOUT2"),
	SND_SOC_DAPM_OUTPUT("VOUT3"),
	SND_SOC_DAPM_OUTPUT("VOUT4"),
	SND_SOC_DAPM_OUTPUT("VOUT5"),
	SND_SOC_DAPM_OUTPUT("VOUT6"),
	SND_SOC_DAPM_OUTPUT("VOUT7"),
	SND_SOC_DAPM_OUTPUT("VOUT8"),
};

static const struct snd_soc_dapm_route audio_map[] = {
	{ "VOUT1", NULL, "Playback" }, 
	{ "VOUT2", NULL, "Playback" },
	{ "VOUT3", NULL, "Playback" },
	{ "VOUT4", NULL, "Playback" },
	{ "VOUT5", NULL, "Playback" },
	{ "VOUT6", NULL, "Playback" },
	{ "VOUT7", NULL, "Playback" }, //NC
	{ "VOUT8", NULL, "Playback" }, //NC
};
*/

/*
//	Define Dynamic Audio Power Management (DAPM) widgets
// will override codec widgets, set in sound card init
static const struct snd_soc_dapm_widget pcm1690_dapm_widgets[] = {
	SND_SOC_DAPM_SPK("Speaker Out", NULL),
	SND_SOC_DAPM_LINE("Line Out", NULL),
};

static const struct snd_soc_dapm_route audio_map[] = {
	{"Speaker Out", NULL, "VOUT1"},
	{"Speaker Out", NULL, "VOUT2"},
	{"Speaker Out", NULL, "VOUT3"},
	{"Speaker Out", NULL, "VOUT4"},
	{"Line Out", NULL, "VOUT5"},
	{"Line Out", NULL, "VOUT6"},
	
};
*/

/* ---------------------------------------------------------------------
 * Setting PLL1705 clock output 256*fs
 */
/*
static void snd_pcm1690_select_clk(struct snd_soc_codec *codec, int clk_id)
{
switch (clk_id) {
	case CLK48EN:  //0
		gpio_set_value(50, 0); //gpio 1_18 = 32*1+18=50
		break;
	case CLK44EN: //1
		gpio_set_value(50, 1); 
		break;
	}
}

static void snd_pcm1690_select_speed(struct snd_soc_codec *codec, int speed)
{
switch (speed) {
	case SINGLE_SPEED:  //0
		gpio_set_value(51, 0); //gpio 1_19 = 32*1+19=51
		break;
	case DOUBLE_SPEED: //1
		gpio_set_value(51, 1);		
		break;

	}
}

static void snd_pcm1690_set_sclk(struct snd_soc_codec *codec, int sample_rate)
{	
	struct pcm1690_private *priv = snd_soc_codec_get_drvdata(codec);
	
	switch (sample_rate) {
	
	case 44100:
		snd_pcm1690_select_clk(codec,CLK44EN);
		snd_pcm1690_select_speed(codec,SINGLE_SPEED);
		priv->sclk= CLK_44EN_SINGLE;
		break;
	case 48000:
		snd_pcm1690_select_clk(codec,CLK48EN);
		snd_pcm1690_select_speed(codec,SINGLE_SPEED);
		priv->sclk= CLK_48EN_SINGLE;
		break;
	case 88200:
		snd_pcm1690_select_clk(codec,CLK44EN);
		snd_pcm1690_select_speed(codec,DOUBLE_SPEED);
		priv->sclk= CLK_44EN_DOUBLE;
		break;
	case 96000:
		snd_pcm1690_select_clk(codec,CLK48EN);
		snd_pcm1690_select_speed(codec,DOUBLE_SPEED);
		priv->sclk= CLK_48EN_DOUBLE;
		break;
	default:
		snd_pcm1690_select_clk(codec,CLK48EN);
		snd_pcm1690_select_speed(codec,SINGLE_SPEED);
		priv->sclk= CLK_48EN_SINGLE;
		break;
	}
}
*/
/*
	Sound card init
*/
static int snd_pcm1690_audiocard_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_card *card = rtd->card;
	struct device_node *np = card->dev->of_node;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	int ret = 0;
	unsigned int tdm_mask = 0x00;
	u32 tdm_slots;
	
	//snd_soc_dapm_new_controls(&card->dapm, pcm1690_dapm_widgets2,ARRAY_SIZE(pcm1690_dapm_widgets2));

	//Get audio routing from device tree or use built-in routing
	/*
	if (np) {
		dev_dbg(card->dev, "Using configuration from device tree overlay.\n");
		ret = snd_soc_of_parse_audio_routing(card, "audio-routing");
		if (ret){
			return ret;
		}
		ret = of_property_read_u32(np, "audiocard-tdm-slots", &tdm_slots);
		if (tdm_slots > 8 || tdm_slots < 2 || ret){
			dev_dbg(card->dev, "Couldn't get device tree property for tdm slots. Using default (=2).\n");
			tdm_slots = 2;
			tdm_mask = 0x03; // lsb for slot 0, ...
		} else {
			tdm_mask = 0xFF;
			tdm_mask = tdm_mask >> (8 - tdm_slots);
	}
	} else {
		dev_dbg(card->dev, "Use builtin audio routing.\n");
		// Set up specific audio path audio_map 
		snd_soc_dapm_add_routes(&card->dapm, audio_map, ARRAY_SIZE(audio_map));
	}
	*/
		
	ret = of_property_read_u32(np, "audiocard-tdm-slots", &tdm_slots);
	if (tdm_slots > 8 || tdm_slots < 2 || ret){
			dev_dbg(card->dev, "Couldn't get device tree property for tdm slots. Using default (=2).\n");
			tdm_slots = 2;
			tdm_mask = 0x03; // lsb for slot 0, ...
		} else {
			tdm_mask = 0xFF;
			tdm_mask = tdm_mask >> (8 - tdm_slots);
	}
	
	//Configure TDM mode of CPU and audio codec interface
		//(pcm1690 codec driver ignores TX mask width)
	//arguments for tdm slot: dai_config, tx_mask, rx_mask, slots, bit width of slot
	//tx and rx mask represent the active Xx slots, so for 8 output channels it has to be 8 slots
	//codec dai has been set in the codec
	
	//ret = snd_soc_of_parse_audio_routing(card, "audio-routing");
	
	//ret = snd_soc_dai_set_tdm_slot(cpu_dai, 0xFF, 0, 8, 32);
	ret = snd_soc_dai_set_tdm_slot(cpu_dai, tdm_mask, tdm_mask, tdm_slots, 32);
	if (ret < 0){
		dev_err(codec_dai->dev, "Unable to set McASP TDM slots.\n");
		return ret;
	}
	
	return 0;
}

static int snd_pcm1690_audiocard_hw_params(
	struct snd_pcm_substream *substream, struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_codec *codec = rtd->codec;
	struct snd_soc_card *soc_card = rtd->card;
	
	unsigned int rate = params_rate(params);
	unsigned int cpu_clock = ((struct snd_soc_card_drvdata_pcm1690 *)
		snd_soc_card_get_drvdata(soc_card))->sysclk;
	unsigned int codec_clock = ((struct snd_soc_card_drvdata_pcm1690 *)
		snd_soc_card_get_drvdata(soc_card))->codec_clock;	
	
	int ret = 0;
	
	snd_soc_dai_set_clkdiv(cpu_dai, 1, cpu_clock/(rate*256));	//8channels and 32bits=256
	//snd_soc_dai_set_clkdiv(codec_dai, 1, cpu_clock/(rate*256));	//8channels and 32bits=256
	/*
	//Set master clock of CPU and audio codec interface
	ret = snd_soc_dai_set_sysclk(codec_dai, 0, codec_clock, SND_SOC_CLOCK_IN); //SND_SOC_CLOCK_IN
	if (ret < 0){
		dev_err(codec->dev, "Unable to set PCM1690 system clock: %d.\n", ret);
		return ret;
	}*/
	dev_dbg(cpu_dai->dev, "Set codec DAI clock rate to %d.\n", codec_clock);
	ret = snd_soc_dai_set_sysclk(cpu_dai, 0, cpu_clock, SND_SOC_CLOCK_OUT); //SND_SOC_CLOCK_OUT
	if (ret < 0){
		dev_err(cpu_dai->dev, "Unable to set cpu dai sysclk: %d.\n", ret);
		return ret;
	}
	dev_dbg(cpu_dai->dev, "Set CPU DAI clock rate to %d.\n", cpu_clock);
	
	
	
	//dev_info(cpu_dai->dev,"audiocard HW params set %d\n", 1);
	return 0;
}

static int snd_pcm1690_audiocard_startup(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *soc_card = rtd->card;
	struct snd_soc_card_drvdata_pcm1690 *drvdata = snd_soc_card_get_drvdata(soc_card);
	gpio_set_value(61, 1); //gpio1 29 =1*32+29=61 set to 1 //analog mute off
	if (drvdata->mclk){
		return clk_prepare_enable(drvdata->mclk);
	}
	//dev_info(&pdev->dev,"audiocard startup successful %d\n", 1);
	return 0;
}

static void snd_pcm1690_audiocard_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_card *soc_card = rtd->card;
	struct snd_soc_card_drvdata_pcm1690 *drvdata = snd_soc_card_get_drvdata(soc_card);
	gpio_set_value(61, 0); //gpio1 29 =1*32+29=61 set to 0 //analog mute on
	if (drvdata->mclk){
		clk_disable_unprepare(drvdata->mclk);
	}
	//dev_info(&pdev->dev,"audiocard shutdown successful %d\n", 1);
	
}

// SoC audio ops or platform DMA driver
static struct snd_soc_ops snd_pcm1690_audiocard_ops = {
	.hw_params = snd_pcm1690_audiocard_hw_params,
	.startup = snd_pcm1690_audiocard_startup,
	.shutdown = snd_pcm1690_audiocard_shutdown,
};


//in dsp_A, L data MSB after FRM LRC as in pcm1690 datasheet for TDM
//set format to DSP_A, inverse frame, normal bitclock, and codec slave (pcm1690 is always set to slave mode)
#define AUDIOCARD_1690_DAIFMT ( SND_SOC_DAI_FORMAT_DSP_A | SND_SOC_DAIFMT_NB_IF | SND_SOC_DAIFMT_CBS_CFS )

//link between cpu dai and codec dai
static struct snd_soc_dai_link BBB_snd_pcm1690_audiocard_dai = {
	.name		= "pcm1690audiocard", //whatever name
	.stream_name = "TDM", //?!?
	//.stream_name	= "Playback", // same stream name as in the snd_soc_dai_driver!!
	//.cpu_dai_name	= "mcasp-controller", //not to be used, instead cpu_of_node in probe
	.codec_dai_name	= "pcm1690-hifi", //from snd_soc_dai_driver .name 
	//.platform_name	= "mcasp-controller", //not to be used
	//.codec_name	= "pcm1690.1-004c", //i2c-1@4c, device tree will add this?!?
	.dai_fmt	= AUDIOCARD_1690_DAIFMT,
	.ops		= &snd_pcm1690_audiocard_ops,
	.init		= snd_pcm1690_audiocard_init,
};

//making the driver device tree compatible
static const struct of_device_id snd_pcm1690_audiocard_dt_ids[] = {
	{ 
		.compatible = "ti,soundcard-pcm1690", 
		.data = &BBB_snd_pcm1690_audiocard_dai,
	},
	{/* sentinel */}
};
MODULE_DEVICE_TABLE(of, snd_pcm1690_audiocard_dt_ids);

// audio machine driver, codec and driver glue
static struct snd_soc_card snd_BBB_pcm1690_audiocard = { 
	.name         = "sndpcm1690audiocard", //name of the card
	.owner		  = THIS_MODULE,
	.num_links    = 1,//ARRAY_SIZE(BBB_snd_pcm1690_audiocard_dai),
	//.dai_link     = BBB_snd_pcm1690_audiocard_dai,
	//.dapm_widgets = pcm1690_dapm_widgets,
	//.num_dapm_widgets = ARRAY_SIZE(pcm1690_dapm_widgets),
	
};

//sound card probe
static int snd_pcm1690_audiocard_probe(struct platform_device *pdev)
{		
	const struct of_device_id *match =
		of_match_device(of_match_ptr(snd_pcm1690_audiocard_dt_ids), &pdev->dev);
	struct snd_soc_dai_link *dai = (struct snd_soc_dai_link *) match->data;
	struct snd_soc_card_drvdata_pcm1690 *drvdata = NULL;
	struct clk *mclk;
	int ret =0;

	snd_BBB_pcm1690_audiocard.dai_link = dai;
	
	
	dai->codec_of_node = of_parse_phandle(pdev->dev.of_node, "audio-codec", 0);
	if (!dai->codec_of_node){
		dev_err(&pdev->dev,"failed to register codec: %d\n", 1);
		return -EINVAL;
	}
	
	//dai->cpu_dai_name = NULL;
	dai->cpu_of_node=of_parse_phandle(pdev->dev.of_node,"mcasp-controller", 0);
	if (!dai->cpu_of_node){
		dev_err(&pdev->dev,"failed to get cpu name: %d\n", 1);
		return -EINVAL;
	}
	//dai->platform_name = NULL;
	dai->platform_of_node =of_parse_phandle(pdev->dev.of_node,"mcasp-controller", 0);
	
	snd_BBB_pcm1690_audiocard.dev = &pdev->dev;
	ret = snd_soc_of_parse_card_name(&snd_BBB_pcm1690_audiocard, "model");
	if (ret){
		return ret;
	}
		
	mclk = devm_clk_get(&pdev->dev, "mclk");
	if (PTR_ERR(mclk) == -EPROBE_DEFER) {
		return -EPROBE_DEFER;
	} else if (IS_ERR(mclk)) {
		dev_dbg(&pdev->dev, "mclk not found.\n");
		mclk = NULL;
	}
	
	drvdata = devm_kzalloc(&pdev->dev, sizeof(*drvdata), GFP_KERNEL);
	if (!drvdata){
		dev_err(&pdev->dev,"failed to get memory: %d\n", 1);
		return -ENOMEM;	
	}
	drvdata->mclk = mclk;
	
	ret = of_property_read_u32(pdev->dev.of_node, "codec-clock-rate", &drvdata->codec_clock);
	if (ret < 0){
		dev_err(&pdev->dev, "No codec clock rate defined.\n");
		return -EINVAL;
	}
	
		ret = of_property_read_u32(pdev->dev.of_node, "cpu-clock-rate", &drvdata->sysclk);
		if (ret < 0) {
			if (!drvdata->mclk) {
				dev_err(&pdev->dev, "No clock or clock rate defined.\n");
				return -EINVAL;
			}
			drvdata->sysclk = clk_get_rate(drvdata->mclk);
		} else if (drvdata->mclk) {
			unsigned int requestd_rate = drvdata->sysclk;
			clk_set_rate(drvdata->mclk, drvdata->sysclk);
			drvdata->sysclk = clk_get_rate(drvdata->mclk);
			if (drvdata->sysclk != requestd_rate)
				dev_warn(&pdev->dev, "Could not get requested rate %u using %u.\n",
					requestd_rate, drvdata->sysclk);
		}
	

	snd_soc_card_set_drvdata(&snd_BBB_pcm1690_audiocard, drvdata);
	ret = devm_snd_soc_register_card(&pdev->dev,&snd_BBB_pcm1690_audiocard);
	if(ret){
		dev_err(&pdev->dev,"devm_snd_soc_register_card() failed: %d\n", ret);
	}
	dev_info(&pdev->dev,"registering snd-BBB-pcm1690 successful %d\n", 1);
	return ret;
}

// Sound card disconnect, shouldn't be needed since devm in card register takes care of it
//static int snd_pcm1690_audiocard_remove(struct platform_device *pdev)
//{
//	dev_info(&pdev->dev,"disconnected?!? %d\n", 4);
//	return snd_soc_unregister_card(&snd_BBB_pcm1690_audiocard);
//}


//snd_soc_platform_driver
static struct platform_driver snd_pcm1690_audiocard_driver = {
	.driver = {
		.name   = "snd-BBB-pcm1690",  
		.owner  = THIS_MODULE,
		.of_match_table = of_match_ptr(snd_pcm1690_audiocard_dt_ids),//snd_pcm1690_audiocard_dt_ids, //
		//.pm = &snd_soc_pm_ops,
	},
	.probe          = snd_pcm1690_audiocard_probe,
	//.remove         = snd_pcm1690_audiocard_remove,
};
module_platform_driver(snd_pcm1690_audiocard_driver);

MODULE_AUTHOR("add_one_day <kollero@hotmail.com> ");
MODULE_DESCRIPTION("ASoC Driver for pcm1690 as soundcard");
MODULE_LICENSE("GPL v2");
