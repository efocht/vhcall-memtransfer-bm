#include <sys/types.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "libvepseudo.h"
#include "vhcalltestlib.h"

//#define MAP_HUGE_2MB    (21 << MAP_HUGE_SHIFT)

int64_t
alloc_buff(void *hdl, void *ip, size_t isize, void *op, size_t osize)
{
	int ret;
	struct alloc_data *inp = ip, *outp = op;

	outp->size = inp->size;
	outp->hugepage = inp->hugepage;
	if (inp->hugepage) {
		outp->addr = mmap(0, inp->size, PROT_READ | PROT_WRITE,
				  MAP_PRIVATE|MAP_HUGETLB|MAP_ANONYMOUS|MAP_POPULATE,
				  -1, 0);
		if (outp->addr == MAP_FAILED) {
			perror("vh_buffer mmap huge pages");
			return 1;
		}
	} else {
		outp->addr = malloc(inp->size);
		if (!outp->addr) {
			perror("vh_buffer malloc small pages");
			return 1;
		}
	}
	// initialize buffer, just to make sure it's in memory
	memset(outp->addr, 0xba, inp->size);
	return 0;
}

int64_t
free_buff(void *hdl, void *ip, size_t isize, void *op, size_t osize)
{
	struct alloc_data *inp = ip;
	if (inp->hugepage) {
		int ret = munmap(inp->addr, inp->size);
		if (ret) {
			perror("vh_buffer unmap hugepage");
		}
	} else
		free(inp->addr);
	return 0;
}

int64_t
ve2vh_send(void *hdl, void *ip, size_t isize, void *op, size_t osize)
{
	int ret;
	void *vh_buffer = NULL;
	struct sendrecv_data *data = ip;

	if (isize != sizeof(struct sendrecv_data)) {
		return 1;
	}
	vh_buffer = data->dst;
	if (!vh_buffer) {
		perror("vh_buffer is NULL?");
		return 1;
	}
	ret = ve_recv_data(hdl, (uint64_t)data->src, data->size, vh_buffer);
	if (ret != 0) {
		return 1;
	}

	return 0;
}

int64_t
vh2ve_send(void *hdl, void *ip, size_t isize, void *op, size_t osize)
{
	int ret;
	uint64_t ve_buffer =0;
	struct sendrecv_data *data = ip;

	if (isize != sizeof(struct sendrecv_data)) {
		return 1;
	}
	ve_buffer = (uint64_t)data->dst;
	if (!ve_buffer) {
		perror("vh_buffer is NULL?");
		return 1;
	}

	ret = ve_send_data(hdl, ve_buffer, data->size, data->src);
	if (ret != 0) {
		return 1;
	}

	return 0;
}

