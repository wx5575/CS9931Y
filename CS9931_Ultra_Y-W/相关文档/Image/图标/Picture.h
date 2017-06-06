/******************************************************************************
 * 文件信息 :  
 *
 * 创 建 者 :  
 *
 * 创建日期 :  2007.09.29
 * 
 * 原始版本 : 
 *     
 * 修改版本 :  
 *    
 * 修改日期 : 
 *
 * 修改内容 :
 * 
 * 审 核 者 :
 *
 * 附    注 :
 *
 * 描    述 :   源代码
 *
 * 版    权 :   南京长盛仪器有限公司 , Copyright Reserved
 * 
******************************************************************************/
 
#ifndef    _PICTURE_H_
#define    _PICTURE_H_

#if defined(__cplusplus)

    extern "C" {     /* Make sure we have C-declarations in C++ programs */

#endif

/******************************************************************************
 *                             包含文件声明
******************************************************************************/ 

#ifndef    _INCLUDES_H_

    #include    "Includes.h"   
    
#endif

/******************************************************************************
 *                            文件接口数据声明
******************************************************************************/

extern GUI_CONST_STORAGE GUI_BITMAP bmApplicationIcon; 
extern GUI_CONST_STORAGE GUI_BITMAP bmApplicationBigIcon;
extern GUI_CONST_STORAGE GUI_BITMAP bmCloseButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMaxButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmMinButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmCaptionBarLinePict;
extern GUI_CONST_STORAGE GUI_BITMAP bmCaptionBarDimmedLinePict;
extern GUI_CONST_STORAGE GUI_BITMAP bmDimmedCloseButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmDimmedMaxButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmDimmedMinButton;
extern GUI_CONST_STORAGE GUI_BITMAP bmButtonLeftDownPict;
extern GUI_CONST_STORAGE GUI_BITMAP bmButtonLeftUpPict;
extern GUI_CONST_STORAGE GUI_BITMAP bmButtonRightDownPict;
extern GUI_CONST_STORAGE GUI_BITMAP bmButtonRightUpPict;
extern GUI_CONST_STORAGE GUI_BITMAP bmStartUp;
extern GUI_CONST_STORAGE GUI_BITMAP bmFile;
extern GUI_CONST_STORAGE GUI_BITMAP bmStepInfo;
extern GUI_CONST_STORAGE GUI_BITMAP bmModeInfo;
extern GUI_CONST_STORAGE GUI_BITMAP bmStartupSmallPict;
extern const GUI_FONT GUI_Fonts_PictureAllwin;

extern GUI_CONST_STORAGE GUI_BITMAP bmWorkMode_G;
extern GUI_CONST_STORAGE GUI_BITMAP bmDUTMode_Whole;
extern GUI_CONST_STORAGE GUI_BITMAP bmWorkMode_N;
extern GUI_CONST_STORAGE GUI_BITMAP bmDUTMode_Single;

extern GUI_CONST_STORAGE GUI_BITMAP bmCapacityFull;
extern GUI_CONST_STORAGE GUI_BITMAP bmCapacityNone;
extern GUI_CONST_STORAGE GUI_BITMAP bmCpacityLack;

#if defined(__cplusplus)

    }

#endif 

/******************************************************************************
 *                             END  OF  FILE                                                                          
******************************************************************************/
#endif
