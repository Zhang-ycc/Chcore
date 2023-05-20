/*
 * Copyright (c) 2022 Institute of Parallel And Distributed Systems (IPADS)
 * ChCore-Lab is licensed under the Mulan PSL v1.
 * You can use this software according to the terms and conditions of the Mulan PSL v1.
 * You may obtain a copy of Mulan PSL v1 at:
 *     http://license.coscl.org.cn/MulanPSL
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY OR FIT FOR A PARTICULAR
 * PURPOSE.
 * See the Mulan PSL v1 for more details.
 */

#include <stdio.h>
#include <string.h>
#include <chcore/types.h>
#include <chcore/fsm.h>
#include <chcore/tmpfs.h>
#include <chcore/ipc.h>
#include <chcore/internal/raw_syscall.h>
#include <chcore/internal/server_caps.h>
#include <chcore/procm.h>
#include <chcore/fs/defs.h>

typedef __builtin_va_list va_list;
#define va_start(v, l) __builtin_va_start(v, l)
#define va_end(v)      __builtin_va_end(v)
#define va_arg(v, l)   __builtin_va_arg(v, l)
#define va_copy(d, s)  __builtin_va_copy(d, s)


extern struct ipc_struct *fs_ipc_struct;

/* You could add new functions or include headers here.*/
/* LAB 5 TODO BEGIN */

#define BUF_SIZE 512

int atoi(char* str) {
	long number = 0;
	int flag = 1;
	while (*str == ' ') {
		str++;
	}
	if (*str == '-')
	{
		flag = -1;
		str++;
	}
	while ((*str >= '0') && (*str <= '9')) 
	{
		number = number * 10 + *str - '0';
		str++;
	}
	return flag * number;
}

char* itoa(int n, char* buf) {
    int i, j, sign;
	char str[BUF_SIZE];

	if ((sign = n) < 0) {
		n = -n;
	}

	i = 0;
	do{
		str[i++] = n % 10 + '0';
	}
	while ((n /= 10) > 0);

	if (sign < 0) {
		str[i++]= '-';
	}
	str[i] = '\0';

	j = i;
	while (j > 0) {
		buf[i - j] = str[j - 1];
		j --;
	}

	return buf;
}


int alloc_fd() 
{
	static int cnt = 0;
	return ++cnt;
}

int open_file(int fd, const char *fspath, int mode)
{
	int ret;
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request) + 2, 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_OPEN;
    strcpy(fr->open.pathname, fspath);
	fr->open.flags = mode;
	fr->open.mode = mode;
	fr->open.new_fd = fd;
    ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    return ret;
}

int create_file(const char *fspath)
{
	int ret;
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request), 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_CREAT;
    strcpy(fr->creat.pathname, fspath);
    ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);
    return ret;
}

/* LAB 5 TODO END */

FILE *fopen(const char * filename, const char * mode) {
	/* LAB 5 TODO BEGIN */

	FILE *file = malloc(sizeof(FILE));
	int fd = alloc_fd();
	int open_mode = 0;

	if (*mode == 'r'){
		open_mode = O_RDONLY;
	}
	else if (*mode == 'w'){
		open_mode = O_WRONLY;
	}

	int ret = open_file(fd, filename, open_mode);
	if (ret >= 0) {
		file->fd = fd;
	}
	else
	{
		//create
		create_file(filename);
		file = fopen(filename, mode);
	}

	/* LAB 5 TODO END */
    return file;
}

size_t fwrite(const void * src, size_t size, size_t nmemb, FILE * f) {

	// /* LAB 5 TODO BEGIN */

	// int ret;
	// int len = size * nmemb;

    // struct ipc_msg *ipc_msg = ipc_create_msg(
    //     fs_ipc_struct, sizeof(struct fs_request) + len + 2, 0);
    // chcore_assert(ipc_msg);
    // struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    // fr->req = FS_REQ_WRITE;
    // fr->write.count = len;
	// fr->write.fd = f->fd;
	// ipc_set_msg_data(ipc_msg, src, sizeof(struct fs_request), len + 2);
    // ret = ipc_call(fs_ipc_struct, ipc_msg);
    // ipc_destroy_msg(fs_ipc_struct, ipc_msg);

	// /* LAB 5 TODO END */
    // return ret;

	 /* LAB 5 TODO BEGIN */
        size_t len = size * nmemb;
        struct ipc_msg *ipc_msg = ipc_create_msg(
                fs_ipc_struct, sizeof(struct fs_request) + len, 0);
        chcore_assert(ipc_msg);
        struct fs_request *fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
        memcpy((void *)fr + sizeof(struct fs_request), src, len);
        fr->req = FS_REQ_WRITE;
        fr->write.count = len;
        fr->write.fd = f->fd;
        int ret = ipc_call(fs_ipc_struct, ipc_msg);
        chcore_bug_on(ret != len);
        ipc_destroy_msg(fs_ipc_struct, ipc_msg);
        /* LAB 5 TODO END */
        return ret;

}

