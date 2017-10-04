#include <riffa.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CMD_ID_RESET 1
#define CMD_ID_SAVE 2
#define CMD_ID_RESTORE 3
#define CMD_ID_UNKNOWN -1

#define CMD_STR_RESET "reset"
#define CMD_STR_SAVE "save"
#define CMD_STR_RESTORE "restore"

static inline void help(FILE * out) {
    fprintf(out, "\nusage: reprog <fpga_id> <COMMAND1> <COMMAND2> ..\n"
            "Valid Commands: save reset restore\n");
}

int getCmdId(const char * cmdStr)
{
    if (strncmp(CMD_STR_RESET, cmdStr, sizeof(CMD_STR_RESET)) == 0) {
        return CMD_ID_RESET;
    } else if (strncmp(CMD_STR_SAVE, cmdStr, sizeof(CMD_STR_SAVE)) == 0) {
        return CMD_ID_SAVE;
    } else if (strncmp(CMD_STR_RESTORE, cmdStr, sizeof(CMD_STR_RESTORE)) == 0) {
        return CMD_ID_RESTORE;
    }
    return CMD_ID_UNKNOWN;
}

int main(int argc, char * argv[])
{
    if (argc < 3) {
        help(stdout);
        exit(-1);
    }

    int fpga_id = -1;
    if (sscanf(argv[1], "%d", &fpga_id) != 1) {
        help(stdout);
        fprintf(stderr, "ERROR: Unable to read fpga_id\n");
        exit(-1);
    }

    fpga_t * fpga;
	if ((fpga = fpga_open(fpga_id)) == NULL) {
        fprintf(stderr, "ERROR: Unable to open fpag with id = %d\n", fpga_id);
        exit(-1);
    }

    for (int i = 2; i < argc; i++) {
        int cmdId = getCmdId(argv[i]);
        switch (cmdId) {
            case CMD_ID_RESET:
		        fpga_reset(fpga);
                break;
            case CMD_ID_SAVE:
		        fpga_save_state(fpga);
                break;
            case CMD_ID_RESTORE:
		        fpga_restore_state(fpga);
                break;
            default:
                fprintf(stderr, "ERROR: Unknown command: %s\n", argv[i]);
                help(stdout);
                fpga_close(fpga);
                exit(-1);
                break;
        }
    }
    fpga_close(fpga);
    exit(0);
} 

