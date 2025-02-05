#ifndef _ASM_X86_UNWIND_H
#define _ASM_X86_UNWIND_H

#include <linux/sched.h>
#include <linux/ftrace.h>
#include <asm/ptrace.h>
#include <asm/stacktrace.h>

struct unwind_state {
	struct stack_info stack_info;
	unsigned long stack_mask;
	struct task_struct *task;
	int graph_idx;
#ifdef CONFIG_DWARF_UNWIND
	union {
		struct pt_regs regs;
		char regs_arr[sizeof(struct pt_regs)];
	} u;
	unsigned long dw_sp;
	unsigned call_frame:1;
#elif defined(CONFIG_FRAME_POINTER)
	unsigned long *bp, *orig_sp;
	struct pt_regs *regs;
#else
	unsigned long *sp;
#endif
};

void __unwind_start(struct unwind_state *state, struct task_struct *task,
		    struct pt_regs *regs, unsigned long *first_frame);

bool unwind_next_frame(struct unwind_state *state);

unsigned long unwind_get_return_address(struct unwind_state *state);

static inline bool unwind_done(struct unwind_state *state)
{
	return state->stack_info.type == STACK_TYPE_UNKNOWN;
}

static inline
void unwind_start(struct unwind_state *state, struct task_struct *task,
		  struct pt_regs *regs, unsigned long *first_frame)
{
	first_frame = first_frame ? : get_stack_pointer(task, regs);

	__unwind_start(state, task, regs, first_frame);
}

#ifdef CONFIG_DWARF_UNWIND

#include <asm/dwarf.h>

static inline
unsigned long *unwind_get_return_address_ptr(struct unwind_state *state)
{
	if (unwind_done(state))
		return NULL;

	return &DW_PC(state);
}

static inline struct pt_regs *unwind_get_entry_regs(struct unwind_state *state)
{
	if (unwind_done(state))
		return NULL;

	return NULL;
}

#elif defined(CONFIG_FRAME_POINTER)

static inline
unsigned long *unwind_get_return_address_ptr(struct unwind_state *state)
{
	if (unwind_done(state))
		return NULL;

	return state->regs ? &state->regs->ip : state->bp + 1;
}

static inline struct pt_regs *unwind_get_entry_regs(struct unwind_state *state)
{
	if (unwind_done(state))
		return NULL;

	return state->regs;
}

#else /* !CONFIG_FRAME_POINTER */

static inline
unsigned long *unwind_get_return_address_ptr(struct unwind_state *state)
{
	return NULL;
}

static inline struct pt_regs *unwind_get_entry_regs(struct unwind_state *state)
{
	return NULL;
}

#endif /* CONFIG_FRAME_POINTER */

#endif /* _ASM_X86_UNWIND_H */
