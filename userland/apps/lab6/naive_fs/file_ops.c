#include <string.h>
#include <stdio.h>

#include "file_ops.h"
#include "block_layer.h"

#define MAX_FILE 100
#define BLOCK_SIZE 512

int naive_fs_access(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */

    for (int i = 1; i <= MAX_FILE; i ++) {
        char read_name[BLOCK_SIZE];
        sd_bread(i, read_name);
        if (strcmp(name, read_name) == 0){
            return 0;
        }
    }

    /* BLANK END */
    /* LAB 6 TODO END */
    return -1;
}

int naive_fs_creat(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */

    if (naive_fs_access(name) == 0) {
        return -1;
    }

    for (int i = 1; i <= MAX_FILE; i ++) {
        char read_name[BLOCK_SIZE];
        sd_bread(i, read_name);
        if (read_name[0] == '\0'){
            sd_bwrite(i, name);
            return 0;
        }
    }

    /* BLANK END */
    /* LAB 6 TODO END */
    return -1;
}

int naive_fs_pread(const char *name, int offset, int size, char *buffer)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */

    if (naive_fs_access(name) != 0) {
        return -1;
    }

    int file_lba = MAX_FILE;
    for (int i = 1; i <= MAX_FILE; i ++) {
        char read_name[BLOCK_SIZE];
        sd_bread(i, read_name);
        if (strcmp(name, read_name) == 0){
            file_lba += i;
            break;
        }
    }

    char buf[BLOCK_SIZE];
    sd_bread(file_lba, buf);
    memcpy(buffer, buf + offset, size);
    return size;

    /* BLANK END */
    /* LAB 6 TODO END */
    return -1;
}

int naive_fs_pwrite(const char *name, int offset, int size, const char *buffer)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */

    if (naive_fs_access(name) != 0) {
        return -1;
    }

    int file_lba = MAX_FILE;
    for (int i = 1; i <= MAX_FILE; i ++) {
        char read_name[BLOCK_SIZE];
        sd_bread(i, read_name);
        if (strcmp(name, read_name) == 0){
            file_lba += i;
            break;
        }
    }

    printf("find block %d\n", file_lba);

    char buf[BLOCK_SIZE];
    sd_bread(file_lba, buf);
    memcpy(buf + offset, buffer, size);
    sd_bwrite(file_lba, buf);
    return size;

    /* BLANK END */
    /* LAB 6 TODO END */
    return -1;
}

int naive_fs_unlink(const char *name)
{
    /* LAB 6 TODO BEGIN */
    /* BLANK BEGIN */

    for (int i = 1; i <= MAX_FILE; i ++) {
        char read_name[BLOCK_SIZE];
        sd_bread(i, read_name);
        if (strcmp(name, read_name) == 0){
            char* buf;
            sd_bwrite(i, buf);
            return 0;
        }
    }

    /* BLANK END */
    /* LAB 6 TODO END */
    return -1;
}
