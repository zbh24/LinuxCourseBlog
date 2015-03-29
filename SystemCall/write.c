#include<stdio.h>
int main() {

   char *s = "hello,zbh\n";
   asm volatile (
	"movl $4,%%eax;\n\t"
	"movl $1,%%ebx;\n\t"
	"movl %0,%%ecx;\n\t"
	"movl $10,%%edx;\n\t"
	"int $0x80;\n\t"
	:
	:"D"(s)
	);
   return 0;
}
