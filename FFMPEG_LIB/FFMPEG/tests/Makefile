#
# Makefile for tests
# (c) 2002 Fabrice Bellard
#
include ../config.mak

VPATH     = $(SRC_PATH_BARE)/tests
SRC_DIR   = $(SRC_PATH)/tests
BUILD_DIR = $(BUILD_ROOT)/tests
CFLAGS = -O2 -Wall -g

FFMPEG_REFFILE   = $(SRC_DIR)/ffmpeg.regression.ref
FFSERVER_REFFILE = $(SRC_DIR)/ffserver.regression.ref
LIBAV_REFFILE    = $(SRC_DIR)/libav.regression.ref
ROTOZOOM_REFFILE = $(SRC_DIR)/rotozoom.regression.ref
SEEK_REFFILE     = $(SRC_DIR)/seek.regression.ref

all fulltest test: codectest libavtest seektest

test-server: vsynth1/00.pgm asynth1.sw
	@echo
	@echo "Unfortunately ffserver is broken and therefore its regression"
	@echo "test fails randomly. Treat the results accordingly."
	@echo
	$(SRC_DIR)/server-regression.sh $(FFSERVER_REFFILE) $(SRC_DIR)/test.conf

codectest mpeg4 mpeg ac3 snow snowll: vsynth1/00.pgm vsynth2/00.pgm asynth1.sw tiny_psnr$(EXESUF)
	$(SRC_DIR)/regression.sh $@ $(FFMPEG_REFFILE) vsynth1
	$(SRC_DIR)/regression.sh $@ $(ROTOZOOM_REFFILE) vsynth2

ifeq ($(CONFIG_GPL),yes)
libavtest: vsynth1/00.pgm asynth1.sw
	$(SRC_DIR)/regression.sh $@ $(LIBAV_REFFILE) vsynth1
seektest: seek_test$(EXESUF)
	$(SRC_DIR)/seek_test.sh $(SEEK_REFFILE)
else
libavtest seektest:
	@echo
	@echo "This test requires FFmpeg to be compiled with --enable-gpl."
	@echo
	@exit 1
endif

ifeq ($(CONFIG_SWSCALER),yes)
test-server codectest mpeg4 mpeg ac3 snow snowll libavtest: swscale_error
swscale_error:
	@echo
	@echo "This regression test is incompatible with --enable-swscaler."
	@echo
	@exit 1
endif

vsynth1/00.pgm: videogen$(EXESUF)
	mkdir -p vsynth1
	$(BUILD_DIR)/$< 'vsynth1/'

vsynth2/00.pgm: rotozoom$(EXESUF)
	mkdir -p vsynth2
	$(BUILD_DIR)/$< 'vsynth2/' $(SRC_DIR)/lena.pnm

asynth1.sw: audiogen$(EXESUF)
	$(BUILD_DIR)/$< $@

%$(EXESUF): %.c
	$(CC) $(LDFLAGS) $(CFLAGS) -o $@ $<

seek_test$(EXESUF): seek_test.c
	$(CC) $(LDFLAGS) $(CFLAGS) -DHAVE_AV_CONFIG_H -I.. -I$(SRC_PATH)/libavformat -I$(SRC_PATH)/libavcodec -I$(SRC_PATH)/libavutil -o $@ $< $(BUILD_ROOT)/libavformat/libavformat.a $(BUILD_ROOT)/libavcodec/libavcodec.a $(BUILD_ROOT)/libavutil/libavutil.a $(EXTRALIBS)

distclean clean:
	rm -rf vsynth1 vsynth2 data asynth1.sw *~
	rm -f $(addsuffix $(EXESUF),audiogen videogen rotozoom seek_test tiny_psnr)

.PHONY: all fulltest test codectest libavtest test-server seektest
.PHONY: mpeg4 mpeg ac3 snow snowll swscale_error distclean clean
