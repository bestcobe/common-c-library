# Copyright (C) 2014 The QpidNetwork Project
# LiveChat Module Makefile
#
# Created on: 2015/10/21
# Author: Samson Fan
# LiveChatManager for man

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := livechatmanmanager

LOCAL_MODULE_FILENAME := liblivechatmanmanager

#LOCAL_LDLIBS += -llog
#LOCAL_LDLIBS += -lz

LOCAL_STATIC_LIBRARIES += common
LOCAL_STATIC_LIBRARIES += json
LOCAL_STATIC_LIBRARIES += amf
LOCAL_STATIC_LIBRARIES += livechat
LOCAL_STATIC_LIBRARIES += imghandle

LOCAL_CPPFLAGS  := -std=c++11

LOCAL_C_INCLUDES += $(LOCAL_PATH)/../
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../manrequesthandler
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../manrequesthandler/item
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../third_party/
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../livechat
LOCAL_C_INCLUDES += $(LOCAL_PATH)/../imghandle

REAL_PATH := $(realpath $(LOCAL_PATH))
LOCAL_SRC_FILES := $(call all-cpp-files-under, $(REAL_PATH))

include $(BUILD_STATIC_LIBRARY)