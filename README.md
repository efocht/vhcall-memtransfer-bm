# A simple benchmark for VE <-> VH memory transfer speed

Implemented as a VHcall module this tests the privileged DMA
performance with various buffer sizes by

* allocating a buffer on the VH
* calling either `ve_send_data()` or `ve_recv_data()`
* measuring the time and calculating the bandwidth

These mechanisms are using unregistered and unpinned buffers
which are translated and pinned on the fly.

## Building

```
git clone https://github.com/efocht/vhcall-memtransfer-bm.git
cd vhcall-memtransfer-bm

make
```

## Executing

### ve2vh / vh2ve

```
./ve2vh [-s <buffer_size_kb>] [-p <threads>] [-H]

./vh2ve [-s <buffer_size_kb>] [-p <threads>] [-H]
```

The command line options are:

* `-s <size>` : buffer size in kB. Default is 64MB.
* `-p <threads>` : number of OpenMP threads. Default is 1.
* `-H` : enable huge pge buffer on VH.


### `scan_ve2vh.sh` and `scan_vh2ve.sh`

The scripts run `ve2vh` or `vh2ve` with a range of buffer sizes.

Parallel execution ca be enabled by setting the env variable NPAR to the number of desired OpenMP threads. Each thread will transfer the same size of buffer.

Hue page execution is enabled by setting the environment variable HUGE to 1.

## Example

The results below were obtained with a tuned version of VEOS which uses bulk translation and pinning for the DMA engine using the system/privileged descriptors.

```
[root@aurora0 mem_transfer]# HUGE=1 ./scan_ve2vh.sh 
 buff kb   BW MiB/s
      32       110
      64       291
     128       550
     256       928
     512      1566
    1024      2949
    2048      4421
    4096      6905
    8192      8272
   16384      9570
   32768     10129
   65536     10545
  131072     10690
  262144     10669
  524288     10802
 1048576     10872
[root@aurora0 mem_transfer]# ./scan_ve2vh.sh 
 buff kb   BW MiB/s
      32       106
      64       263
     128       477
     256       839
     512      1714
    1024      1755
    2048      2504
    4096      3071
    8192      4320
   16384      5378
   32768      5670
   65536      5328
  131072      5817
  262144      5748
  524288      5557
 1048576      5527
[root@aurora0 mem_transfer]# ./scan_vh2ve.sh 
 buff kb   BW MiB/s
      32       126
      64       248
     128       499
     256       900
     512      1548
    1024      1547
    2048      2458
    4096      3456
    8192      4709
   16384      5314
   32768      5480
   65536      5636
  131072      5547
  262144      5551
  524288      5469
 1048576      5569
[root@aurora0 mem_transfer]# HUGE=1 ./scan_vh2ve.sh 
 buff kb   BW MiB/s
      32       143
      64       250
     128       535
     256       834
     512      1833
    1024      2893
    2048      4554
    4096      6773
    8192      8095
   16384      9148
   32768      9693
   65536      9992
  131072      9669
  262144     10271
  524288     10316
 1048576     10198
```

