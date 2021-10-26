#include "helper.h"

int first = 0;

int mmap_mem()
{
    mem_fd = open("/dev/mem", O_RDWR | O_SYNC | O_CLOEXEC);

    if (mem_fd < 0)
    {
        printf("failed open /dev/mem, try sudo\n");
        return 1;
    }

    smi_obj.mem_buf.fd = open("/dev/vcio", 0);
    if (smi_obj.mem_buf.fd < 0)
    {
        printf("Failed to open dev/vcio\n");
        return 1;
    }

    smi_obj.gpio_reg = mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, GPIO_BASE);
    smi_obj.smi_reg = mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, SMI_BASE);
    smi_obj.dma_reg = mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, DMA_BASE);
    smi_obj.clk_reg = mmap(0, PAGE_SIZE, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, CLK_BASE);

    // printf("GPIO: %p -> %p\n", GPIO_BASE, smi_obj.gpio_reg);
    // printf("SMI: %p -> %p\n", SMI_BASE, smi_obj.smi_reg);
    // printf("DMA: %p -> %p\n", DMA_BASE, smi_obj.dma_reg);
    // printf("CLK: %p -> %p\n", CLK_BASE, smi_obj.clk_reg);

    return 0;
}

#define PRINT_REG(msg, reg) \
    printf(msg);            \
    printf("%p  ->  %lu  ->  %#lx\n", reg, *reg, *reg)

void setup_smi()
{
    int setup = 5, width = 1, ns = 15, strobe = 5, hold = 5;

    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_CS) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_L) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_A) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_DSR0) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_DSW0) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_DCS) = 0;
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_DCA) = 0;

    smi_obj.smi_fields.cs = (smi_cs_f *)(smi_obj.smi_reg + SMI_CS);
    smi_obj.smi_fields.dsr = (smi_dsrw *)(smi_obj.smi_reg + SMI_DSR0);
    smi_obj.smi_fields.dsw = (smi_dsrw *)(smi_obj.smi_reg + SMI_DSW0);
    smi_obj.smi_fields.dmc = (smi_dmc *)(smi_obj.smi_reg + SMI_DMC);

    if (*(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_DIV) != ns << 12)
    {
        *(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_CTL) = CLK_PASSWD | (1 << 5);
        while (*(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_CTL) & (1 << 7))
            ;
        *(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_DIV) = CLK_PASSWD | (ns << 12);
        *(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_CTL) = CLK_PASSWD | 6 | (1 << 4);
        while (!(*(volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_CTL) & (1 << 7)))
            ;
    }
    if (smi_obj.smi_fields.cs->seterr)
        smi_obj.smi_fields.cs->seterr = 1;

    smi_obj.smi_fields.dsr->setup = smi_obj.smi_fields.dsw->setup = setup;
    smi_obj.smi_fields.dsr->strobe = smi_obj.smi_fields.dsw->strobe = strobe;
    smi_obj.smi_fields.dsr->hold = smi_obj.smi_fields.dsw->hold = hold;
    smi_obj.smi_fields.dsr->width = smi_obj.smi_fields.dsw->width = width;

    smi_obj.smi_fields.dmc->panicr = smi_obj.smi_fields.dmc->panicw = 8;
    smi_obj.smi_fields.dmc->reqr = smi_obj.smi_fields.dmc->reqw = REQUEST_THRESH;

    // PRINT_REG("CS: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_CS));
    // PRINT_REG("L: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_L));
    // PRINT_REG("A: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_A));

    // PRINT_REG("DCS: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_DCS));
    // PRINT_REG("DCA: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_DCA));

    // PRINT_REG("CLK_SMI_DIV: ", (volatile uint32_t *)(smi_obj.clk_reg + CLK_SMI_DIV));

    // PRINT_REG("DSR: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_DSR0));
    // PRINT_REG("DSW: ", (volatile uint32_t *)(smi_obj.smi_reg + SMI_DSW0));
}

