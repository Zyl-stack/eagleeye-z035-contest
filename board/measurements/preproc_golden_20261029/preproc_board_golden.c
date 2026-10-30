#define _POSIX_C_SOURCE 200809L
#include <errno.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <time.h>
#include <unistd.h>

enum { FRAME=691200, PLANE=409600, TENSOR=1228800, POOL=0x400000 };
static volatile uint32_t *regmap(int fd, off_t addr) {
  void *p=mmap(NULL,4096,PROT_READ|PROT_WRITE,MAP_SHARED,fd,addr);
  if(p==MAP_FAILED){perror("mmap regs");exit(2);} return p;
}
static void wr(volatile uint32_t *p,unsigned o,uint32_t v){__sync_synchronize();p[o/4]=v;__sync_synchronize();}
static uint32_t rd(volatile uint32_t *p,unsigned o){uint32_t v=p[o/4];__sync_synchronize();return v;}
static uint64_t ns(void){struct timespec t;clock_gettime(CLOCK_MONOTONIC,&t);return (uint64_t)t.tv_sec*1000000000ULL+t.tv_nsec;}
static int dma_start(volatile uint32_t *p,int out,uint32_t addr,uint32_t len){
  unsigned cr=out?0x30:0,sr=out?0x34:4,ar=out?0x48:0x18,lr=out?0x58:0x28;
  wr(p,cr,4);uint64_t end=ns()+100000000ULL;
  while((rd(p,cr)&4)&&ns()<end){}
  if(rd(p,cr)&4){fprintf(stderr,"DMA reset timeout\n");return -1;}
  wr(p,sr,0xffffffffU);wr(p,cr,1);wr(p,ar,addr);wr(p,lr,len);return 0;
}
static int dma_done(volatile uint32_t *p,int out){
  uint32_t s=rd(p,out?0x34:4);if(s&0x770U)return -1;return (s&2U)?1:0;
}
int main(int argc,char **argv){
  if(argc!=3){fprintf(stderr,"usage: %s captured.rgb board_tensor.f32\n",argv[0]);return 2;}
  int in=open(argv[1],O_RDONLY);if(in<0){perror("input");return 2;}
  uint8_t *rgb=malloc(FRAME);if(!rgb)return 2;
  if(read(in,rgb,FRAME)!=FRAME){fprintf(stderr,"input must be 691200 bytes\n");return 2;}
  uint8_t extra;if(read(in,&extra,1)!=0){fprintf(stderr,"input has trailing data\n");return 2;}close(in);
  int fd=open("/dev/mem",O_RDWR|O_SYNC);if(fd<0){perror("/dev/mem");return 2;}
  uint8_t *mem=mmap(NULL,POOL,PROT_READ|PROT_WRITE,MAP_SHARED,fd,0x1e300000);
  if(mem==MAP_FAILED){perror("mmap DDR");return 2;}
  volatile uint32_t *ip=regmap(fd,0x40400000),*di=regmap(fd,0x40410000),*dr=regmap(fd,0x40420000),*dg=regmap(fd,0x40430000),*db=regmap(fd,0x40440000);
  uint32_t v=rd(ip,0x14);printf("PREPROC_VERSION=0x%08x\n",v);fflush(stdout);
  if(v!=0x00010000U){fprintf(stderr,"version mismatch; no DMA started\n");return 3;}
  memcpy(mem,rgb,FRAME);memset(mem+0x100000,0,TENSOR);__sync_synchronize();
  if(dma_start(dr,1,0x1e400000,PLANE)||dma_start(dg,1,0x1e400000+PLANE,PLANE)||dma_start(db,1,0x1e400000+2*PLANE,PLANE))return 3;
  wr(ip,0x04,0x1ff);wr(ip,0,1);
  if(dma_start(di,0,0x1e300000,FRAME))return 3;
  uint64_t end=ns()+1000000000ULL;int a,b,c,d;uint32_t status;
  do{
    status=rd(ip,4);a=dma_done(di,0);b=dma_done(dr,1);c=dma_done(dg,1);d=dma_done(db,1);
    if(a<0||b<0||c<0||d<0)break;
    if((status&2U)&&a&&b&&c&&d)break;
  }while(ns()<end);
  printf("IP_STATUS=0x%08x CYCLES=%u INPUT=%u OUTPUT=%u DMA_IN=0x%08x DMA_R=0x%08x DMA_G=0x%08x DMA_B=0x%08x\n",status,rd(ip,8),rd(ip,0xc),rd(ip,0x10),rd(di,4),rd(dr,0x34),rd(dg,0x34),rd(db,0x34));
  if(!(status&2U)||a!=1||b!=1||c!=1||d!=1||(status&0x1fcU)||rd(ip,0xc)!=230400U||rd(ip,0x10)!=102400U){fprintf(stderr,"hardware run failed; no output published\n");return 4;}
  int out=open(argv[2],O_CREAT|O_EXCL|O_WRONLY,0600);if(out<0){perror("output");return 2;}
  __sync_synchronize();size_t n=0;while(n<TENSOR){ssize_t z=write(out,mem+0x100000+n,TENSOR-n);if(z<=0){perror("write tensor");return 5;}n+=(size_t)z;}
  if(close(out)){perror("close tensor");return 5;}
  printf("BOARD_TENSOR_BYTES=%u SHAPE=1x3x320x320 DTYPE=FP32_LE_NCHW\n",TENSOR);
  return 0;
}
