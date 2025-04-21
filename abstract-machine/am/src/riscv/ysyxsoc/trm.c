#include <am.h>
#include <klib-macros.h>

extern char _heap_start;
extern char _heap_end;
int main(const char *args);


#define UART_REG_RBR 0x00  // 接收缓冲寄存器
#define UART_REG_THR 0x00  // 发送保持寄存器
#define UART_REG_DLL 0x00  // 除数锁存器低位 (DLAB=1)
#define UART_REG_DLM 0x01  // 除数锁存器高位 (DLAB=1)
#define UART_REG_LCR 0x03  // 线路控制寄存器
#define UART_REG_LSR 0x05  // 线路状态寄存器
#define UART_LSR_THRE 0x20 // 发送保持寄存器空标志位




// 链接脚本中定义的符号
extern char _data_lma;          // 数据段在MROM中的位置
extern char _data_vma_start;    // 数据段在SRAM中的起始位置
extern char _data_vma_end;      // 数据段在SRAM中的结束位置

extern char _text_lma;          // 代码段在flash中的位置
extern char _text_vma_start;    // 代码段在SRAM中的起始位置
extern char _text_vma_end;      // 代码段在SRAM中的结束位置

extern char _execute_main;

extern char _bootloader_lma;
extern char _bootloader_vma_start;
extern char _bootloader_vma_end;

extern char _rodata_lma;        // 只读数据段在flash中的位置
extern char _rodata_vma_start;  // 只读数据段在SRAM中的起始位置
extern char _rodata_vma_end;    // 只读数据段在SRAM中的结束位置

  extern char _data_extra_lma[];    // .data.extra 段的 LMA (通过 LOADADDR 获取)
  extern char _data_extra_vma_start[];      // .data.extra 段在 RAM 中的起始位置
  extern char _data_extra_vma_end[];   // .data.extra 段在 RAM 中的结束位置



extern char _bss_start;         // BSS段起始位置

//extern char _bss_end;           // BSS段结束位置


void __attribute__((section(".fsbl"))) fsbl(void) {
    // 1. 复制bootloader从flash到sram
    uint32_t *src = (uint32_t*)&_bootloader_lma;
    uint32_t *dst = (uint32_t*)&_bootloader_vma_start;
    size_t words = (&_bootloader_vma_end - &_bootloader_vma_start) / 4;
    
    for (size_t i = 0; i < words; i++) {
        dst[i] = src[i];  // 32位对齐访问
    }

    // 2. 直接跳转到bootloader，不返回
    asm volatile (
        "la t0, _bootloader_vma_start\n\t"
        "jalr zero, t0, 0"
        : : : "t0"
    );


}

void __attribute__((section(".bootloader"), used)) bootloader(void) {

  uint32_t *src = (uint32_t*)&_data_lma;// 按字复制保证对齐
  uint32_t *dst = (uint32_t*)&_data_vma_start;
  size_t words = (&_data_vma_end - &_data_vma_start) / 4;
  
  for (size_t i=0; i<words; i++) {
    dst[i] = src[i]; // 32位对齐访问
  }

   src = (uint32_t*)&_text_lma;
  dst = (uint32_t*)&_text_vma_start;
  words = (&_text_vma_end - &_text_vma_start) / 4;
  
  for (size_t i = 0; i < words; i++) {
    dst[i] = src[i];  // 32位对齐访问
  }
 

  // 复制.rodata段
  src = (uint32_t*)&_rodata_lma;
  dst = (uint32_t*)&_rodata_vma_start;
  words = (&_rodata_vma_end - &_rodata_vma_start) / 4;
  
  for (size_t i = 0; i < words; i++) {
    dst[i] = src[i];  // 32位对齐访问
  }



//extra data
src = (uint32_t*)_data_extra_lma;
dst = (uint32_t*)_data_extra_vma_start;   //在ysyxsoclinker2中data extra lma前面是一道杠，其余extra的是两道
words = (_data_extra_vma_end - _data_extra_vma_start) / 4;

// 检查是否需要复制
/*if (src == dst) {
  // 源地址和目标地址相同，不需要复制
  putch('S'); putch('k'); putch('i'); putch('p'); putch(':');
  putch('S'); putch('a'); putch('m'); putch('e');
  putch('\n');
} else if (words > 0 && words < 1000000) {  // 添加合理性检查
  for (size_t i = 0; i < words; i++) {
    dst[i] = src[i];  // 32位对齐访问
  }
} else {
  // 处理异常情况
  putch('E'); putch('r'); putch('r'); putch(':');
  if (words == 0) {
    putch('Z'); putch('e'); putch('r'); putch('o');
  } else {
    putch('O'); putch('v'); putch('e'); putch('r');
  }
  putch('\n');
}*/






asm volatile (
    "la sp, _stack_pointer\n\t"  // 直接加载栈指针
    "la t0, _execute_main\n\t"    // 加载目标地址
    "jalr zero, t0, 0"            // 跳转
    : : : "t0"                    // 只声明 t0 被修改
);
}


