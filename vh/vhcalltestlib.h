#ifndef VHCALLTESTLIB_H_
#define VHCALLTESTLIB_H_

struct sendrecv_data {
	void *src;
	void *dst;
	size_t size;
};

struct alloc_data {
	void *addr;
	size_t size;
	int hugepage;
};

#endif /*VHCALLTESTLIB_H_*/
