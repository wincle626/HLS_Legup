#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <assert.h>

#include "riffa.h"
#include "riffa_wrapper.h"

typedef unsigned char  uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int   uint32_t;
typedef unsigned long long uint64_t;
#define CONTROL_ADDRESS (-1)

static fpga_t * fpga = NULL;
static pthread_mutex_t fpga_lock = PTHREAD_MUTEX_INITIALIZER;

int dma_is_idle(ssize_t f)
{
    assert(0);
}

void dma_update(ssize_t f)
{
    assert(0);
}

// Function definitions
void pci_init()
{
    int isLockedByMe = pthread_mutex_trylock(&fpga_lock);
	fpga_info_list info;
    if (fpga_list(&info) != 0) {
        printf("Error populating fpga_info_list\n");
        exit(-1);
    }
	printf("Number of devices: %d\n", info.num_fpgas);

    if (info.num_fpgas <= 0) {
        fprintf(stderr, "Error: no FPGA found.");
        exit(-1);
    }
	printf("Info: using FPGA with id:%d\n", info.id[0]);

	int id = info.id[0];
	fpga = fpga_open(id);
    if (fpga == NULL) {
        fprintf(stderr, "ERROR: can't open fpga with id:%d\n", id);
        exit(-1);
    }

	// fpga_reset(fpga);
    if (isLockedByMe == 0) pthread_mutex_unlock(&fpga_lock);
    return;

    /*
    static pthread_mutex_t fd_lock = PTHREAD_MUTEX_INITIALIZER;
    pthread_mutex_lock(&fd_lock);
    // Open the device file, if needed
    if (!fd) {
        fd = open("/dev/de4", O_RDWR);
        if (fd == -1) {
            fd = 0;
            pthread_mutex_unlock(&fd_lock);
            exit(1);
        }
    }
    pthread_mutex_unlock(&fd_lock);
    */
}

void pci_close()
{
    pthread_mutex_lock(&fpga_lock);
    fpga_close(fpga);
    fpga = NULL;
    pthread_mutex_unlock(&fpga_lock);
    return;
}

/* FPGA -> PC SEQUENCE
 *      1. send header from PC to FPGA over fpga_send to CONTROL_ADDRESS
 *      2. receive data from FPGA to PC over fpga_recv
 */

// header format for FPGA to PC
typedef struct {
    int type:2; // always be 1
    int len:30; // length in words
    int addr;   // target address
    int pad1;
    int pad2;
} FPGA2PCHeader;

int pci_read_direct_jenny(void *recvBuffer, size_t len, int offset)
{
    assert(len % 4 == 0 && len >= 0 && offset != CONTROL_ADDRESS);
    if (!fpga) pci_init();

    // assume only using channel 0
    const int chnl = 0;

    int numWords = len >> 2;
    FPGA2PCHeader header = {1, numWords, offset};
    assert(sizeof(header) == 16);
    /*
    int fpga_recv(fpga_t * fpga, int chnl, void * data, int len, long long timeout);
    */

    int sent = 0;
    // send header
	sent = fpga_send(fpga, chnl, &header, sizeof(header) / sizeof(int), CONTROL_ADDRESS, 1, 25000);
    assert(sent > 0);

    // receive data
	int recvd = fpga_recv(fpga, chnl, recvBuffer, numWords, 25000);
    assert(recvd > 0);
    return recvd * sizeof(int);
}

int pci_read_direct(void *recvBuffer, size_t len, int offset)
{
    return pcie_read(recvBuffer, len, offset);

    assert(len % 4 == 0 && len >= 0 && offset != CONTROL_ADDRESS);
    if (!fpga) pci_init();

    // assume only using channel 0
    const int chnl = 0;

    int numWords = len >> 2;
    FPGA2PCHeader header = {1, numWords, offset};
    assert(sizeof(header) == 16);
    /*
    int fpga_recv(fpga_t * fpga, int chnl, void * data, int len, long long timeout);
    */

    int sent = 0;
    // send header
	sent = fpga_send(fpga, chnl, &header, sizeof(header) / sizeof(int), CONTROL_ADDRESS, 1, 25000);
    assert(sent > 0);

    // receive data
	int recvd = fpga_recv(fpga, chnl, recvBuffer, numWords, 25000);
    assert(recvd > 0);
    return recvd * sizeof(int);
}

