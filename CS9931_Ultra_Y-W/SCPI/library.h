#ifndef    _LIB_LIBRARY_H_
#define    _LIB_LIBRARY_H_

extern void   LIB_DelayUs(uint16_t delayUs);

extern void   LIB_SoftDelayUs(uint16_t delayUs);

extern void   LIB_SoftDelayMs(uint16_t delayUs);

extern void   LIB_DelayMs(uint16_t delayMs);

extern uint32_t  LIB_MonitorUs(uint32_t monitorUs);

extern uint32_t LIB_Get10nData(uint32_t n);

extern void LIB_DataSplit(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr);

extern uint32_t LIB_DataCalculate(const uint8_t *rscStr, uint32_t calculateLen);

extern int8_t LIB_Strpos(const uint8_t *rscStr, uint8_t c);

extern uint32_t LIB_StrInsert(uint8_t *rscStr, uint8_t insertPosIndex, uint8_t insertChar);

extern int8_t LIB_StrBkpos(const uint8_t *rscStr, uint32_t c);

extern int8_t LIB_Strnpos(const uint8_t *rscStr, uint32_t c, uint32_t len);

extern uint8_t LIB_DataBitLenGet(uint32_t rscData);

extern void LIB_ConvertNmubToChar(uint8_t *prscSaveAddr, uint32_t convertLen);

extern int8_t LIB_SearchNoneNumbIndex(uint8_t *rscStr, uint32_t strLen);

extern void LIB_HideZeroForString(uint8_t *rscStr, uint32_t strLen);

extern void LIB_ConvertNmubToCharWithSpace(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr);

extern void LIB_ConvertNmubToString(uint32_t rscData, uint32_t splitLen, uint8_t *pdestSaveAddr);

extern uint8_t LIB_GetChkSum(uint8_t *rscAddr, uint32_t len);

#ifdef			LIB__CHK_DATA_STR_VALID

extern uint32_t LIB_ChkDateStrValid(uint8_t *rscDateStr);

extern uint32_t LIB_ChkTimeStrValid(uint8_t *rscTimeStr);

#endif

extern void LIB_StringRor(uint8_t *rscAddr, uint32_t len, uint32_t rorCnt);

extern void LIB_StringRol(uint8_t *rscAddr, uint32_t len, uint32_t rolCnt);

extern void LIB_StringLsr(uint8_t *rscAddr, uint32_t len, uint32_t lsrCnt);

extern void LIB_StringLsl(uint8_t *rscAddr, uint32_t len, uint32_t lsrCnt);

extern void LIB_StringReplace(uint8_t *rscAddr, uint32_t len, uint32_t rscChar, uint32_t destChar);

extern uint32_t LIB_Strncmp(const char *prscStr, const char *pcmpStr, uint32_t count);

extern void LIB_LockKeyFuction(void);

extern void LIB_OpenKeyFuction(void);

#endif 
