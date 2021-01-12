# Switches between processes by pushing registers of old Proc to its own stack,
# calling scheduler, then loading the registers of the new Proc from its own stack.

	.globl running, scheduler, tswitch
tswitch:
SAVE:
	pushal
	pushfl
	movl running, %ebx
	movl %esp, 4(%ebx)

FIND:
	call scheduler

RESUME:
	movl running, %ebx
	movl 4(%ebx), %esp
	popfl
	popal
	ret
