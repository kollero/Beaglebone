/*
 * Driver for the PCM1690 CODECs	
* Copyright 2017 
* Author: change_one_day
 */

#include <linux/pm.h>
#include <linux/regmap.h>
/*
#define PCM1690_FORMATS ( SNDRV_PCM_FMTBIT_S16_LE  	\
			    |	 SNDRV_PCM_FMTBIT_S24_LE  		\
				| SNDRV_PCM_FMTBIT_S32_LE ) 
				 //last one not supported in TDM mode
*/
//#define PCM1690_FORMATS ( SNDRV_PCM_FMTBIT_S24_LE  )

				 
				 //only 44.1 and 48khz multiple freqs is supported to the master clock with 256 fs single or dual rate
#define PCM1690_PCM_RATES (	SNDRV_PCM_RATE_44100  |SNDRV_PCM_RATE_48000 \
							| SNDRV_PCM_RATE_88200  | SNDRV_PCM_RATE_96000 )

/*				 
#define PCM1690_PCM_RATES   (SNDRV_PCM_RATE_8000 | SNDRV_PCM_RATE_16000 | \
			     SNDRV_PCM_RATE_32000 | SNDRV_PCM_RATE_44100  | \
			     SNDRV_PCM_RATE_48000 | SNDRV_PCM_RATE_88200  | \
			     SNDRV_PCM_RATE_96000 | SNDRV_PCM_RATE_192000)
*/
#define PCM1690_SOFT_MUTE_ALL		0xff
#define PCM1690_DEEMPH_RATE_MASK	0x30 //bits 5 and 4
#define PCM1690_DEEMPH_MASK		0x30 //bits 5 and 4 //remove
#define PCM1690_POWER_SAVE_MASK	0x80

#define PCM1690_ATT_CONTROL(X)	(X >= 72 ? X : X + 71) // Attenuation levels in registers: 72-79 
#define PCM1690_SOFT_MUTE	0x44	// Soft mute control register bits  
#define PCM1690_FMT_CONTROL	0x41	// Audio interface data format 
#define PCM1690_POWER_SAVE	0x41
#define PCM1690_DEEMPH_CONTROL	0x46	// De-emphasis control 
#define PCM1690_ZERO_DETECT_STATUS	0x45	// Zero detect status reg read only

#define PCM1690_SYS_RESET_register 0x40
#define PCM1690_SYS_RESET 0x00	// system reset may make a popping sound

#define CLK48EN 0	//44khz 
#define CLK44EN 1 	//48khz
#define SINGLE_SPEED 0 //single rate
#define DOUBLE_SPEED 1 //double clock speed

#define CLK_44EN_SINGLE 11289600UL
#define CLK_48EN_SINGLE 12288000UL
#define CLK_44EN_DOUBLE 22579200UL
#define CLK_48EN_DOUBLE 24576000UL


extern const struct dev_pm_ops pcm1690_pm_ops;
extern const struct regmap_config pcm1690_regmap_config;

int pcm1690_codec_probe(struct device *dev, struct regmap *regmap);
void pcm1690_codec_remove(struct device *dev);



