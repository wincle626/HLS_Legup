#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>

#include "pcie_linux_driver_exports.h"
#include "alt_up_pci_lib_helper.h"

#define BAR_ID         0
#define DMA_BAR_ID     2

static int fd = 0;

int dma_is_idle(ssize_t f)
{
   unsigned int result = 0;
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_GET_DMA_IDLE_STATUS;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = &result;
   read (f, &driver_cmd, sizeof(result));
   
   return (result != 0);
}

void dma_update(ssize_t f)
{
   struct aclpci_cmd driver_cmd;
   driver_cmd.bar_id      = ACLPCI_CMD_BAR;
   driver_cmd.command     = ACLPCI_CMD_DMA_UPDATE;
   driver_cmd.device_addr = NULL;
   driver_cmd.user_addr   = NULL;
   read (f, &driver_cmd, 0);
   
}

// Function definitions
void pci_init()
{
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
}

void pci_close()
{
  close(fd);
  fd = 0;
}

void pci_read_direct(void *buf, size_t len, int offset)
{
  if (!fd) {
    pci_init();
  }

  // Read directly
  struct aclpci_cmd read_cmd = {BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf};
  if (read(fd, &read_cmd, len) == -1) {
    exit(1);
  }
}

void pci_write_direct(void *buf, size_t len, int offset)
{
  if (!fd) {
    pci_init();
  }

  // Write directly
  struct aclpci_cmd read_cmd = {BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf};
  if (write(fd, &read_cmd, len)) {
    exit(1);
  }
}

void pci_read_dma(void *buf, size_t len, int offset)
{
  // Read the data back
  struct aclpci_cmd read_cmd = {DMA_BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf};
  if (write(fd, &read_cmd, len)) {
    exit(1);
  }
}

void pci_write_dma(void *buf, size_t len, int offset)
{
  if (!fd) {
    pci_init();
  }

  // Write data via DMA
  struct aclpci_cmd read_cmd = {DMA_BAR_ID, ACLPCI_CMD_DEFAULT, (void *)offset, buf};
  if (write(fd, &read_cmd, len)) {
    exit(1);
  }
}

void pci_dma_go()
{
  // Release DMA
  while (!dma_is_idle(fd)) {
    dma_update(fd);
  }
}
