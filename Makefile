pfsd_test: pfsd_test.cc
	cc -o $@ -I /usr/local/polarstore/pfsd/include/ pfsd_test.cc -L /usr/local/polarstore/pfsd/lib/ -lpfsd -lpthread

