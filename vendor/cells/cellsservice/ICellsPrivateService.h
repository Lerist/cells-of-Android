#ifndef __ICELLSSERVICE_H__
#define __ICELLSSERVICE_H__

#include <binder/IInterface.h>
#include <binder/Parcel.h>
#include <binder/BinderService.h>
#include <cutils/properties.h>
#include <utils/String8.h>

namespace android
{

class String8;
class ICellsPrivateService : public IInterface
{
protected:
    enum {
        SETPROPERTY = IBinder::FIRST_CALL_TRANSACTION,
        STARTCELLSVM = IBinder::FIRST_CALL_TRANSACTION + 1,
        STOPCELLSVM = IBinder::FIRST_CALL_TRANSACTION + 2,
        SWITCHCELLSVM = IBinder::FIRST_CALL_TRANSACTION + 3,
        CELLSSWITCHVM = IBinder::FIRST_CALL_TRANSACTION + 4,
        CELLSSWITCHHOST = IBinder::FIRST_CALL_TRANSACTION + 5,
        ENTERCELL = IBinder::FIRST_CALL_TRANSACTION + 6,
        ENTERHOST = IBinder::FIRST_CALL_TRANSACTION + 7,
        EXITCELL = IBinder::FIRST_CALL_TRANSACTION + 8,
        EXITHOST = IBinder::FIRST_CALL_TRANSACTION + 9,
    };

public:
    DECLARE_META_INTERFACE(CellsPrivateService);

    virtual status_t setProperty(const String8& name,const String8& value) = 0;
    virtual status_t startCellsVM(const String8& name) = 0;
    virtual status_t stopCellsVM(const String8& name) = 0;
    virtual status_t cellsSwitchVM(const String8& name) = 0;
    virtual status_t cellsSwitchHOST(const String8& name) = 0;
    virtual status_t enterCell(const String8& name) = 0;
    virtual status_t enterHost(const String8& name) = 0;
    virtual status_t exitCell(const String8& name) = 0;
    virtual status_t exitHost(const String8& name) = 0;
    virtual status_t switchCellsVM(const String8& name) = 0;
};

class BnCellsPrivateService : public BnInterface<ICellsPrivateService>
{
    virtual status_t onTransact(uint32_t code,
                                const Parcel& data,
                                Parcel* reply,
                                uint32_t flags = 0);
};

};

#endif
