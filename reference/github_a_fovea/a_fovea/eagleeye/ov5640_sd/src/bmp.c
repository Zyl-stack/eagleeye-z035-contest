#include "xil_types.h"
#include "ff.h"
#include "stdio.h"
#include "string.h"
#include "math.h"


unsigned char read_line_buf[1920 * 3];
unsigned char Write_line_buf[1920 * 3];
void bmp_read(char * bmp,u8 *frame,u32 stride, FIL *fil)
{
	short y,x;
	short Ximage;
	short Yimage;
	u32 iPixelAddr = 0;
	FRESULT res;
	unsigned char TMPBUF[64];
	unsigned int br;

	res = f_open(fil, bmp, FA_OPEN_EXISTING | FA_READ);
	if(res != FR_OK)
	{
		printf("Failed to Read!\r\n") ;
		return ;
	}
	res = f_read(fil, TMPBUF, 54, &br);
	if(res != FR_OK)
	{
		f_close(fil);
		printf("Failed to Read!\r\n") ;
		return ;
	}
	Ximage=(unsigned short int)TMPBUF[19]*256+TMPBUF[18];
	Yimage=(unsigned short int)TMPBUF[23]*256+TMPBUF[22];
	iPixelAddr = (Yimage-1)*stride ;

	for(y = 0; y < Yimage ; y++)
	{
		f_read(fil, read_line_buf, Ximage * 3, &br);
		for(x = 0; x < Ximage; x++)
		{
			frame[x * 3 + iPixelAddr + 0] = read_line_buf[x * 3 + 0];
			frame[x * 3 + iPixelAddr + 1] = read_line_buf[x * 3 + 1];
			frame[x * 3 + iPixelAddr + 2] = read_line_buf[x * 3 + 2];
		}
		iPixelAddr -= stride;
	}
	f_close(fil);
}


/* void bmp_write(char * name, char *head_buf, char *data_buf, u32 stride, FIL *fil)
{
	short y,x;
	short Ximage;
	short Yimage;
	u32 iPixelAddr = 0;
	FRESULT res;
	unsigned int br;         // File R/W count

	memset(&Write_line_buf, 0, 1920*3) ;

	res = f_open(fil, name, FA_CREATE_ALWAYS | FA_WRITE);
	if(res != FR_OK)
	{
		printf("Failed to Write!\r\n") ;
		return ;
	}
 	res = f_write(fil, head_buf, 54, &br) ;
	if(res != FR_OK)
	{
		f_close(fil);
		printf("Failed to Write!\r\n") ;
		return ;
	}
	Ximage=(unsigned short)head_buf[19]*256+head_buf[18];
	Yimage=(unsigned short)head_buf[23]*256+head_buf[22];
	iPixelAddr = (Yimage-1)*stride ;
	for(y = 0; y < Yimage ; y++)
	{
		for(x = 0; x < Ximage; x++)
		{
			Write_line_buf[x*3 + 0] = data_buf[x*3 + iPixelAddr + 0] ;
			Write_line_buf[x*3 + 1] = data_buf[x*3 + iPixelAddr + 1] ;
			Write_line_buf[x*3 + 2] = data_buf[x*3 + iPixelAddr + 2] ;
		}
		res = f_write(fil, Write_line_buf, Ximage*3, &br) ;

		iPixelAddr -= stride;
	}
	f_close(fil);
} */