/* PC -> FPGA SEQUENCE
 *      1. send header from PC to FPGA over fpga_send to CONTROL_ADDRESS
 *      2. send data from PC to FPGA over fpga_send to target address (offset)
 */

// header format for PC to FPGA
typedef struct {
    int type:2; // always be 0
    int len:30; // length in words
    int pad1;
    int pad2;
} PC2FPGAHeader;

int pci_write_direct(void *sendBuffer, size_t len, int offset)
{
    return pcie_write(sendBuffer, len, offset);

    assert(len % 4 == 0 && len >= 0 && offset != CONTROL_ADDRESS);
    if (!fpga) pci_init();

    // assume only using channel 0
    const int chnl = 0;

    int numWords = len >> 2;
    // PC2FPGAHeader header = {numWords, 0};
    //assert(sizeof(header) == 4);
    /*
    int fpga_send(fpga_t * fpga, int chnl, void * data, int len, int destoff, 
        int last, long long timeout);
    */
    int sent = 0;

    // send header
	//sent = fpga_send(fpga, chnl, &header, sizeof(header) / sizeof(int), CONTROL_ADDRESS, 1, 25000);
    //assert(sent > 0);

    // send data
	sent = fpga_send(fpga, chnl, sendBuffer, numWords, offset, 1, 25000);
    assert(sent > 0);

    return sent * sizeof(int);

}

void pci_read_dma(void *buf, size_t len, int offset)
{
    pcie_read(buf, len, offset);
    return;

    /*
    // Read the data back
    struct aclpci_cmd read_cmd =
        { DMA_BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf };
    if (write(fd, &read_cmd, len)) {
        exit(1);
    }
    */
}

void pci_write_dma(void *buf, size_t len, int offset)
{
    pcie_write(buf, len, offset);
    return;

    /*
    if (!fd) {
        pci_init();
    }
    // Write data via DMA
    struct aclpci_cmd read_cmd =
        { DMA_BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf };
    if (write(fd, &read_cmd, len)) {
        exit(1);
    }
    */
}

void pci_dma_go()
{
    return;

    // Release DMA
    /*
    while (!dma_is_idle(fd)) {
        dma_update(fd);
    }
    */
}

int pcie_send_command(uint8_t * cmd)
{
    // printf("cmd: ");
    // int i;
    // for (i = 15; i >= 0; i--)
    //     printf("%02x ", cmd[i]);
    // printf("\n");

    assert(((uint64_t)cmd & PCIE_ALIGN_MASK) == 0);
    const int chnl = 0;
    return fpga_send(fpga, chnl, cmd, 4, CONTROL_ADDRESS, 1, 25000);
}

typedef struct {
    long long data;
    uint32_t offset;
    uint32_t control;
} WriteHeader;

int pcie_write_16_aligned(void *buf, size_t len, int offset)
{
    assert((offset & 0xf) == 0);
    assert((len & 0xf) == 0);
    assert(((uint64_t)buf & PCIE_ALIGN_MASK) == 0);

    int len_in_words = len >> 2;
    const int chnl = 0;

	int sent = fpga_send(fpga, chnl, buf, len_in_words, offset, 1, 25000);

    return sent * sizeof(int);
}

int pcie_write_8_aligned(void *buf, int offset)
{
    // have to be 8 bytes aligned
    assert((offset & 0x7) == 0);

    uint32_t shift_count = (offset & 0x8);
    uint32_t byteenable  = 0xff << shift_count;

    uint64_t data = *(uint64_t *) buf;

    WriteHeader wh = {
        data,  // data
        (offset & 0xfffffff0), // offset (16 align)
        0x10000000 | byteenable
    };

    int sent = pcie_send_command((uint8_t *)&wh);

    if (sent == 4)
        return 8;

    // in error
    return 0;
}

int pcie_write_4_aligned(void *buf, int offset)
{
    // have to be 8 bytes aligned
    assert((offset & 0x3) == 0);

    uint32_t shift_count = (offset & 0xc);
    uint32_t byteenable  = 0xf << shift_count;

    uint64_t data = *(uint32_t *) buf;
    data = (data << 32) | data;

    WriteHeader wh = {
        data, // data
        (offset & 0xfffffff0), // offset (16 align)
        0x10000000 | byteenable
    };

    int sent = pcie_send_command((uint8_t *)&wh);

    if (sent == 4)
        return 4;

    // in error
    return 0;
}

