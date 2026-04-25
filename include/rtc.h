#ifndef GUARD_RTC_UTIL_H
#define GUARD_RTC_UTIL_H

#include "global.h"
#include "siirtc.h"
#include "config/overworld.h"
#include "constants/rtc.h"

extern struct Time gLocalTime;
extern enum Season gLoadedSeason;
extern const s32 sNumDaysInMonths[MONTH_COUNT];

bool8 IsBetweenHours(s32 hours, s32 begin, s32 end);
bool8 IsLeapYear(u32 year);
const u8 *GetTimeOfDayName(enum TimeOfDay timeOfDay);
const u8* GetSeasonName(enum Season);
enum Month GetMonth(void);
enum Season GetSeason(void);
enum TimeOfDay GenConfigTimeOfDay(enum TimeOfDay timeOfDay);
enum TimeOfDay GetTimeOfDay(void);
enum TimeOfDay GetTimeOfDayForDex(void);
enum TimeOfDay TryDecrementTimeOfDay(enum TimeOfDay timeOfDay);
enum TimeOfDay TryIncrementTimeOfDay(enum TimeOfDay timeOfDay);
enum Weekday GetDayOfWeek(void);
u16 ConvertDateToDayCount(u8 year, u8 month, u8 day);
u16 GetFullYear(void);
u16 RtcCheckInfo(struct SiiRtcInfo *rtc);
u16 RtcGetDayCount(struct SiiRtcInfo *rtc);
u16 RtcGetErrorStatus(void);
u32 ConvertBcdToBinary(u8 bcd);
u32 GetGen5TimeOfDayStart(enum TimeOfDay);
u32 RtcGetLocalDayCount(void);
u32 RtcGetMinuteCount(void);
u8 GetCurrentHour(void);
u8 GetCurrentMinute(void);
u8 GetDay(void);
u8 GetSeasonDay(void);
void CalcTimeDifference(struct Time *result, struct Time *t1, struct Time *t2);
void FormatDecimalTimeWithoutSeconds(u8 *dest, s8 hour, s8 minute, bool32 is24Hour);
void RtcCalcLocalTime(void);
void RtcCalcLocalTimeOffset(s32 days, s32 hours, s32 minutes, s32 seconds);
void RtcCalcTimeDifference(struct SiiRtcInfo *rtc, struct Time *result, struct Time *t);
void RtcDisableInterrupts(void);
void RtcGetDateTime(struct SiiRtcInfo *rtc);
void RtcGetInfo(struct SiiRtcInfo *rtc);
void RtcGetRawInfo(struct SiiRtcInfo *rtc);
void RtcGetStatus(struct SiiRtcInfo *rtc);
void RtcInit(void);
void RtcInitLocalTimeOffset(s32 hour, s32 minute);
void RtcReset(void);
void RtcRestoreInterrupts(void);
void UpdateLoadedSeason();

#endif // GUARD_RTC_UTIL_H
