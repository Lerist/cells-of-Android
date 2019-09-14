#pragma GCC system_header
#ifndef __GUIEXT_SERVICE_H__
#define __GUIEXT_SERVICE_H__

#include <utils/threads.h>
#include "ICellsPrivateService.h"

namespace android
{

class String8;
class CellsPrivateService :
        public BinderService<CellsPrivateService>,
        public BnCellsPrivateService
//        public Thread
{
    friend class BinderService<CellsPrivateService>;
public:

    CellsPrivateService();
    ~CellsPrivateService();

    static char const* getServiceName() { return "CellsPrivateService"; }

    virtual status_t setProperty(const String8& name,const String8& value);
    virtual status_t startCellsVM(const String8& name);
    virtual status_t stopCellsVM(const String8& name);
    virtual status_t cellsSwitchVM(const String8& name);
    virtual status_t cellsSwitchHOST(const String8& name);
    virtual status_t enterCell(const String8& name);
    virtual status_t enterHost(const String8& name);
    virtual status_t exitCell(const String8& name);
    virtual status_t exitHost(const String8& name);
    virtual status_t switchCellsVM(const String8& name);
};
};
#endif