void setup_gpio()
{

    for (int i = 8; i < 24; i++) // SMI gpios
    {
        uint32_t *reg = (uint32_t *)(GPIO_MODE0 + (uint32_t)(smi_obj.gpio_reg)) + i / 10, shift = (i % 10) * 3;
        *reg = (*reg & ~(7 << shift)) | (GPIO_ALT1 << shift);
        //        PRINT_REG("REG PIN: ", reg);
    }

    for (int i = 0; i < 8; i++) // LEDs gpios
    {
        uint32_t *reg = (uint32_t *)(GPIO_MODE0 + (uint32_t)(smi_obj.gpio_reg)) + i / 10, shift = (i % 10) * 3;
        *reg = (*reg & ~(7 << shift)) | (GPIO_OUT << shift);
    }

    // while (1)
    // {
    //     *((unsigned *)(GPIO_SET_OFFSET + smi_obj.gpio_reg)) ^= 0b11111111;
    //     usleep(1000000);
    // }
}

int setup_dma()
{
    if (fps_count == 0)
    {
        printf("Memory prikols\n");
        smi_obj.mem_buf.size = DATA_SIZE;

        VC_MSG msg = {.tag = 0x3000c, .blen = 12, .dlen = 12, .uints = {smi_obj.mem_buf.size, PAGE_SIZE, DMA_FLAGS}};
        smi_obj.mem_buf.h = msg_box(&msg);
        VC_MSG msgl = {.tag = 0x3000d, .blen = 4, .dlen = 4, .uints = {smi_obj.mem_buf.h}};
        smi_obj.mem_buf.bus = (void *)msg_box(&msgl);

        munmap(smi_obj.mem_buf.virt, smi_obj.mem_buf.size);

        smi_obj.mem_buf.virt = mmap(0, smi_obj.mem_buf.size, PROT_WRITE | PROT_READ, MAP_SHARED, mem_fd, (uint32_t)(((void *)((uint32_t)(smi_obj.mem_buf.bus) & ~0xC0000000))));

        if (smi_obj.mem_buf.virt == MAP_FAILED)
        {
            printf("Failed to mmap mem_buf virt\n");
            return 1;
        }

        smi_obj.cbs = smi_obj.mem_buf.virt;
        smi_obj.data_buf = (uint16_t *)(smi_obj.cbs + 1);
    }

    smi_obj.smi_fields.dmc->dmaen = 1;

    // PRINT_REG("Before edit: ", smi_obj.smi_fields.cs);
    smi_obj.smi_fields.cs->enable = 1;
    smi_obj.smi_fields.cs->clear = 1;
    smi_obj.smi_fields.cs->pxldat = 1;

    // PRINT_REG("bbbbefsafdoas edit: ", smi_obj.smi_fields.cs);
    *(volatile uint32_t *)(smi_obj.smi_reg + SMI_L) = smi_obj.mem_buf.size * sizeof(uint16_t);
    // printf("NSamp: %d, smi_l: %d, smi_l val: %lu\n", smi_obj.mem_buf.size, smi_obj.mem_buf.size * sizeof(uint16_t), (volatile uint32_t *)(smi_obj.smi_reg + SMI_L));
    // PRINT_REG("1: ", smi_obj.smi_fields.cs);

    smi_obj.smi_fields.cs->write = 1;
    // PRINT_REG("After edit: ", smi_obj.smi_fields.cs);

    *(volatile uint32_t *)(smi_obj.dma_reg + DMA_ENABLE) |= (1 << DMA_CHAN);
    // PRINT_REG("2: ", smi_obj.smi_fields.cs);
    *(volatile uint32_t *)(smi_obj.dma_reg + (DMA_CHAN)*0x100) = 1 << 31;
    // PRINT_REG("3: ", smi_obj.smi_fields.cs);
    smi_obj.cbs[0].ti = DMA_DEST_DREQ | (DMA_SMI_DREQ << 16) | DMA_CB_SRCE_INC | DMA_WAIT_RESP;
    // PRINT_REG("4: ", smi_obj.smi_fields.cs);
    smi_obj.cbs[0].tfr_len = smi_obj.mem_buf.size;
    // PRINT_REG("5: ", smi_obj.smi_fields.cs);
    // printf("Size: %d", smi_obj.cbs[0].tfr_len);
    smi_obj.cbs[0].srce_ad = ((uint32_t)smi_obj.data_buf - (uint32_t)smi_obj.mem_buf.virt + (uint32_t)smi_obj.mem_buf.bus);
    // PRINT_REG("6: ", smi_obj.smi_fields.cs);
    smi_obj.cbs[0].dest_ad = ((uint32_t)(SMI_BASE - PHYS_REG_BASE + BUS_REG_BASE) + (uint32_t)(0x0c));

    // PRINT_REG("After DMA init: ", smi_obj.smi_fields.cs);
    return 0;
}

