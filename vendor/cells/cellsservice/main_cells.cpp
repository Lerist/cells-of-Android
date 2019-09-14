#define LOG_TAG "CellsService"

#include <fcntl.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#include <binder/BinderService.h>
#include <CellsPrivateService.h>

using namespace android;

static void init_vm_property_value()
{
	char buf[10];
	int len = 0;
	int vmfd = open("/.cell",O_RDONLY);
	if(vmfd>=0){
		len = read(vmfd, buf, 10);
		if(len > 0){
			if(strcmp(buf,"0") == 0){
				property_set("persist.sys.exit", "0");
				close(vmfd);
				return ;
			}
		}
		close(vmfd);
	}

	property_set("persist.sys.exit", "1");
}

static void config_vm_net_work(void)
{
	system("ip link set vm_wlan0 name wlan0");
	ALOGD("ip link set vm_wlan0 name wlan0 errno=%s",strerror(errno));

	system("ifconfig wlan0 172.17.3.3 netmask 255.255.0.0 up");
	ALOGD("ifconfig wlan0 172.17.3.3 netmask 255.255.0.0 up errno=%s",strerror(errno));

	system("route add default gw 172.17.3.2");
	ALOGD("route add default gw 172.17.3.2 errno=%s",strerror(errno));
};

int main(int /*argc*/, char** /*argv*/)
{
	ALOGI("GuiExt service start...");

	char value[PROPERTY_VALUE_MAX];
	property_get("ro.boot.vm", value, "1");
	if((strcmp(value, "0") == 0)){
		property_set("persist.sys.exit", "0");
		property_set("persist.sys.vm.init", "0");
	}else{
		init_vm_property_value();
		config_vm_net_work();
	}

	CellsPrivateService::publishAndJoinThreadPool(true);

	ProcessState::self()->setThreadPoolMaxThreadCount(4);

	ALOGD("Cells service exit...");
	return 0;
}
