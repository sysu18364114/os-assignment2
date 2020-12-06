
//  2020/12/1 //

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>

#define TLB_SIZE 16 // TLB大小
#define FRAME_SIZE 256 // 帧大小
#define ADDR_SIZE 1000 // 文件addresses.txt中的虚拟地址条目数

typedef struct
{
	int frameNum;
	int pageNum;
	int access;
} TLB; // TLB结构定义

char phyMem[FRAME_SIZE][FRAME_SIZE]; // 物理内存
int page[FRAME_SIZE];				 // 页表
int exist[FRAME_SIZE];				 // 用于说明某页是否存在数据
int virAddr[ADDR_SIZE];					 // 存储读入的虚拟地址，由于addresses.txt中有1000个虚拟地址，故设置数组大小为1000

int tlbHit = 0; // tlb命中次数
int miss = 0;	// page失效次数
int count = 0;	// 记录读入的虚拟地址数目

FILE *addrFilePtr;		// 虚拟地址文件
FILE *backStoreFilePtr; // 存储数据的文件
FILE *outFilePtr; // 储存输出的文件

TLB tlb[TLB_SIZE]; // 实例化TLB
int ind = 0; // 全局变量ind，用于模拟FIFO中的队列功能

// // 打印十进制数转化为二进制数的数值，用于调试程序
// void printf_bin(int num)
// {
// 	int ii, jj, kk;
// 	unsigned char *p = (unsigned char *)&num + 3; // p先指向num后面第3个字节的地址，即num的最高位字节地址

// 	for (ii = 0; ii < 4; ii++) // 依次处理4个字节(32位）
// 	{
// 		jj = *(p - ii);				// 取每个字节的首地址，从高位字节到低位字节，即p p-1 p-2 p-3地址处
// 		for (kk = 7; kk >= 0; kk--) // 处理每个字节的8个位，注意字节内部的二进制数是按照人的习惯存储！
// 		{
// 			if (jj & (1 << kk)) // 1左移kk位，与单前的字节内容jj进行或运算，如kk=7时，00000000&10000000=0 ->该字节的最高位为0
// 				printf("1");
// 			else
// 				printf("0");
// 		}
// 		printf(" "); // 每8位加个空格，方便查看
// 	}
// 	printf("\r\n");
// }

