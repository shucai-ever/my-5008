/*
 * interrupt.h
 *
 *  Created on: 2017. 3. 31.
 *      Author: ibkim
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

void INTC_initialize(void);
void INTC_swi_enable(void);
void INTC_swi_disable(void);
void INTC_swi_clean(void);
void INTC_swi_trigger(void);
uint32 INTC_irq_mask(sint32 IN vecId);
void INTC_irq_unmask(uint32 IN mask);
void INTC_irq_clean(sint32 IN vecId);
void INTC_irq_clean_all(void);
void INTC_irq_disable(uint32 IN vecId);
void INTC_irq_disable_all(void);
void INTC_irq_enable(sint32 IN vecId);
void INTC_irq_set_priority(uint32 IN prio1, uint32 IN prio2);
void INTC_irq_config(sint32 IN vecId, uint32 IN edge);
uint32 INTC_get_all_pend(void);

#endif
