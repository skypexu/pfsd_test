假设你有一个curve盘: cbd:pool//pfstest


1. 先以root启动pfsdaemon:

/usr/local/polarstore/pfsd/bin/start_pfsd.sh pool@@pfstest

以上'@@'字符串是因为pfs不允许盘名包含'/'字符，所以将它变换为@，内部再转成'/'传递给curve.

2. 以root身份制作pfsd文件系统:
pfs -H 1 -C curve mkfs -f pool@@pfstest


3. 返回到普通用户：
编译：
	make

测试:
	./pfsd_test 1 pool@@pfstest