// 实现虚拟内存管理器的功能模块
int main(int argc, char *argv[])
{
	int mode = 0; // 页面置换策略的标志位，f表示FIFO策略，l表示LRU策略
	mode = getopt(argc, argv, "fl"); // 获取传入的页面置换策略参数

	char *backStoreFilename = argv[2]; // 储存文件BACKING_STORE.bin文件名
	char *addrFilename = argv[3]; // 地址文件correct.txt文件名

	// 打开两个输入文件，一个输出文件
	addrFilePtr = fopen(addrFilename, "r");
	backStoreFilePtr = fopen(backStoreFilename, "r");
	outFilePtr = fopen("out.txt", "w");

	int pageNum = 0;
	int frameNum = 0;
	int offset = 0;
	char res;

	for (int ii = 0; ii < FRAME_SIZE; ii++)
		exist[ii] = 0;
	for (int ii = 0; ii < TLB_SIZE; ii++)
	{
		tlb[ii].frameNum = -1;
		tlb[ii].pageNum = -1;
		tlb[ii].access = 10000; // access设为一个较大的值，
	}

	for (int count = 0; count < ADDR_SIZE; count++)
	{
		fscanf(addrFilePtr, "%d", &virAddr[count]); // 从文件中读入虚拟地址
		pageNum = (virAddr[count] >> 8) & 0xff; // 取得虚拟地址的高8位写入pageNum
		offset = virAddr[count] % FRAME_SIZE;
		int inTlb = 0;
		if (exist[pageNum]) // 页存在于page中
		{
			for (int ii = 0; ii < TLB_SIZE; ii++) // 访问TLB
			{
				if (tlb[ii].pageNum == pageNum) // TLB命中
				{
					res = phyMem[tlb[ii].frameNum][offset];
					printf("Virtual address: %d Physical address: %d Value: %d\n", virAddr[count], FRAME_SIZE * tlb[ii].frameNum + offset, res);
					tlbHit++;
					tlb[ii].access = 0; // 更新为0，说明该页最近被访问过
					inTlb = 1;
				}
				else
					tlb[ii].access++;
			}

			if (!inTlb) // 在page中，但是不在tlb中，利用LRU放入tlb中
			{
				res = phyMem[page[pageNum]][offset];
				printf("Virtual address: %d Physical address: %d Value: %d\n", virAddr[count], FRAME_SIZE * page[pageNum] + offset, res);
	
				// 根据传入的命令行参数选择页面置换算法
				switch (mode)
				{
				case 'f': // FIFO
				{
					ind = (ind++) % TLB_SIZE; // 利用全局变量ind实现FIFO策略
					// 换入页面，即直接向TLB中写入数据
					tlb[ind].frameNum = page[pageNum];
					tlb[ind].pageNum = pageNum;
					// 注意，FIFO策略不用更新access值
					break;
				}

				case 'l': // LRU
				{
					int max = 0;
					int maxInd = 0;
					// 找到最近最久未被使用的页面下标
					for (int jj = 0; jj < TLB_SIZE; jj++)
					{
						if (tlb[jj].access > max)
						{
							maxInd = jj;
							max = tlb[jj].access;
						}
					}
					// LRU策略写入数据到TLB中
					tlb[maxInd].frameNum = page[pageNum];
					tlb[maxInd].pageNum = pageNum;
					tlb[maxInd].access = 0;
					break;
				}
				default:
					printf("Mode Error\n");
					return 0;
				}
			}
			inTlb = 0;
		}
		else // 不在page中，发生缺页错误
		{
			miss++;
			// 寻找对应的页并且进行读取
			fseek(backStoreFilePtr, pageNum * FRAME_SIZE * sizeof(char), 0);
			fread(phyMem[frameNum], sizeof(char), FRAME_SIZE, backStoreFilePtr);
			page[pageNum] = frameNum; 
			exist[pageNum] = 1;

			res = phyMem[frameNum][offset];
			printf("Virtual address: %d Physical address: %d Value: %d\n", virAddr[count], FRAME_SIZE * frameNum + offset, res);
			frameNum++;
	
			// 逻辑同上一个switch，不再赘述
			switch (mode)
			{
			case 'f': // FIFO 
			{
				ind = (ind++) % TLB_SIZE;
				tlb[ind].frameNum = page[pageNum];
				tlb[ind].pageNum = pageNum;
				break;
			}
			case 'l': // LRU
			{
				int max = 0;
				int maxInd = 0;
				for (int j = 0; j < TLB_SIZE; j++)
				{
					if (tlb[j].access > max)
					{
						maxInd = j;
						max = tlb[j].access;
					}
				}

				tlb[maxInd].frameNum = page[pageNum];
				tlb[maxInd].pageNum = pageNum;
				tlb[maxInd].access = 0;

				// 将其他tlb中的条目access+1
				for (int jj = 0; jj < TLB_SIZE; jj++)
				{
					if (jj == maxInd)
						continue;
					tlb[jj].access++;
				}
				break;
			}
			default:
				printf("Mode Error\n");
				return 0;
			}
		}
	}

	double tlbRate = tlbHit / (double)ADDR_SIZE; // TLB命中率
	double pageRate = miss / (double)ADDR_SIZE;  // 缺页率
	printf("Page miss rate %f , TLB hit rate %f\n", pageRate, tlbRate); // 输出统计信息

	// 关闭文件
	fclose(addrFilePtr);
	fclose(backStoreFilePtr);
	fclose(outFilePtr);
}
