/*
 * SPDX-License-Identifier: BSD-2-Clause
 *
 * Copyright (c) 2019 Western Digital Corporation or its affiliates.
 *
 * Authors:
 *   Anup Patel <anup.patel@wdc.com>
 */

#include <sbi/riscv_barrier.h>
#include <sbi/riscv_locks.h>

int spin_lock_check(spinlock_t *lock)
{
	return (lock->lock == __RISCV_SPIN_UNLOCKED) ? 0 : 1;
}

int spin_trylock(spinlock_t *lock)
{
	unsigned long inc = 1u << TICKET_SHIFT;
	unsigned long mask = 0xffffu << TICKET_SHIFT;
	u32 l0, tmp1, tmp2;

	__asm__ __volatile__(
		/* Get the current lock counters. */
		"1:	lw	%0, %3\n"
		"	slli	%2, %0, %6\n"
		"	and	%2, %2, %5\n"
		"	and	%1, %0, %5\n"
		/* Is the lock free right now? */
		"	bne	%1, %2, 2f\n"
		"	add	%0, %0, %4\n"
		/* Acquire the lock. */
		"	sw	%0, %3\n"
		"2:"
		: "=&r"(l0), "=&r"(tmp1), "=&r"(tmp2), "+A"(*lock)
		: "r"(inc), "r"(mask), "I"(TICKET_SHIFT)
		: "memory");

	return l0 == 0;
}

void spin_lock(spinlock_t *lock)
{
	unsigned long inc = 1u << TICKET_SHIFT;
	unsigned long mask = 0xffffu;
	u32 l0, tmp1, tmp2, tmp3;

	__asm__ __volatile__(
		/* Atomically increment the next ticket. */
                "       lw %1, %4\n"
                "       add %0, %1, %5\n"
                "       sw %0, %4\n"

		/* Did we get the lock? */
		"	srli	%2, %1, %7\n"
		"	and	%2, %2, %6\n"
		"1:	and	%3, %1, %6\n"
		"	beq	%2, %3, 2f\n"

		/* If not, then spin on the lock. */
		"	lw	%1, %4\n"
		RISCV_ACQUIRE_BARRIER
		"	j	1b\n"
		"2:"
		: "=r"(tmp3), "=&r"(l0), "=&r"(tmp1), "=&r"(tmp2), "+A"(*lock)
		: "r"(inc), "r"(mask), "I"(TICKET_SHIFT)
		: "memory");
}

void spin_unlock(spinlock_t *lock)
{
	__smp_store_release(&lock->lock, __RISCV_SPIN_UNLOCKED);
}
