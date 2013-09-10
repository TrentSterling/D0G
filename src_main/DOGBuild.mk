DOG_SRC_MAIN := $(call my-dir)
LOCAL_PATH := $(DOG_SRC_MAIN)/$(DOG_PROJECT)

include $(DOG_PROJECT)/DOGBuild.mk

include $(CLEAR_VARS)
LOCAL_MODULE := $(DOG_PROJECT)
include DOGBuildCore.mk

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)
  include $(CLEAR_VARS)
  LOCAL_ARM_NEON := true
  LOCAL_MODULE := $(DOG_PROJECT)_neon
  include DOGBuildCore.mk
endif