/* void bmp_write1(char *name, char *head_buf, char *data_buf, u32 stride, FIL *fil) {
	int n = 0;
	int RN = 0;int GN = 0;int BN = 0;
	
    short y, x;
    short Ximage;
    short Yimage;
    u32 iPixelAddr = 0;
    FRESULT res;
    unsigned int br; // File R/W count
    // 打开文件以写入
    res = f_open(fil, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Failed to open file!\r\n");
        return;
    }
	
    // 预计算噪声以避免实时计算
	#define MAX_WIDTH  1280
	#define MAX_HEIGHT 720
	int noise_array[MAX_WIDTH * MAX_HEIGHT];

    for (int i = 0; i < Ximage * Yimage; i++) {
        float noise1 = 10000 * sin(2 * M_PI * FZ1 * i / FS);
        noise_array[i] = (int)noise1;
    }
	
    // 从头部信息中提取图像宽度和高度
    Ximage = (unsigned short)(head_buf[19] * 256 + head_buf[18]);
    Yimage = (unsigned short)(head_buf[23] * 256 + head_buf[22]);
    iPixelAddr = (Yimage - 1) * stride;

    // 按行写入像素数据
    for (y = 0; y < Yimage; y++) {
        for (x = 0; x < Ximage; x++) {
            // 将 RGB 值转换为二进制字符串并写入文件
            int index = y * Ximage + x;
            int noise = noise_array[index];
			n += 1;
			RN = (int)(data_buf[x * 3 + iPixelAddr + 0] + noise);
			GN = (int)(data_buf[x * 3 + iPixelAddr + 1] + noise);
			BN = (int)(data_buf[x * 3 + iPixelAddr + 2] + noise);
			
			f_printf(fil, "%d\n%d\n%d\n", RN, GN, BN); // 红色绿色蓝色		
			//f_printf(fil, "%d\n", (int)((int)data_buf[x * 3 + iPixelAddr + 1] + noise));	
			//f_printf(fil, "%d\n", (int)((int)data_buf[x * 3 + iPixelAddr + 2] + noise));
        }
        // 更新像素地址
        iPixelAddr -= stride;
    }
	
    // 关闭文件
    f_close(fil);
}
 */
 
#define FZ1 350          // 噪声频率
#define FS 1000          // 采样频率

void bmp_write(char *name, char *head_buf, char *data_buf, u32 stride, FIL *fil) {
    int n = 0;
    short y, x;
    short Ximage, Yimage;
    u32 iPixelAddr = 0;
    FRESULT res;
    unsigned int br; // 文件读写计数

    // 打开文件以写入
    res = f_open(fil, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res != FR_OK) {
        printf("Failed to open file!\r\n");
        return;
    }

    // 从头部信息中提取图像宽度和高度
    Ximage = (unsigned short)(head_buf[19] * 256 + head_buf[18]);
    Yimage = (unsigned short)(head_buf[23] * 256 + head_buf[22]);
    iPixelAddr = (Yimage - 1) * stride;

    // 预计算噪声以避免实时计算
	#define MAX_WIDTH  1280
	#define MAX_HEIGHT 720
	int noise_array[MAX_WIDTH * MAX_HEIGHT];

    for (int i = 0; i < Ximage * Yimage; i++) {
        float noise1 = 10000 * sin(2 * M_PI * FZ1 * i / FS + 0.53 * M_PI);
        noise_array[i] = (int)noise1;
    }

    // 分配缓冲区以提高写入效率
    const int buffer_size = 4096; // 可根据平台调整
    char *write_buffer = (char *)malloc(buffer_size);
    if (!write_buffer) {
        printf("Failed to allocate memory for write buffer!\r\n");
        free(noise_array);
        f_close(fil);
        return;
    }

    int buffer_offset = 0;

    // 按行处理像素数据
    for (y = 0; y < Yimage; y++) {
        for (x = 0; x < Ximage; x++) {
            int index = y * Ximage + x;
            int noise = noise_array[index];
            int RN = (int)(data_buf[x * 3 + iPixelAddr + 0] + noise);
            int GN = (int)(data_buf[x * 3 + iPixelAddr + 1] + noise);
            int BN = (int)(data_buf[x * 3 + iPixelAddr + 2] + noise);

            // 将结果写入缓冲区
            buffer_offset += snprintf(write_buffer + buffer_offset, buffer_size - buffer_offset, "%d\n%d\n%d\n", RN, GN, BN);

            // 如果缓冲区满，则写入文件
            if (buffer_offset >= buffer_size - 50) { // 留余量避免越界
                f_write(fil, write_buffer, buffer_offset, &br);
                buffer_offset = 0; // 清空缓冲区
            }
        }
        iPixelAddr -= stride; // 更新像素地址
    }

    // 写入剩余缓冲区数据
    if (buffer_offset > 0) {
        f_write(fil, write_buffer, buffer_offset, &br);
    }

    // 释放资源
    free(write_buffer);
    free(noise_array);
    f_close(fil);
}
