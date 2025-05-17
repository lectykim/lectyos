

#include <linux/kernel.h>
#include <stdarg.h>
#include <stddef.h>

static char buf[1024];

extern int vsprintf(char * buff, const char * fmt, va_list args);

int printk(const char* fmt, ...)
{
	va_list args;
	int i;

	va_start(args,fmt);
	i = vsprintf(buf,fmt,args);
	va_end(args);
	__asm__("push %%fs \n\t"
		"push %%ds \n\t"
		"pop %%fs \n\t"
		"pushl %0 \n\t"
		"pushl $_buf \n\t"
		"pushl $0 \n\t"
		"call _tty_write \n\t"
		"addl $8,%%esp \n\t"
		"popl %0 \n\t"
		"pop %%fs"
		::"r"(i):"ax","cx","dx");
	return i;
}
