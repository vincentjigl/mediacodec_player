LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_SRC_FILES := \
	avplayer.cpp \
	main.cpp

LOCAL_CLANG := true
LOCAL_CPPFLAGS := 

LOCAL_MODULE := avtest

LOCAL_MODULE_TAGS := optional

LOCAL_SHARED_LIBRARIES := \
	libc \
	libstagefright liblog libutils libbinder libstagefright_foundation \
    libmedia libmedia_omx libgui libcutils libui 
	
LOCAL_C_INCLUDES := \
	frameworks/av/media/libstagefright \
	frameworks/av/media/libstagefright/foundation/include \
	bionic/libc/include/\
    bionic \
	$(TOP)/frameworks/native/include/media/openmax 
	
LOCAL_CFLAGS := -Wno-reorder

LOCAL_LDFLAGS := -ldl

include $(BUILD_EXECUTABLE)

