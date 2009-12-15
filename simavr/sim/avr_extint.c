/*
	avr_extint.c

	Copyright 2008, 2009 Michel Pollet <buserror@gmail.com>

 	This file is part of simavr.

	simavr is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.

	simavr is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with simavr.  If not, see <http://www.gnu.org/licenses/>.
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "avr_extint.h"


static void avr_extint_irq_notify(struct avr_irq_t * irq, uint32_t value, void * param)
{
	avr_extint_t * p = (avr_extint_t *)param;
	avr_t * avr = p->io.avr;

	uint8_t mode = avr_regbit_get_array(avr, p->eint[irq->irq].isc, 2);
	int up = !irq->value && value;
	int down = irq->value && !value;
	switch (mode) {
		case 0:
			// unsuported
			break;
		case 1:
			if (up || down)
				avr_raise_interrupt(avr, &p->eint[irq->irq].vector);
			break;
		case 2:
			if (down)
				avr_raise_interrupt(avr, &p->eint[irq->irq].vector);
			break;
		case 3:
			if (up)
				avr_raise_interrupt(avr, &p->eint[irq->irq].vector);
			break;
	}
}

static void avr_extint_reset(avr_io_t * port)
{
	avr_extint_t * p = (avr_extint_t *)port;

	for (int i = 0; i < EXTINT_COUNT; i++) {
		avr_irq_register_notify(p->io.irq + i, avr_extint_irq_notify, p);

		if (p->eint[i].port_ioctl) {
			avr_irq_t * irq = avr_io_getirq(p->io.avr,
					p->eint[i].port_ioctl, p->eint[i].port_pin);

			avr_connect_irq(irq, p->io.irq + i);
		}
	}
}

static	avr_io_t	_io = {
	.kind = "extint",
	.reset = avr_extint_reset,
};

void avr_extint_init(avr_t * avr, avr_extint_t * p)
{
	p->io = _io;

	// allocate this module's IRQ
	p->io.irq_count = EXTINT_COUNT;
	p->io.irq = avr_alloc_irq(0, p->io.irq_count);
	p->io.irq_ioctl_get = AVR_IOCTL_EXTINT_GETIRQ();

	avr_register_io(avr, &p->io);
	for (int i = 0; i < EXTINT_COUNT; i++)
		avr_register_vector(avr, &p->eint[i].vector);
}
