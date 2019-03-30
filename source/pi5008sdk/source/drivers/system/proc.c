#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "osal.h"

#include "debug.h"
#include "sub_intr.h"
#include "proc.h"

static struct proc_irq_struct *proc_irqs = NULL;
static struct proc_device_struct *proc_devices = NULL;

//IRQ
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PP_RESULT_E do_proc_irq_null(PP_S32 argc, const PP_CHAR **argv)
{
	printf("\n--------------------------------------------------------------------------------------------\n");

	return(eSUCCESS);
}


struct proc_irq_struct default_irqs[] = {

	{ .fn = do_proc_irq_null,	.irqNum = -1,	.next = (void*)0, },
};

void SYS_PROC_addIrq(struct proc_irq_struct *proc_irq)
{
	struct proc_irq_struct *p = proc_irqs;

	if( (proc_irq) && (proc_irq->fn) )
	{
		while (p) 
		{
			if(p->irqNum == proc_irq->irqNum)
			{
				printf("ERROR!! [%s] Invalid irqNum(%d). registered already.\n", __FUNCTION__, proc_irq->irqNum);
				return;
			}
			p = p->next;
		}
		//printf("register irqNum(%d)\n", proc_irq->irqNum);

		proc_irq->next = proc_irqs;
		proc_irqs = proc_irq;
	}
}

int SYS_PROC_ProcessIrq(int argc, const char **argv)
{
	int i, irqNum;
	struct proc_irq_struct *p = proc_irqs;

	printf("  %-24s %4s %10s\n", "Name", "irq", "count");

	for(i = 0; i < 32; i++) //D10 TOP
	{
		irqNum = i;

		p = (struct proc_irq_struct *)proc_irqs;
		while (p) 
		{
			if(p->irqNum == irqNum)
			{
				if(p->fn)
				{
					p->fn(argc, argv);
				}
				break;
			}
			p = p->next;
		}
	}

	for(i = 100; i < 132; i++) //SUB0 PVIRX
	{
		irqNum = i;

		p = (struct proc_irq_struct *)proc_irqs;
		while (p) 
		{
			if(p->irqNum == irqNum)
			{
				if(p->fn)
				{
					p->fn(argc, argv);
				}
				break;
			}
			p = p->next;
		}
	}

	{	//SUB1 Diagnosis
		irqNum = 200/*IRQ_1_VECTOR*/+IRQ_CRC_QUAD;

		p = (struct proc_irq_struct *)proc_irqs;
		while (p) 
		{
			if(p->irqNum == irqNum)
			{
				if(p->fn)
				{
					p->fn(argc, argv);
				}
				break;
			}
			p = p->next;
		}
	}

	#if 0
	for(i = 300; i < 332; i++) //SUB2 ISP
	{
		irqNum = i;

		p = (struct proc_irq_struct *)proc_irqs;
		while (p) 
		{
			if(p->irqNum == irqNum)
			{
				if(p->fn)
				{
					p->fn(argc, argv);
				}
				break;
			}
			p = p->next;
		}
	}
	#endif

	{	//SUB3 VSYNC
		irqNum = 400/*IRQ_24_VECTOR*/;

		p = (struct proc_irq_struct *)proc_irqs;
		while (p) 
		{
			if(p->irqNum == irqNum)
			{
				if(p->fn)
				{
					p->fn(argc, argv);
				}
				break;
			}
			p = p->next;
		}
	}


	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////

//DEVICE
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
static PP_RESULT_E do_proc_device_null(PP_S32 argc, const PP_CHAR **argv)
{
	printf("\n--------------------------------------------------------------------------------------------\n");

	return(eSUCCESS);
}


struct proc_device_struct default_devices[] = {

	{ .fn = do_proc_device_null,	.next = (void*)0, },
};

void SYS_PROC_addDevice(struct proc_device_struct *proc_device)
{
	struct proc_device_struct *p = proc_devices;

	if(proc_device)
	{
		while (p) 
		{
			if(p->fn == proc_device->fn)
			{
				printf("ERROR!! [%s] Invalid fn. registered already.\n", __FUNCTION__);
				return;
			}
			p = p->next;
		}

		proc_device->next = proc_devices;
		proc_devices = proc_device;
	}
}

int SYS_PROC_PrintDeviceName(int argc, const char **argv)
{
	struct proc_device_struct *p = proc_devices;

    while (p) {
        if(p->pName)
        {
            printf("%s\n", p->pName);
        }
		p = p->next;
	}

	return 0;
}
int SYS_PROC_ProcessDevice(int argc, const char **argv)
{
	struct proc_device_struct *p = proc_devices;

	while (p) {

		p->fn(argc, argv);
		p = p->next;
	}

	return 0;
}
////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SYS_PROC_PrintIrq(int argc, const char **argv)
{
	printf("PI5008 IRQ  --------- SUB0:1xx, SUB1:2xx, SUB2:3xx, SUB3:4xx ----------\n");

	SYS_PROC_ProcessIrq(argc, argv);
}

void SYS_PROC_PrintDevice(int argc, const char **argv)
{
    if( (argc) && (!strcmp(argv[0], "name")) )
    {
        SYS_PROC_PrintDeviceName(argc, argv);
    }
    else
    {
        SYS_PROC_ProcessDevice(argc, argv);
    }
}

void SYS_PROC_initialize(void)
{
	//add default proc_irqs
	SYS_PROC_addIrq(default_irqs);

	//add default proc_irqs
	SYS_PROC_addDevice(default_devices);
}
