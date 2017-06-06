#ifndef   __CS99XX_CAL_H__
#define   __CS99XX_CAL_H__

#include "CS9931_Config.h"

/* ACW */
#define		DA_ACW_100V				    (40)
#define		DA_ACW_2000V			    (1200)
#define		DA_ACW_5000V			    (2700)
#define		DA_ACW_CURCAL_VOL		  	(500)

/* DCW */
#define		DA_DCW_100V				    (50)
#define		DA_DCW_2000V			    (1200)
#define		DA_DCW_6000V			    (3600)
#define		DA_DCW_CURCAL_VOL			(500)

/* GR */
#define		DA_GR_3A				      (175)
#define		DA_GR_10A				     (DA_GR_3A * 33 / 10) // (600)
#define		DA_GR_30A				     (DA_GR_3A * 10) // (1800)


/* LC */
/* 主电源校准点 */
#ifdef LC_MAX_VOL_300V
	#define		DA_LC_30V					(200)
	#define		DA_LC_150V				    (900)
	#define		DA_LC_300V				    (1800)

	#define DA_LC_CAL_VOL_P1			DA_LC_30V
	#define DA_LC_CAL_VOL_P2			DA_LC_150V
	#define DA_LC_CAL_VOL_P3			DA_LC_300V
#endif

#ifdef LC_MAX_VOL_250V
	#define		DA_LC_30V					(200)
	#define		DA_LC_150V				    (DA_LC_30V*5)
	#define		DA_LC_300V				    (DA_LC_30V*10)
	
	#define DA_LC_CAL_VOL_P1			DA_LC_30V
	#define DA_LC_CAL_VOL_P2			DA_LC_150V
	#define DA_LC_CAL_VOL_P3			DA_LC_300V
#endif

/* 辅助电源校准点 */
#define		DA_LC_30VS				    (160)
#define		DA_LC_150VS				    (DA_LC_30VS * 5) // (800)
#define		DA_LC_300VS				    (DA_LC_30VS * 85 / 10) //(1600)
#define		DA_LC_CURCAL_VOL	  		(250000)



enum{
	CAL_VOL=0,
	CAL_VOLS,
	CAL_CUR,
	CAL_ARC,
};










#endif
