#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <readline/history.h>
#include <readline/readline.h>
#include <algorithm>
#include <sstream>
#include <iostream>
#include <map>

#include <pfsd_sdk.h>

std::map<std::string, int> fmap;

static void usage(const char *prog)
{
	fprintf(stderr, "usage: %s -H hostid -m mode\n", prog);
	fprintf(stderr, "\thostid\t host id\n");
	fprintf(stderr, "\tmode\t rw|ro\n");
}

static std::string
make_path(const char *pbd, std::string name)
{
	std::string s;

	s = '/';
	s += pbd;
	s += '/';
	s += name;
	return s;
}

int main(int argc, char **argv)
{
	int hostid = -1;
	int mode = 0;
        int fmode = 0;
	const char *cluster = "curve";
        const char *pbd = NULL;
	int opt;

	while ((opt = getopt(argc, argv, "C:H:m:")) != -1) {
		switch(opt) {
		case 'C':
			cluster = optarg;
			break;
		case 'H':
			hostid = atoi(optarg);
			break;
		case 'm':
			if (strcmp(optarg, "rw") == 0) {
				mode = PFS_RDWR;
				fmode = O_RDWR;
			} else if (strcmp(optarg, "ro") == 0) {
				mode = PFS_RD;
				fmode = O_RDONLY;
			} else {
               			fprintf(stderr, "unknown mode\n");
				usage(argv[0]);
			}
			break;
		case '?':
		default:
			usage(argv[0]);
			return 1;
		}
	}

	if (hostid == -1 || mode == 0){
		usage(argv[0]);
		return 1;
	}

	if (optind >= argc) {
               fprintf(stderr, "Expected pbdname tafter options\n");
               exit(EXIT_FAILURE);
        }
	pbd = argv[optind];

	pfsd_set_mode(PFSD_SDK_THREADS);

//	pfsd_set_svr_addr(svraddr, len);
	int ret = pfsd_mount(cluster, pbd, hostid, mode);
	if (ret) {
		printf("pfsd_mount failed\n");
		return 1;
	}

	printf("mount %s sucess\n", pbd);

	char *line = nullptr;

	for (;;) {
		int skip = 0;
		std::string path;
		if (line) {
			free(line);
			line = nullptr;
		}
		std::string line_str;
		std::string cmd;
		line = readline (":");
		if (line == nullptr) {
			std::cout << "\n";
			break;
		}
		line_str = line;
		line_str.erase(line_str.find_last_not_of(" \n\r\t")+1);
		if (line_str.size() == 0)
			continue;
		std::istringstream in(line_str);

		in >> cmd;
		if (cmd == "create") {
			std::string name;
			in >> name;
			if (!in) {
				std::cerr << "create filename\n";
				continue;
			}
			path = make_path(pbd, name);
			ret = pfsd_creat(path.c_str(), 0660);
			if (ret == -1) {
				std::cerr << "can not create " << path << std::endl;
				continue;
			} else {
				std::cout << "created " << path << std::endl;
				pfsd_close(ret);
			}
		} else if (cmd == "open") {
			std::string name;
			in >> name;
			if (!in) {
				std::cerr << "open filename\n";
				continue;
			}

			path = make_path(pbd, name);
			ret = pfsd_open(path.c_str(), fmode, 0660);
			if (ret == -1) {
				int err = errno;
				std::cerr << "can not open " << path << ", " << strerror(err) << std::endl;
				continue;
			} else {
				std::cout << "opened " << path << " fd " << ret << std::endl;
				fmap[name] = ret;
			}
		} else if (cmd == "close") {
			std::string name;
			in >> name;
			if (!in) {
				std::cerr << "close filename\n";
				continue;
			}

			if (fmap.find(name) == fmap.end()) {
				std::cerr << name << " not opened" << std::endl;
				continue;
			}
			pfsd_close(fmap[name]);
			fmap.erase(name);
		} else if (cmd == "showopen") {
			std::for_each(fmap.begin(), fmap.end(),
					[](std::map<std::string, int>::value_type &v) { std::cerr << v.first  << " : " << v.second <<std::endl; });
		} else if (cmd == "pwrite") {
			off_t pos;
			std::string name;
			std::string value;

			in >> name >> pos >> value;
			if (!in) {
				std::cerr << "pwrite filename position value\n";
				continue;
			}

			if (fmap.find(name) == fmap.end()) {
				std::cerr << name << " not opened" << std::endl;
				continue;
			}
			int fd = fmap[name];
			ret = pfsd_pwrite(fd, value.c_str(), value.size(), pos);
			if (ret == -1) {
				int err = errno;
				std::cerr << "write " << pbd << " failed, " << strerror(err) << std::endl;
			} else {
				std::cout << "write success\n";
			}
		} else if (cmd == "pread") {
			off_t pos;
			size_t size;
			std::string name;
			std::string value;

			in >> name >> pos >> size;
			if (!in) {
				std::cerr << "pwrite filename position size\n";
				continue;
			}

			if (fmap.find(name) == fmap.end()) {
				std::cerr << name << " not opened" << std::endl;
				continue;
			}
			int fd = fmap[name];
			value.resize(size);
			ret = pfsd_pread(fd, &value[0], value.size(), pos);
			if (ret == -1) {
				int err;
				std::cerr << "write " << pbd << " failed, " << strerror(err) << std::endl;
			} else {
				std::cout << "read success, data is: " << value << '\n';
			}
		} else {
			std::cerr << "unknown command " << cmd << std::endl;
			skip = 1;
		}

		if (!skip)
			add_history(line);
	}

	free(line);
	line = nullptr;
	return 0;
}