int pcie_write_2_aligned(void *buf, int offset)
{
    // have to be 8 bytes aligned
    assert((offset & 0x1) == 0);

    uint32_t shift_count = (offset & 0xe);
    uint32_t byteenable  = 0x3 << shift_count;

    uint64_t data = *(uint16_t *) buf;
    data = (data << 48) | (data << 32) | (data << 16) | data;

    WriteHeader wh = {
        data, // data
        (offset & 0xfffffff0), // offset (16 align)
        0x10000000 | byteenable
    };

    int sent = pcie_send_command((uint8_t *)&wh);

    if (sent == 4)
        return 2;

    // in error
    return 0;
}

int pcie_write_1_aligned(void *buf, int offset)
{
    // have to be 8 bytes aligned

    uint32_t shift_count = (offset & 0xf);
    uint32_t byteenable  = 0x1 << shift_count;

    uint64_t data = *(uint8_t *) buf;
    data = (data << 56) |
           (data << 48) |
           (data << 40) |
           (data << 32) |
           (data << 24) |
           (data << 16) |
           (data << 8)  |
           data;

    WriteHeader wh = {
        data, // data
        (offset & 0xfffffff0), // offset (16 align)
        0x10000000 | byteenable
    };

    int sent = pcie_send_command((uint8_t *)&wh);

    if (sent == 4)
        return 1;

    // in error
    return 0;
}


int pcie_write_max_aligned(void *buf, int max_count, int offset)
{
    if (max_count >= 16 && (offset & 0xf) == 0 && ((uint64_t)buf & PCIE_ALIGN_MASK) == 0) {
        return pcie_write_16_aligned(buf, max_count & 0xfffffff0, offset);
    } else if (max_count >= 8 && (offset & 0x7) == 0) {
        return pcie_write_8_aligned(buf, offset);
    } else if (max_count >= 4 && (offset & 0x3) == 0) {
        return pcie_write_4_aligned(buf, offset);
    } else if (max_count >= 2 && (offset & 0x1) == 0) {
        return pcie_write_2_aligned(buf, offset);
    } else if (max_count >= 1) {
        return pcie_write_1_aligned(buf, offset);
    }
    // should not go here
    assert(0);
    return 0;
}

int pcie_write(uint8_t *buf, size_t len, int offset)
{
    pthread_mutex_lock(&fpga_lock);

    if (!fpga) pci_init();

    int sent = 0;
    int remain = len;

    while (remain > 0) {
        sent = pcie_write_max_aligned(buf, remain, offset);

        assert(sent > 0);
        buf += sent;
        remain -= sent;
        offset += sent;
    }

    pthread_mutex_unlock(&fpga_lock);

    return len;
}

typedef struct {
    uint32_t offset;
    uint32_t length;
    uint32_t byteenable;
    uint32_t control;
} ReadHeader;

#define MIN_RECV_SIZE 4

int pcie_read_16_aligned(void *buf, size_t len, int offset)
{
    assert((len & 0xf) == 0);
    assert((offset & 0xf) == 0);
    assert(((uint64_t)buf & PCIE_ALIGN_MASK) == 0);

    int len_in_words = len >> 2;
    const int chnl = 0;

    ReadHeader rh = {
        offset,
        len_in_words,
        0x0000ffff,
        0x30000000
    };

    int sent = fpga_send(fpga, chnl, &rh, sizeof(rh) / sizeof(int), CONTROL_ADDRESS, 1, 25000);
    assert(sent == 4);

    int recv = fpga_recv(fpga, chnl, buf, len_in_words, 25000);
    return recv * sizeof(int);
}

int pcie_read_8_aligned(void *buf, int offset)
{
    assert((offset & 0x7) == 0);

    const int chnl = 0;
    uint32_t shift_count = (offset & 0xf);
    uint32_t byteenable  = 0xff << shift_count;

    ReadHeader rh = {
        offset & 0xfffffff0,
        MIN_RECV_SIZE,
        byteenable,
        0x30000000
    };

    int sent = pcie_send_command((uint8_t *)&rh);
    assert(sent == 4);

    uint64_t recv_buf[2];
    uint64_t * recvBuf = recv_buf;
    assert(((uint64_t)recvBuf & PCIE_ALIGN_MASK) == 0);
    int recv = fpga_recv(fpga, chnl, recvBuf, MIN_RECV_SIZE, 25000);
    assert(recv == 4);

    *((uint64_t*) buf) = recvBuf[shift_count >> 3];
    return 8;
}

