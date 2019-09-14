#define LOG_TAG "GuiExt"

#include <cutils/log.h>
#include <binder/BinderService.h>
#include "ICellsPrivateService.h"

using namespace android;

int main(int argc, char** argv)
{
   const sp<IServiceManager> sm = defaultServiceManager();
    if (sm != NULL) {
        sp<IBinder> binder = sm->checkService(String16("CellsPrivateService"));
        if (binder != NULL) {
            sp<ICellsPrivateService> pCellsPrivateService = interface_cast<ICellsPrivateService>(binder);
            if(pCellsPrivateService == NULL){
                ALOGE("could not get service CellsPrivateService \n");
                return 0;
            }

            pCellsPrivateService->switchCellsVM(android::String8("host"));
        }
    }

    return 0;
}
