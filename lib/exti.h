
#ifndef	__EXTI_H
#define	__EXTI_H

#include	"config.h"

#define	EXT_INT0			0	//初始化外中断0
#define	EXT_INT1			1	//初始化外中断1
#define	EXT_INT2			2	//初始化外中断2
#define	EXT_INT3			3	//初始化外中断3
#define	EXT_INT4			4	//初始化外中断4

#define	EXT_MODE_RiseFall	0	//上升沿/下降沿中断
#define	EXT_MODE_Fall		1	//下降沿中断

/*
 * Configure INT0 or INT1.
 * MODE: EXT_MODE_RiseFall, EXT_MODE_Fall
 * PRIORITY: PriorityHigh, PriorityLow
 * INTERRUPT: ENABLE, DISABLE
 */
#define EXTI0_INIT(MODE, PRIORITY, INTERRUPT) do { \
	EX0 = ((INTERRUPT) == ENABLE); \
	PX0 = ((PRIORITY) == PriorityHigh); \
	IT0 = ((MODE) == EXT_MODE_Fall); \
} while (0)
#define EXTI1_INIT(MODE, PRIORITY, INTERRUPT) do { \
	EX1 = ((INTERRUPT) == ENABLE); \
	PX1 = ((PRIORITY) == PriorityHigh); \
	IT1 = ((MODE) == EXT_MODE_Fall); \
} while (0)

/*
 * Configure INT2, INT3, or INT4. Operation is fixed to falling-edge and
 * low priority.
 * INTERRUPT: ENABLE, DISABLE
 */
#define EXTI2_INIT(INTERRUPT) do { \
	INT_CLKO = (INT_CLKO & ~(1 << 4)) | (((INTERRUPT) == ENABLE) << 4); \
} while (0)
#define EXTI3_INIT(INTERRUPT) do { \
	INT_CLKO = (INT_CLKO & ~(1 << 5)) | (((INTERRUPT) == ENABLE) << 5); \
} while (0)
#define EXTI4_INIT(INTERRUPT) do { \
	INT_CLKO = (INT_CLKO & ~(1 << 6)) | (((INTERRUPT) == ENABLE) << 6); \
} while (0)

#endif
