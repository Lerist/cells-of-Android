#define LOG_TAG "CELLSSERVICE"

#include <utils/Errors.h>
#include <utils/RefBase.h>
#include <utils/Vector.h>
#include <utils/Timers.h>
#include <utils/String8.h>

#include <binder/Parcel.h>
#include <binder/IInterface.h>

#include <cutils/log.h>

#include "ICellsPrivateService.h"

namespace android {

class BpCellsPrivateService : public BpInterface<ICellsPrivateService>
{
public:
    BpCellsPrivateService(const sp<IBinder>& impl) : BpInterface<ICellsPrivateService>(impl)
    {
    }

    virtual status_t setProperty(const String8& name,const String8& value)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        data.writeString8(value);
        status_t result = remote()->transact(SETPROPERTY, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not set property\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t startCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(STARTCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not start cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t stopCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(STOPCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not stop cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t cellsSwitchVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(CELLSSWITCHVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not cells switch vm. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t cellsSwitchHOST(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(CELLSSWITCHHOST, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not cells switch host. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t enterCell(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(ENTERCELL, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not enter cell. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t enterHost(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(ENTERHOST, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not enter host. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t exitCell(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(EXITCELL, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not exit cell. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t exitHost(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(EXITHOST, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not exit host. \n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t switchCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(SWITCHCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not switch cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t uploadCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(UPLOADCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not upload cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t downloadCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(DOWNLOADCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not download cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t untarCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(UNTARCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not untar cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }

    virtual status_t tarCellsVM(const String8& name)
    {
        Parcel data, reply;
        data.writeInterfaceToken(ICellsPrivateService::getInterfaceDescriptor());
        data.writeString8(name);
        status_t result = remote()->transact(TARCELLSVM, data, &reply);
        if (result != NO_ERROR) {
            ALOGE("could not tar cellsvm\n");
            return result;
        }
        result = reply.readInt32();
        return result;
    }
};

IMPLEMENT_META_INTERFACE(CellsPrivateService, "CellsPrivateService");

status_t BnCellsPrivateService::onTransact(uint32_t code, const Parcel& data, Parcel* reply, uint32_t flags)
{
    switch(code)
    {
        case SETPROPERTY:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();
            String8 value = data.readString8();

            status_t result = setProperty(name, value);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case STARTCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = startCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case STOPCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = stopCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case CELLSSWITCHVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = cellsSwitchVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case CELLSSWITCHHOST:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = cellsSwitchHOST(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case ENTERCELL:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = enterCell(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case ENTERHOST:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = enterHost(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case EXITCELL:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = exitCell(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case EXITHOST:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = exitHost(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case SWITCHCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = switchCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case UPLOADCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = uploadCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case DOWNLOADCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = downloadCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case UNTARCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = untarCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
        case TARCELLSVM:
        {
            CHECK_INTERFACE(ICellsPrivateService, data, reply);
            String8 name = data.readString8();

            status_t result = tarCellsVM(name);
            reply->writeInt32(result);
            return NO_ERROR;
        }
        break;
    }
    return BBinder::onTransact(code, data, reply, flags);
}

};