void clear_mem()
{
    shm_unlink("VirtualCubeSHMemmory");
    free(smi_obj.tmp_buf);
}

clock_t prev_draw;

void draw()
{
    // PRINT_REG("before memset1: ", smi_obj.smi_fields.cs);
    // for (int i = 0; i < RGB_DATA_SIZE >> 1; i++)
    // {
    //     smi_obj.tmp_buf[i] = i & 1 ? 0b0101010101010101 : 0b1010101010101010;
    // }

    setup_smi();
    setup_dma();

    memcpy((smi_obj.data_buf + LEAD_ZEROS), smi_obj.tmp_buf, RGB_DATA_SIZE);
    // PRINT_REG("After memeset: ", smi_obj.smi_fields.cs);

    *(volatile uint32_t *)(smi_obj.dma_reg + DMA_CHAN * 0x100 + DMA_CONBLK_AD) = ((uint32_t)&smi_obj.cbs[0] - (uint32_t)smi_obj.mem_buf.virt + (uint32_t)smi_obj.mem_buf.bus);
    // PRINT_REG("1: ", smi_obj.smi_fields.cs);
    *(volatile uint32_t *)(smi_obj.dma_reg + DMA_CHAN * 0x100 + DMA_CS) = 2;
    // PRINT_REG("2: ", smi_obj.smi_fields.cs);
    *(volatile uint32_t *)(smi_obj.dma_reg + DMA_CHAN * 0x100 + DMA_DEBUG) = 7;
    // PRINT_REG("3: ", smi_obj.smi_fields.cs);
    *(volatile uint32_t *)(smi_obj.dma_reg + DMA_CHAN * 0x100 + DMA_CS) = 1;
    smi_obj.smi_fields.cs->start = 1;

    while ((*(volatile uint32_t *)(smi_obj.dma_reg + DMA_CHAN * 0x100 + DMA_CS)) & 1)
        usleep(10);

    //     printf("Delay: %.6f\n ms", ((double) (clock() - prev_draw)) / CLOCKS_PER_SEC / 1000.0f);
    //   prev_draw = clock();
    fps_count++;
}

uint32_t msg_box(VC_MSG *msgp)
{
    for (int i = msgp->dlen / 4; i <= msgp->blen / 4; i += 4)
        msgp->uints[i++] = 0;

    msgp->len = (msgp->blen + 6) * 4;
    msgp->req = 0;

    if (ioctl(smi_obj.mem_buf.fd, _IOWR(100, 0, void *), msgp) < 0)
    {
        printf("VC IOCTL failed\n");
        return 1;
    }
    else if ((msgp->req & 0x80000000) == 0)
    {
        printf("VC IOCTL error\n");
        return 1;
    }
    else if (msgp->req == 0x80000001)
    {
        printf("VC IOCTL partial error\n");
        return 1;
    }
    return msgp->uints[0];
}

void set_led(char *grb, int n)
{
    int start_byte = n * 96;
    for (int i = 0; i < 3; i++)
    {
        for (int j = 7; j >= 0; j--)
        {
            smi_obj.tmp_buf[start_byte + i * 32 + j * 4] = 0xffff;
            smi_obj.tmp_buf[start_byte + i * 32 + j * 4 + 1] = grb[i] & (1 << j) ? 0xffff : 0x0000;
            smi_obj.tmp_buf[start_byte + i * 32 + j * 4 + 2] = 0x0000;
            smi_obj.tmp_buf[start_byte + i * 32 + j * 4 + 3] = 0x0000;
        }
    }
}

