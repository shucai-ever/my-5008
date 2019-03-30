#ifndef __PROC_H__
#define __PROC_H__

#include "type.h"
#include "error.h"

#define PRINT_PROC_IRQ(NAME, NUM, CNT) {printf("  %-24s %4d %10x\n", NAME, NUM, CNT);}
struct proc_irq_struct {

	PP_RESULT_E (*fn)(PP_S32 argc, const PP_CHAR **argv);
	PP_S32 irqNum;
	struct proc_irq_struct *next;
};

struct proc_device_struct {
    const PP_CHAR *pName;
	PP_RESULT_E (*fn)(PP_S32 argc, const PP_CHAR **argv);
	struct proc_device_struct *next;
};

void SYS_PROC_addIrq(struct proc_irq_struct *proc_irq);
int SYS_PROC_ProcessIrq(PP_S32 argc, const PP_CHAR **argv);

void SYS_PROC_addDevice(struct proc_device_struct *proc_device);

void SYS_PROC_PrintIrq(PP_S32 argc, const PP_CHAR **argv);
void SYS_PROC_PrintDevice(PP_S32 argc, const PP_CHAR **argv);

void SYS_PROC_initialize(void);

#endif /* __PROC_H__ */
