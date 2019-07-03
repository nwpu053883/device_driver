#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <string.h>
#include <errno.h>
#include <assert.h>

#define MEM_4K    4096
#define GMEM_DEV "/dev/simple_chr"

int main(int argc, char *argv[]) {
    int ret = 0;
    int fd, opt, count, cmd, pos;
    char tmp_buf[MEM_4K];

    if (argc < 2) {
        printf("-h for help\n");
    }

    fd = open(GMEM_DEV, O_RDWR);

    if (fd < 0) {
        printf("open %s failed! errno: %d(%m)\n", GMEM_DEV, errno);
        exit(EXIT_FAILURE);
    }

    while ((opt = getopt(argc, argv, "hr:w:i:s:"))  != -1) {
        switch (opt) {
            case 'i':   /* ioctl*/
                // printf("do ioctl case!\n");
                cmd = atoi(optarg);
                ioctl(fd, cmd);
                break;

            case 'r':   /* read data to stdout */
                // printf("do read case!\n");
                count = atoi(optarg);
                ret = read(fd, tmp_buf, count);
                if (ret <= 0) {
                    fprintf(stderr, "read failed! errno: %d(%m)\n", errno);
                } else {
                    printf("%s\n", tmp_buf);
                }

                break;

            case 'w':
                // printf("do write case!\n");
                assert(optarg);
                count = strlen(optarg);
                ret = write(fd, optarg, count+1);
                if (ret <= 0) {
                    fprintf(stderr, "write failed! errno: %d(%m)\n", errno);
                }
                break;

            case 's':
                printf("do seek case!\n");
                pos = atoi(optarg);
                ret = lseek(fd, pos, SEEK_SET);
                if (ret <= 0) {
                    fprintf(stderr, "lseek failed! errno: %d(%m)\n", errno);
                }
                break;

            case 'h':
                printf("usage: gmem_demo -rwis\n");
                break;

            default:
                printf("-h for help\n");
                break;
        }
    }

    close(fd);

    printf("exit main...\n");

    return 0;
}

