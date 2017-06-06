#ifndef    _SCPI_INSTRUCTION_TABLE_H_
#define    _SCPI_INSTRUCTION_TABLE_H_

#include    "scpi_parser.h"
extern const SingleScaleParserStruct    t_InstructionStruct_RST;
extern const SingleScaleParserStruct    t_InstructionStruct_IDN;
extern const HeadScaleParserStruct      t_InstructionStruct_Communication;
extern const HeadScaleParserStruct      t_InstructionStruct_System;
//文件指令集第一级（根级）
extern const HeadScaleParserStruct		__t_InstructionStruct_File;

extern const MidleScaleParserStruct     t_InstructionStruct_ResuFetch;


extern const MidleScaleParserStruct     t_InstructionStruct_SrcTest;
extern const MidleScaleParserStruct     t_InstructionStruct_SrcLoad;
extern const MidleScaleParserStruct     t_InstructionStruct_SrcList;
extern const TailScaleParserStruct      t_InstructionStruct_SrcListSindex;
extern const TailScaleParserStruct      t_InstructionStruct_SrcListSmessage;

extern const TailScaleParserStruct      t_InstructionStruct_SystemOALArm;

extern const TailScaleParserStruct      t_InstructionStruct_SystemKeyVolume;

//---------------------------------Dcw----------------------------------------------------
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwVolt; 	 
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwCurHigh; 
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwCurLow;  
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwTtime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwRtime;
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwItime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwArc;
extern const TailScaleParserStruct    t_InstructionStruct_StepDcwFtime;
extern const TailScaleParserStruct		t_InstructionStruct_StepDcwRange;

//---------------------------------Acw----------------------------------------------------
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwVolt;	   	 
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwCurHigh;
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwCurLow;  
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwTtime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwRtime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwItime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwArc;
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwFREQuency;
extern const TailScaleParserStruct		t_InstructionStruct_StepAcwRange;
extern const TailScaleParserStruct    t_InstructionStruct_StepAcwFtime;

//---------------------------------GR----------------------------------------------------
extern const TailScaleParserStruct		t_InstructionStruct_StepGrCurr;
extern const TailScaleParserStruct		t_InstructionStruct_StepGrHigh;
extern const TailScaleParserStruct		t_InstructionStruct_StepGrLow;  	   	 
extern const TailScaleParserStruct		t_InstructionStruct_StepGrTtime;
extern const TailScaleParserStruct		t_InstructionStruct_StepGrItime;
extern const TailScaleParserStruct		t_InstructionStruct_StepGrFREQuency;

//---------------------------------IR----------------------------------------------------
extern const TailScaleParserStruct      t_InstructionStruct_StepIrVolt;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrVolt_2676N;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrArange;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrHigh;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrLow;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrTtime;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrDtime;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrItime;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrFtime;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrRtime;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrHrange;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrOmode;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrHighMax;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrHighMin;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrLowMax;
extern const TailScaleParserStruct      t_InstructionStruct_StepIrLowMin;

//------------------------------LC------------------------------------------------------
extern const TailScaleParserStruct		t_InstructionStruct_StepLcVolt;	   	 
extern const TailScaleParserStruct		t_InstructionStruct_StepLcCurHigh;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcCurLow;  
extern const TailScaleParserStruct		t_InstructionStruct_StepLcTtime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepLcRtime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepLcItime; 
extern const TailScaleParserStruct		t_InstructionStruct_StepLcFREQuency;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcRange;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcPHASe;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcMDNEt;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcMDVOl;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcASSVOl;
extern const TailScaleParserStruct		t_InstructionStruct_StepLcMDCOm;

/******************************************************************************
 *                            文件接口函数声明
******************************************************************************/

extern uint32_t APP_RootParserContainerInfoGet(struct PARSER_CONTAINER **ptrootParserContainer, uint8_t *prootParserContainerCapacity);

#endif

