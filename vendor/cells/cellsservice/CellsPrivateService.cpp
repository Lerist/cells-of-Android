#define LOG_TAG "CELLSSERVICE"
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <cutils/log.h>
#include <cutils/properties.h>
#include "CellsPrivateService.h"

#include <binder/IServiceManager.h>
#include <gui/ISurfaceComposer.h>
#include <utils/String16.h>

#include <powermanager/IPowerManager.h>
#include <powermanager/PowerManager.h>

namespace android {

#define SYSTEMPRIVATE_LOGV(x, ...) ALOGV("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGD(x, ...) ALOGD("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGI(x, ...) ALOGI("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGW(x, ...) ALOGW("[CellsPrivate] " x, ##__VA_ARGS__)
#define SYSTEMPRIVATE_LOGE(x, ...) ALOGE("[CellsPrivate] " x, ##__VA_ARGS__)

CellsPrivateService::CellsPrivateService()
{
}

CellsPrivateService::~CellsPrivateService()
{
}

status_t CellsPrivateService::setProperty(const String8& name,const String8& value)
{
    SYSTEMPRIVATE_LOGD("SETPROPERTY arg %s %s", name.string(), value.string());
    status_t result = property_set(name, value);
    SYSTEMPRIVATE_LOGD("SETPROPERTY result = %d", result);
    return result;
}

status_t CellsPrivateService::startCellsVM(const String8& name)
{
    name;

    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc start cell1");
    SYSTEMPRIVATE_LOGD("STARTCELLSVM cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

status_t CellsPrivateService::stopCellsVM(const String8& name)
{
    name;

    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc stop cell1");
    SYSTEMPRIVATE_LOGD("STOPCELLSVM cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

status_t CellsPrivateService::cellsSwitchVM(const String8& name)
{
    name;

    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc switch cell1");
    SYSTEMPRIVATE_LOGD("CELLSSWITCHVM cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

status_t CellsPrivateService::cellsSwitchHOST(const String8& name)
{
    name;

    char cmd[200];
    snprintf(cmd, sizeof(cmd), "cellc switch host");
    SYSTEMPRIVATE_LOGD("CELLSSWITCHHOST cmd = %s", cmd);
    system(cmd);
    return NO_ERROR;
}

static void* gotosleep(void* o)
{
    o;

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::IPowerManager> mPowerManager = 
            android::interface_cast<android::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->goToSleep(long(ns2ms(systemTime())),GO_TO_SLEEP_REASON_POWER_BUTTON,0);
        }
    }

    ALOGD("BACK SYSTEM go to sleep...");

    return (void*)0;
};

static void create_gotosleep_pthread()
{
    int ret;
    pthread_t daemon_thread;
    
    /* Start listening for connections in a new thread */
    ret = pthread_create(&daemon_thread, NULL,gotosleep, NULL);
    if (ret) {
        ALOGE("create_gotosleep_pthread err: %s", strerror(errno));
    }
};

status_t CellsPrivateService::switchCellsVM(const String8& name)
{
    name;

    char value[PROPERTY_VALUE_MAX];
    property_get("ro.boot.vm", value, "1");
    if((strcmp(value, "0") == 0)){
        {
            exitHost(android::String8("host"));
        }
    
        {
            cellsSwitchVM(android::String8("cell1"));
        }
    
        {
            android::sp<android::IServiceManager> sm = android::otherdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
                android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->enterCell(android::String8("cell1"));
            }
        }
    }else{
        {
            exitCell(android::String8("cell1"));
        }
    
        {
            android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
            android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->cellsSwitchHOST(android::String8("host"));
            }
        }
    
        {
            android::sp<android::IServiceManager> sm = android::initdefaultServiceManager();
            android::sp<android::ICellsPrivateService> mCellsPrivateService = 
            android::interface_cast<android::ICellsPrivateService>(sm->checkService(android::String16("CellsPrivateService")));
            if(mCellsPrivateService != NULL){
                mCellsPrivateService->enterHost(android::String8("host"));
            }
        }
    }

    {
        create_gotosleep_pthread();
    }

    SYSTEMPRIVATE_LOGD("SWITCHCELLSVM result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::enterHost(const String8& name)
{
    name;

    {
        property_set("persist.sys.exit", "0");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::IPowerManager> mPowerManager = 
            android::interface_cast<android::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->wakeUp(long(ns2ms(systemTime())),android::String16("enter_self"),android::String16(""));
        }
    }

    {
        property_set("ctl.start", "adbd");
        property_set("ctl.start", "qmuxd");
        property_set("ctl.start", "ril-daemon");
    }

    {
        system("ndc tether interface remove vm_wlan");
        system("ndc network interface remove local vm_wlan");
        //system("ndc tether stop");
        system("ndc ipfwd disable vm_tethering");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->enterSelf();
        }
    }

    SYSTEMPRIVATE_LOGD("ENTERHOST result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::exitHost(const String8& name)
{
    name;

    {
        property_set("persist.sys.exit", "1");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->exitSelf();
        }
    }

    {
        property_set("ctl.stop", "adbd");
        property_set("ctl.stop", "qmuxd");
        property_set("ctl.stop", "ril-daemon");
    }

    {
        system("ndc tether interface remove vm_wlan");
        system("ndc network interface remove local vm_wlan");
        //system("ndc tether stop");
        system("ndc ipfwd disable vm_tethering");

        system("ndc interface setcfg vm_wlan 172.17.3.2 16 up multicast broadcast");
        system("ndc tether interface add vm_wlan");
        system("ndc network interface add local vm_wlan");
        system("ndc network route add local vm_wlan 172.17.0.0/16");
        system("ndc ipfwd enable vm_tethering");
        system("ndc nat enable vm_wlan wlan0 1 172.17.0.0/16");
        system("ndc ipfwd add vm_wlan wlan0");
    }

    /*{
        setCellstared();
    }*/

    SYSTEMPRIVATE_LOGD("EXITHOST result = %d", 0);
    return NO_ERROR;
}

static void write_vm_exit(bool bexit){
    int vmfd = open("/.cell",O_WRONLY);
    if(vmfd>=0){
        if(bexit)
            write(vmfd,"1",strlen("1"));
        else
            write(vmfd,"0",strlen("0"));
        close(vmfd);
    }
}

status_t CellsPrivateService::enterCell(const String8& name)
{
    name;

    {
        property_set("persist.sys.exit", "0");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::IPowerManager> mPowerManager = 
            android::interface_cast<android::IPowerManager>(sm->checkService(android::String16("power")));
        if(mPowerManager != NULL){
            mPowerManager->wakeUp(long(ns2ms(systemTime())),android::String16("enter_self"),android::String16(""));
        }
    }

    {
        property_set("ctl.start", "adbd");
        property_set("ctl.start", "qmuxd");
        property_set("ctl.start", "ril-daemon");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->enterSelf();
        }
    }

    {
        write_vm_exit(false);
    }

    SYSTEMPRIVATE_LOGD("ENTERCELL result = %d", 0);
    return NO_ERROR;
}

status_t CellsPrivateService::exitCell(const String8& name)
{
    name;

    {
        write_vm_exit(true);
    }

    {
        property_set("persist.sys.exit", "1");
    }

    {
        android::sp<android::IServiceManager> sm = android::defaultServiceManager();
        android::sp<android::ISurfaceComposer> mComposer = 
            android::interface_cast<android::ISurfaceComposer>(sm->checkService(android::String16("SurfaceFlinger")));
        if(mComposer != NULL){
            mComposer->exitSelf();
        }
    }

    {
        property_set("ctl.stop", "adbd");
        property_set("ctl.stop", "qmuxd");
        property_set("ctl.stop", "ril-daemon");
    }

    SYSTEMPRIVATE_LOGD("EXITCELL result = %d", 0);
    return NO_ERROR;
}

};
