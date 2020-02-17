
seqtestpath = $(ROOTDIR)/tools/testframework/seqtest

vpath-y += $(seqtestpath)/src
inc-y += $(seqtestpath)/inc

obj-y += seqtest.o
obj-y += seqtest_output.o

