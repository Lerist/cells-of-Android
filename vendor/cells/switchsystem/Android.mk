LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

LOCAL_STATIC_JAVA_LIBRARIES := \
    android-support-v4 \
    okhttp3 \
    okhttp-okio

LOCAL_MODULE_TAGS := optional
LOCAL_SRC_FILES := $(call all-subdir-java-files)
LOCAL_RESOURCE_DIR += $(LOCAL_PATH)/res
LOCAL_PACKAGE_NAME := switchsystem
LOCAL_MODULE_OWNER := huawei
LOCAL_CERTIFICATE := platform
include $(BUILD_PACKAGE)

include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
	okhttp3:libs/okhttp-3.9.0.jar \

include $(BUILD_MULTI_PREBUILT)

include $(CLEAR_VARS)

LOCAL_PREBUILT_STATIC_JAVA_LIBRARIES := \
	okhttp-okio:libs/okio-1.13.0.jar \

include $(BUILD_MULTI_PREBUILT)

# Use the folloing include to make our test apk.
include $(call all-makefiles-under,$(LOCAL_PATH))