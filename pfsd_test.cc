#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>

#include "pfsd_sdk.h"

int main(int argc, char **argv)
{
	if (argc < 3) {
		printf("usage: %s host_id pbd_name\n", argv[0]);
		return 1;
	}

	pfsd_set_mode(PFSD_SDK_THREADS);
//	pfsd_set_svr_addr(svraddr, len);
	int hostid = atoi(argv[1]);
	const char *pbd = argv[2];
	int ret = pfsd_mount("curve", argv[2], hostid, PFS_RDWR);
	if (ret) {
		printf("pfsd_mount failed\n");
		return 1;
	}

	char path[256];
	snprintf(path, sizeof(path), "/%s/my_test_file", pbd);
	int fd = pfsd_open(path, O_CREAT|O_RDWR, 0777);
	if (fd == -1) {
		printf("pfsd_open failed\n");
		return 1;
	}

	ret = pfsd_write(fd, "hello world\n", 12);
	if (ret != 12) {
		printf("pfsd_write failed\n");
		return 1;
	}
	printf("wrote %d bytes\n", ret);

	ret = pfsd_lseek(fd, 0, SEEK_SET);
	if (ret == -1) {
		printf("pfsd_lseek failed\n");
		return 1;
	}

	char buf[128];
	ret = pfsd_read(fd, buf, 12);
	printf("read %d bytes\n", ret);
	if (ret != 12) {
		printf("pfsd_read failed %d\n", ret);
		return 1;
	} else {
		printf("data is consistent\n");
	}

	if (strncmp(buf, "hello world\n", 12)) {
		printf("data inconsistent\n");
		return 1;
	}

	ret = pfsd_close(fd);
	if (ret) {
		printf("pfsd_close failed\n");
		return 1;
	}

	ret = pfsd_unlink(path);
	if (ret) {
		printf("pfsd_unlink failed\n");
	}

	ret = pfsd_umount(pbd);
	if (ret) {
		printf("pfsd_umount failed\n");
	}
	return 0;
}