int init_shm()
{
    int shm_id = shm_open("VirtualCubeSHMemmory", O_RDWR | O_CREAT, 0);
    if (shm_id < 0)
    {
        printf("\033[38;2;255;0;0mERROR\x1b[0m Unable to create shared memmory object\n");
        return 1;
    }

    if (ftruncate(shm_id, sizeof(buf_struct)) < 0)
    {
        printf("\033[38;2;255;0;0mERROR\x1b[0m Unable perform ftruncate\n");
        return 1;
    }

    shm_buf = (buf_struct *)mmap(NULL, sizeof(buf_struct), PROT_READ | PROT_WRITE, MAP_SHARED, shm_id, 0);

    if (shm_buf == MAP_FAILED)
    {
        printf("\033[38;2;255;0;0mERROR\x1b[0m Unable to mmap to shared memmory buffer\n");
        return 1;
    }

    pthread_mutexattr_t attr;

    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    if (pthread_mutex_init(&(shm_buf->new_frame_lock), NULL))
        printf("\033[38;2;255;0;0mERROR\x1b[0m Was unable to initialize data_transfer_lock mutex\n");

    if (pthread_mutex_init(&(shm_buf->new_frame_lock), NULL))
        printf("\033[38;2;255;0;0mERROR\x1b[0m Was unable to initialize data_transfer_lock mutex\n");

    if (pthread_mutex_unlock(&(shm_buf->shm_buf_lock)))
        printf("Error during unclocking shm lock\n");

    if (pthread_mutex_lock(&(shm_buf->new_frame_lock)))
        printf("Error during locking new frame lock\n");

    printf("\033[38;2;0;255;0mOK\x1b[0m Successfully initialized shared memmory object\n");

    memset(shm_buf->buf, 0, SHM_BUF_SIZE);
    return 0;
}

// void *writer_thread(void *vargp)
void writer_thread()
{
    prev_draw = clock();
    printf("starting thread\n");
    while (1)
    {
        printf("Locking frame lock\n");
        if (pthread_mutex_lock(&(shm_buf->new_frame_lock)))
            printf("Error during locking new frame lock\n");

        printf("New Frame arrived!, Loking shm bufe lock\n");

        pthread_mutex_lock(&(shm_buf->shm_buf_lock)); // Wait for drawer programm to copy data to shared memory
        memset(smi_obj.tmp_buf, 0, RGB_DATA_SIZE);

        for (int i = 0; i < (RGB_DATA_SIZE >> 3); i++)
            smi_obj.tmp_buf[i << 2] = 0xffff;

        int tmpx;
        int tmpz;

        for (int y = 15; y >= 0; y--)
        {
            for (int x = 0; x < 16; x++)
            {
                int smi_sb = (((15 - y) << 4) + x) * 96 + 1;
                for (int z = 0; z < 16; z++)
                {
                    if (y & 1)
                        if (z & 1)
                        {
                            tmpx = x;
                            tmpz = z - 1;
                        }
                        else
                        {
                            tmpx = 15 - x;
                            tmpz = z + 1;
                        }
                    else
                    {
                        tmpx = z & 1 ? (15 - x) : (x);
                        tmpz = z;
                    }

                    int shm_sb = ((16 * tmpz + y) * 16 + tmpx) * 3;

                    for (int j = 0; j < 3; j++)
                    {
                        for (int i = 0; i < 8; i++)
                        {
                            if (shm_buf->buf[shm_sb + j] & (1 << (7 - i)))
                                smi_obj.tmp_buf[smi_sb + ((8 * j + i) << 2)] |= 1 << z;
                        }
                    }
                }
            }
        }
        pthread_mutex_unlock(&(shm_buf->shm_buf_lock));
        printf("Unlocking shm buf lock\n");

        draw();
    }
}

int init_cube()
{
    printf("Res: %d\n", _POSIX_THREAD_PROCESS_SHARED);
    fps_count = 0;

    srand(time(NULL));

    if (init_shm())
        return 1;

    if (mmap_mem())
        return 1;
    setup_gpio();

    // if (setup_dma())
    //     return 1;

    smi_obj.tmp_buf = (uint16_t *)malloc(RGB_DATA_SIZE);
    // PRINT_REG("After malloc: ", smi_obj.smi_fields.cs);

    //    pthread_create(&writer_thread_id, NULL, writer_thread, NULL);
    //    pthread_detach(writer_thread_id);
    writer_thread();

    return 0;
}
