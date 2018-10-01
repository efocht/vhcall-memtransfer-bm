#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/un.h>
#include <unistd.h>
#include <string.h>
#include <sys/time.h>
#include <time.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <libvhcall.h>
#include <omp.h>
#include "vh/vhcalltestlib.h"

size_t bufflen = 64 * 1024 * 1024;
double bandwidth[256], totalbw = 0.0;

static inline uint64_t lhm(void *vehva)
{
	uint64_t ret;
	asm volatile("lhm.l %0,0(%1)":"=r"(ret):"r"(vehva));
	return ret;
}

static inline uint64_t stm(void)
{
	return lhm((void *)0x000000001000);
}

int
main (int argc, char **argv)
{
	int opt;
	int par = 1, huge = 0, i;

	while ((opt = getopt(argc, argv, "s:p:H")) != -1) {
		switch (opt) {
		case 's':
			bufflen = atoi(optarg) * 1024;
			break;
		case 'p':
			par = atoi(optarg);
			break;
		case 'H':
			huge = 1;
			break;
		default:
			printf("unknown option -%c\n", (char)opt);
			return(1);
		}
	}

	int64_t sym_vh2ve = -1, sym_alloc = -1, sym_free = -1;
	vhcall_handle h = vhcall_install("vh/libvhcalltestlib.so");
	if (h == (vhcall_handle)-1) {
		fprintf(stderr, "vhcall_install failed\n");
		exit(1);
	}
	sym_vh2ve = vhcall_find(h, "vh2ve_send");
	if (sym_vh2ve == -1) {
		fprintf(stderr, "vhcall_find failed\n");
		exit(1);
	}
	sym_alloc = vhcall_find(h, "alloc_buff");
	if (sym_alloc == -1) {
		fprintf(stderr, "vhcall_find failed\n");
		exit(1);
	}
	sym_free = vhcall_find(h, "free_buff");
	if (sym_free == -1) {
		fprintf(stderr, "vhcall_find failed\n");
		exit(1);
	}

		
	printf("prepared\n");
	omp_set_num_threads(par);

#pragma omp parallel shared(h, par, sym_vh2ve, sym_alloc, sym_free, bandwidth, totalbw) private(i)
	{
#pragma omp for
		for (i = 0; i < par; i++) {
			struct sendrecv_data srd;
			struct alloc_data ad1, ad2;

			void *p = malloc(bufflen);
			if (!p) {
				perror("malloc");
				exit(1);
			}
			
			ad1.size = bufflen;
			ad1.hugepage = huge;
			if (vhcall_invoke(sym_alloc, &ad1, sizeof(ad1), &ad2, sizeof(ad2))) {
				perror("vhcall_invoke alloc");
				exit(1);
			}
			// sleep a bit 
			usleep(200000);
			//printf("after malloc thread %d\n", omp_get_thread_num());
			srd.src = ad2.addr;
			srd.dst = p;
			srd.size = bufflen;
	
			//printf("before fill\n");
	
			uint64_t cntr, *dp = (uint64_t *)p;
			uint64_t bufflen64 = bufflen/sizeof(uint64_t);
#pragma _NEC ivdep
			for (cntr = 0; cntr < bufflen64; cntr++)
				dp[cntr] = cntr;
		
			//printf("after fill\n");
		
			struct timespec ts, te;
			clock_gettime(CLOCK_REALTIME, &ts); 
		
			uint64_t start = ts.tv_sec * 1000 * 1000 * 1000 + ts.tv_nsec;
			uint64_t start_stm = stm();
		
			if (vhcall_invoke(sym_vh2ve, &srd, sizeof(srd), 0, 0)) {
				perror("vhcall_invoke");
				exit(1);
			}
		
			uint64_t end_stm = stm();
		
			clock_gettime(CLOCK_REALTIME, &te);
			uint64_t stop = te.tv_sec * 1000 * 1000 * 1000 + te.tv_nsec;
		
			double timeSec = (double)(stop - start) / 1000 / 1000 / 1000;
			double sizeMeg = (double)bufflen / 1024 / 1024;
			bandwidth[omp_get_thread_num()] = sizeMeg/timeSec;
		
			//printf("write: %ld %ld (diff %ld) \n", start_stm, end_stm, end_stm - start_stm);
			if (vhcall_invoke(sym_free, &ad2, sizeof(ad2), 0, 0)) {
				perror("vhcall_invoke free");
				exit(1);
			}
			free(p);
		}
		//#pragma omp master
	}
		{
			for (i = 0; i < par; i++) {
				fprintf(stderr, "%f[MiB/s]\n", bandwidth[i]);
				totalbw += bandwidth[i];
			}
			fprintf(stderr, "Total: %f[MiB/s]\n", totalbw);
		}

	return 0;
}
