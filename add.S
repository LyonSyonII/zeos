#include <asm.h>

ENTRY (addASM)
	movl 8(%esp), %eax
    addl 4(%esp), %eax
    retl