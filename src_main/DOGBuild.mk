DOG_SRC_MAIN := $(call my-dir)
LOCAL_PATH := $(DOG_SRC_MAIN)/$(DOG_PROJECT)

include $(DOG_PROJECT)/DOGBuild.mk

ifeq ($(TARGET_ARCH_ABI), armeabi-v7a)

  include $(CLEAR_VARS)
  DOG_LIBSUFFIX := arm
  include DOGBuildCore.mk

  include $(CLEAR_VARS)
  DOG_LIBSUFFIX := neon
  LOCAL_ARM_NEON := true
  include DOGBuildCore.mk

else

  include $(CLEAR_VARS)
  DOG_LIBSUFFIX := x86
  include DOGBuildCore.mk

endif