#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#include "alt_up_pci_lib.h"
#include "alt_up_pci_ioctl.h"

// Usage: ./reprogram save
//        <program a new sof>
//        ./repgrogram restore

// See acl_driver and acl_demo for the source of how reprogramming is done

#define SAVE "save"
#define RESTORE "restore"

int main(int argc, char *argv[])
{
  if (argc != 2) {
    printf("Usage: %s \"" SAVE "|" RESTORE "\"\n", argv[0]);
    return 1;
  }

  int fd = 0;
  struct alt_up_ioctl_arg handler;

  if (!strcmp(argv[1], SAVE)) {
    if (alt_up_pci_open(&fd, "/dev/alt_up_pci0")) {
      printf("Failed to open device\n");
      return 1;
    }

    // call pcie save registers
    if (ioctl(fd, ALT_UP_IOCTL_SAVE_REG, &handler)) {
      printf("ioctl() failed in saving pcie registers\n");
      return 1;
    }

    alt_up_pci_close(fd);
  } else if (!strcmp(argv[1], RESTORE)) {
    if (alt_up_pci_open(&fd, "/dev/alt_up_pci0")) {
      printf("Failed to open device\n");
      return 1;
    }

    // call pcie restore registers
    if (ioctl(fd, ALT_UP_IOCTL_RESTORE_REG, &handler)) {
      printf("ioctl() failed in restoring pcie registers\n");
      return 1;
    }

    alt_up_pci_close(fd);
  } else {
    printf("Usage: ./%s \"" SAVE "|" RESTORE "\"", argv[0]);
    return 1;
  }

  return 0;
}
