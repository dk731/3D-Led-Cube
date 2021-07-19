#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include <signal.h>

#include <unistd.h>
#include <time.h>
#include <string.h>
#include <pthread.h>
#include <fcntl.h>

#include <sys/shm.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#define PAGE_SIZE 0x1000

#define PHYS_REG_BASE 0x3F000000
#define BUS_REG_BASE 0x7E000000

#define GPIO_BASE (PHYS_REG_BASE + 0x200000)
#define DMA_BASE (PHYS_REG_BASE + 0x007000)
#define CLK_BASE (PHYS_REG_BASE + 0x101000)
#define SMI_BASE (PHYS_REG_BASE + 0x600000)
#define SMI_CS 0x00
#define SMI_L 0x04
#define SMI_A 0x08
#define SMI_D 0x0c
#define SMI_DSR0 0x10
#define SMI_DSW0 0x14
#define SMI_DSR1 0x18
#define SMI_DSW1 0x1c
#define SMI_DSR2 0x20
#define SMI_DSW2 0x24
#define SMI_DSR3 0x28
#define SMI_DSW3 0x2c
#define SMI_DMC 0x30
#define SMI_DCS 0x34
#define SMI_DCA 0x38

#define SMI_DCD 0x3c
#define SMI_FD 0x40

#define CLK_SMI_CTL 0xb0
#define CLK_SMI_DIV 0xb4
#define CLK_PASSWD 0x5a000000

#define GPIO_SET0 0x1c
#define GPIO_CLR0 0x28

#define GPIO_OUT 1
#define GPIO_ALT1 5
#define GPIO_MODE0 0x00
#define GPIO_SET_OFFSET 7

#define REQUEST_THRESH 2

#define DMA_CHAN 10
#define DMA_ENABLE 0xff0
#define DMA_CS 0x00

#define DMA_FLAGS 0b00010100

#define DMA_WAIT_RESP (1 << 3)
#define DMA_CB_DEST_INC (1 << 4)
#define DMA_DEST_DREQ (1 << 6)
#define DMA_CB_SRCE_INC (1 << 8)
#define DMA_SRCE_DREQ (1 << 10)
#define DMA_PRIORITY(n) ((n) << 16)

#define DMA_CS 0x00
#define DMA_CONBLK_AD 0x04
#define DMA_TI 0x08
#define DMA_SRCE_AD 0x0c
#define DMA_DEST_AD 0x10
#define DMA_TXFR_LEN 0x14
#define DMA_STRIDE 0x18
#define DMA_NEXTCONBK 0x1c
#define DMA_DEBUG 0x20
#define DMA_ENABLE 0xff0

#define DMA_SMI_DREQ 4
#define DATA_SIZE (PAGE_SIZE * 13)
#define LEAD_ZEROS 350
#define RGB_DATA_SIZE (PAGE_SIZE * 12)
#define SHM_BUF_SIZE 12288

typedef struct smi_cs_f
{
    volatile uint32_t enable : 1, done : 1, active : 1, start : 1, clear : 1,
        write : 1, _x1 : 2, teen : 1, intd : 1, intt : 1,
        intr : 1, pvmode : 1, seterr : 1, pxldat : 1, edreq : 1,
        _x2 : 8, _x3 : 1, aferr : 1, txw : 1, rxr : 1,
        txd : 1, rxd : 1, txe : 1, rxf : 1
} smi_cs_f;

typedef struct smi_dsrw
{
    volatile uint32_t strobe : 7, dreq : 1, pace : 7, paceall : 1, hold : 6,
        fsetup : 1, mode68 : 1, setup : 6, width : 2
} smi_dsrw;

typedef struct smi_dmc
{
    volatile uint32_t reqw : 6, reqr : 6, panicw : 6, panicr : 6, dmap : 1,
        _x1 : 3, dmaen : 1
} smi_dmc;

typedef struct memory_buf
{
    int fd, h, size;
    void *bus, *virt, *phys;
} memory_buf;

typedef struct VC_MSG
{
    uint32_t len,
        req,
        tag,
        blen,
        dlen;
    uint32_t uints[27];
} VC_MSG __attribute__((aligned(16)));

typedef struct DMA_CB
{
    uint32_t ti,
        srce_ad,
        dest_ad,
        tfr_len,
        stride,
        next_cb,
        debug,
        unused;
} DMA_CB __attribute__((aligned(32)));

typedef struct smi_fields
{
    smi_cs_f *cs;
    smi_dsrw *dsr;
    smi_dsrw *dsw;
    smi_dmc *dmc;
} smi_fields;

typedef struct smi_obj_struct
{
    volatile void *gpio_reg;
    volatile void *smi_reg;
    volatile void *dma_reg;
    volatile void *clk_reg;
    volatile void *mem_p;
    memory_buf mem_buf;
    uint16_t *data_buf;
    DMA_CB *cbs;
    smi_fields smi_fields;
    uint16_t *tmp_buf;
} smi_obj_struct;

typedef struct shm_flags
{
    uint8_t frame_shown : 1, lock : 1, other : 6;
} shm_flags;

typedef struct buf_struct
{
    uint8_t buf[SHM_BUF_SIZE];
    shm_flags flags;
} buf_struct;

smi_obj_struct smi_obj;

buf_struct *shm_buf;

pthread_t writer_thread_id;

int init_cube();

int init_shm();

int mmap_mem();
void setup_smi();
void setup_gpio();
int setup_dma();

void *writer_thread(void *vargp);

void unmap_periph_mem(void *mp);
void clear_mem();

void draw();

uint32_t msg_box(VC_MSG *msgp);

void set_led(char *grb, int n);