/*void bootloader() {
  uint32_t *src = (uint32_t*)&_data_lma;// 按字复制保证对齐
  uint32_t *dst = (uint32_t*)&_data_vma_start;
  size_t words = (&_data_vma_end - &_data_vma_start) / 4;
  
  for (size_t i=0; i<words; i++) {
    dst[i] = src[i]; // 32位对齐访问
  }


}*/

//extern char _pmem_start;
//#define PMEM_SIZE (16 * 1024 * 1024)  // ysyxSoC的SRAM大小，16MB
//#define PMEM_END  ((uintptr_t)&_pmem_start + PMEM_SIZE)

Area heap = RANGE(&_heap_start, &_heap_end);
#ifndef MAINARGS
#define MAINARGS ""
#endif
static const char mainargs[] = MAINARGS;
static inline void outb(uintptr_t addr, uint8_t data) { *(volatile uint8_t *)addr = data; }
static inline uint8_t inb(uintptr_t addr) { 
    return *(volatile uint8_t *)addr; 
}
#define UART_BASE     0x10000000  // ysyxSoC的UART设备基地址
#define UART_TX       0x00        // 发送寄存器偏移

// 初始化UART
void uart_init() {
    // 1. 设置DLAB位以访问除数锁存器
    //outb(UART_BASE + UART_REG_LCR, 0x10); // LCR[7] = 1
  // outb(UART_BASE + UART_REG_LCR, 0x20); // LCR[7] = 1
    outb(UART_BASE + UART_REG_LCR, 0x80); // LCR[7] = 1
    // 2. 设置波特率除数（示例值1，最高波特率）
    outb(UART_BASE + UART_REG_DLL, 0x01); // 除数低位
    outb(UART_BASE + UART_REG_DLM, 0x00); // 除数高位
    
    
    // 3. 配置数据格式：8位数据，1停止位，无校验，清除DLAB
    outb(UART_BASE + UART_REG_LCR, 0x03); // LCR = 0000 0011
    
    // 4. 可选：启用并重置FIFO
    // outb(UART_BASE + 2, 0x07); // FCR寄存器
}




void putch(char ch) {
    
    while ((inb(UART_BASE + UART_REG_LSR) & UART_LSR_THRE) == 0) {
        // 等待THRE位置1
    }
   outb(UART_BASE + UART_TX, ch);
}

void halt(int code) {
  asm volatile ("mv a0, %0" : : "r"(code));  // 将返回码移到a0寄存器
  asm volatile("ebreak");  // 触发ebreak指令
  
  while (1);
}

static void put_dec(uint32_t num) {
    char buf[10];
    int i = 0;
    do {
        buf[i++] = '0' + num % 10;
        num /= 10;
    } while (num > 0);
    
    while (--i >= 0) {
        putch(buf[i]);
    }
}
void execute_main(void) __attribute__((used));
void execute_main() {
    uart_init();
  //  bootloader();
    //uart_init();
      // 添加CSR读取
  
  uint32_t vendor_id, arch_id;
  __asm__ __volatile__("csrr %0, 0xF11" : "=r"(vendor_id));  // mvendorid
  __asm__ __volatile__("csrr %0, 0xF12" : "=r"(arch_id));    // marchid

  // 从mvendorid解析"ysyx"
  putch((vendor_id >> 24) & 0xFF); // y
  putch((vendor_id >> 16) & 0xFF); // s
  putch((vendor_id >> 8)  & 0xFF); // y
  putch( vendor_id        & 0xFF); // x
  
  // 输出下划线
  putch('_');
  
  // 从marchid解析学号（直接输出十进制值）
  put_dec(arch_id);  // 这里假设arch_id寄存器存储的是24090014的十进制值
  
  // 换行
  putch('\n');
   
 volatile int ret = main(mainargs);
  halt(ret);
    // 切回text段
   
}


void _trm_init() {
  
  /*uart_init();
  //  bootloader();
    //uart_init();
      // 添加CSR读取
  
  uint32_t vendor_id, arch_id;
  __asm__ __volatile__("csrr %0, 0xF11" : "=r"(vendor_id));  // mvendorid
  __asm__ __volatile__("csrr %0, 0xF12" : "=r"(arch_id));    // marchid

  // 从mvendorid解析"ysyx"
  putch((vendor_id >> 24) & 0xFF); // y
  putch((vendor_id >> 16) & 0xFF); // s
  putch((vendor_id >> 8)  & 0xFF); // y
  putch( vendor_id        & 0xFF); // x
  
  // 输出下划线
  putch('_');
  
  // 从marchid解析学号（直接输出十进制值）
  put_dec(arch_id);  // 这里假设arch_id寄存器存储的是24090014的十进制值
  
  // 换行
  putch('\n');*/
  fsbl();

  

  bootloader();

   execute_main();


 
}