size_t fread(void * destv, size_t size, size_t nmemb, FILE * f) {

	/* LAB 5 TODO BEGIN */

	int ret;
	int len = size * nmemb;

    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request) + len + 2, 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_READ;
	fr->read.count = len;
	fr->read.fd = f->fd;
    ret = ipc_call(fs_ipc_struct, ipc_msg);
	memcpy(destv, ipc_get_msg_data(ipc_msg), ret);
	printf("%s\n", destv);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);

	/* LAB 5 TODO END */
    return ret;

}

int fclose(FILE *f) {

	/* LAB 5 TODO BEGIN */

	int ret;
    struct ipc_msg *ipc_msg = ipc_create_msg(
        fs_ipc_struct, sizeof(struct fs_request), 0);
    chcore_assert(ipc_msg);
    struct fs_request * fr = (struct fs_request *)ipc_get_msg_data(ipc_msg);
    fr->req = FS_REQ_CLOSE;
	fr->close.fd = f->fd;
    ret = ipc_call(fs_ipc_struct, ipc_msg);
    ipc_destroy_msg(fs_ipc_struct, ipc_msg);

	/* LAB 5 TODO END */
    return ret;

}

/* Need to support %s and %d. */
int fscanf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */

	char scan_buf[BUF_SIZE];
	memset(scan_buf, 0x0, sizeof(scan_buf));
	fread(scan_buf, sizeof(char), sizeof(scan_buf), f);
	printf("scanf buf is %s\n", scan_buf);

	va_list list;
	va_start(list, fmt);
	printf("%s\n", fmt);

	char *cur = (char *)fmt;
	char *start = scan_buf;

	while (*cur && *cur != '\0')
	{
		if (*cur == '%'){
			cur ++;
			if (*cur == 's'){
				char *end = start;
				while (*end && *end != ' ' && *end != '\0'){
					end ++;
				}

				char *arg = va_arg(list, char *);
				memcpy(arg, start, end - start);

				start = end + 1;
			}
			else if (*cur == 'd'){
				char *end = start;
				while (*end && *end != ' ' && *end != '\0'){
					end ++;
				}

				char str[BUF_SIZE];
				memcpy(str, start, end - start);

				int *arg = va_arg(list, int *);
				memcpy(arg, start, end - start);
				*arg = atoi(str);

				start = end + 1;
			}
		}
		cur ++;
	}

	va_end(list);

	/* LAB 5 TODO END */
    return 0;
}

void int2str(int n, char *str){
	char buf[256];
	int i = 0, tmp = n;

	if(!str){
		return;
	}
	while(tmp){
		buf[i] = (char)(tmp % 10) + '0';
		tmp /= 10;
		i++;
	}
	int len = i;
	str[i] = '\0';
	while(i > 0){
		str[len - i] = buf[i - 1];
		i--;
	}
}

/* Need to support %s and %d. */
int fprintf(FILE * f, const char * fmt, ...) {

	/* LAB 5 TODO BEGIN */

	char print_buf[BUF_SIZE];
	memset(print_buf, 0x0, sizeof(print_buf));

	va_list list;
	va_start(list, fmt);

	char *cur = (char *)fmt;
	char *offset = print_buf;
	char *prev = (char *)fmt;

	while (*cur && *cur != '\0')
	{
		if (*cur == '%'){
			memcpy(offset, prev, cur - prev);
			offset += cur - prev;

			cur ++;
			if (*cur == 's'){
				char *arg = va_arg(list, char *);
				memcpy(offset, arg, strlen(arg));
				offset += strlen(arg);
			}
			else if (*cur == 'd'){
				char str[BUF_SIZE];
				int arg = va_arg(list, int);
				// int2str(arg, str);
				itoa(arg, str);
				memcpy(offset, str, strlen(str));
			}

			prev = cur + 1;
		}
		cur ++;
	}
	
	fwrite(print_buf, sizeof(char), sizeof(print_buf), f);

	va_end(list);

	/* LAB 5 TODO END */
    return 0;

}