int pcie_read_4_aligned(void *buf, int offset)
{
    assert((offset & 0x3) == 0);

    const int chnl = 0;
    uint32_t shift_count = (offset & 0xf);
    uint32_t byteenable  = 0xf << shift_count;

    ReadHeader rh = {
        offset & 0xfffffff0,
        MIN_RECV_SIZE,
        byteenable,
        0x30000000
    };

    int sent = pcie_send_command((uint8_t *)&rh);
    assert(sent == 4);

    uint64_t recv_buf[2];
    uint32_t * recvBuf = (uint32_t *) recv_buf;
    assert(((uint64_t)recvBuf & PCIE_ALIGN_MASK) == 0);
    int recv = fpga_recv(fpga, chnl, recvBuf, MIN_RECV_SIZE, 25000);
    assert(recv == 4);

    *((uint32_t*) buf) = recvBuf[shift_count >> 2];
    return 4;
}

int pcie_read_2_aligned(void *buf, int offset)
{
    assert((offset & 0x1) == 0);

    const int chnl = 0;
    uint32_t shift_count = (offset & 0xf);
    uint32_t byteenable  = 0x3 << shift_count;

    ReadHeader rh = {
        offset & 0xfffffff0,
        MIN_RECV_SIZE,
        byteenable,
        0x30000000
    };

    int sent = pcie_send_command((uint8_t *)&rh);
    assert(sent == 4);

    uint64_t recv_buf[2];
    uint16_t * recvBuf = (uint16_t *) recv_buf;
    assert(((uint64_t)recvBuf & PCIE_ALIGN_MASK) == 0);
    int recv = fpga_recv(fpga, chnl, recvBuf, MIN_RECV_SIZE, 25000);
    assert(recv == 4);

    *((uint16_t*) buf) = recvBuf[shift_count >> 1];
    return 2;
}

int pcie_read_1_aligned(void *buf, int offset)
{
    const int chnl = 0;
    uint32_t shift_count = (offset & 0xf);
    uint32_t byteenable  = 0x1 << shift_count;

    ReadHeader rh = {
        offset & 0xfffffff0,
        MIN_RECV_SIZE,
        byteenable,
        0x30000000
    };

    int sent = pcie_send_command((uint8_t *)&rh);
    assert(sent == 4);

    uint64_t recv_buf[2];
    uint8_t * recvBuf = (uint8_t *) recv_buf;
    assert(((uint64_t)recvBuf & PCIE_ALIGN_MASK) == 0);
    int recv = fpga_recv(fpga, chnl, recvBuf, MIN_RECV_SIZE, 25000);
    assert(recv == 4);

    *((uint8_t*) buf) = recvBuf[shift_count];
    return 1;
}

int pcie_read_max_aligned(void *buf, int max_count, int offset)
{
    if (max_count >= 16 && (offset & 0xf) == 0 && (((uint64_t)buf & PCIE_ALIGN_MASK)) == 0) {
        return pcie_read_16_aligned(buf, max_count & 0xfffffff0, offset);
    } else if (max_count >= 8 && (offset & 0x7) == 0) {
        return pcie_read_8_aligned(buf, offset);
    } else if (max_count >= 4 && (offset & 0x3) == 0) {
        return pcie_read_4_aligned(buf, offset);
    } else if (max_count >= 2 && (offset & 0x1) == 0) {
        return pcie_read_2_aligned(buf, offset);
    } else if (max_count >= 1) {
        return pcie_read_1_aligned(buf, offset);
    }
    // should not go here
    assert(0);
    return 0;
}

int pcie_read(void *buf, size_t len, int offset)
{
    pthread_mutex_lock(&fpga_lock);

    if (!fpga) pci_init();

    int recv = 0;
    int remain = len;

    while (remain > 0) {
        recv = pcie_read_max_aligned(buf, remain, offset);

        assert(recv > 0);
        buf += recv;
        remain -= recv;
        offset += recv;
    }

    pthread_mutex_unlock(&fpga_lock);

    return len;
}

