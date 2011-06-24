
#include "tpe.h"

/*

This file contains many different ways to locate a symbol's address based on name,
and tries to be the most efficient about it. It uses your System.map file as a last
resort.

*/

#define SYSTEM_MAP_PATH "/boot/System.map-"
#define MAX_LEN 256

// callback for find_symbol_address

static int find_symbol_callback(struct kernsym *sym, const char *name, struct module *mod,
	unsigned long addr) {

	if (sym->found) {
		sym->end_addr = addr;
		return 1;
	}

	// this symbol was found. the next callback will be the address of the next symbol
	if (name && sym->name && !strcmp(name, sym->name)) {
		sym->addr = addr;
		sym->found = true;
	}

	return 0;
}

// find this symbol

struct kernsym *find_symbol_address(const char *symbol_name) {

	struct kernsym *sym;
	unsigned long *addr;
	int ret;

	sym = kmalloc(sizeof(sym), GFP_KERNEL);

	if (sym == NULL)
		return -ENOMEM;

	sym->name = symbol_name;
	sym->found = 0;

	ret = kallsyms_on_each_symbol(find_symbol_callback, sym);

	if (!ret)
		return -EFAULT;

	sym->size = (unsigned int)sym->end_addr - (unsigned int)sym->addr;

	return sym;
}

// RHEL kernels don't compile with CONFIG_PRINTK_TIME. lame.

void up_printk_time(void) {

	int *printk_time_ptr;

	printk_time_ptr = find_symbol_address("printk_time");

	// no dice? oh well, no biggie
	if (IS_ERR(printk_time_ptr))
		return;

	if (*printk_time_ptr == 0) {
		*printk_time_ptr = 1;
		printk("Flipped printk_time to 1 because, well, I like it that way!\n");
	}

}

