#include <cstdlib>
#include <unistd.h>
#include <fcntl.h>
#include <android/log.h>

#include "zygisk.hpp"

using zygisk::Api;
using zygisk::AppSpecializeArgs;
using zygisk::ServerSpecializeArgs;

#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG, "Zygisk", __VA_ARGS__)

class CustomModule : public zygisk::ModuleBase {
private:
    Api *api;
    JNIEnv *env;

public:
    CustomModule() : api(nullptr) , env(nullptr) {}

    void onLoad(Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }

    void preAppSpecialize(AppSpecializeArgs *args) override {
        // Use JNI to fetch our process name
        const char *process = env->GetStringUTFChars(args->nice_name, nullptr);
        preSpecializeHandler(process);
        env->ReleaseStringUTFChars(args->nice_name, process);
    }

    void preServerSpecialize(ServerSpecializeArgs *args) override {
        preSpecializeHandler("system_server");
    }


    void postAppSpecialize(const AppSpecializeArgs *args) override {

    }

    void postServerSpecialize(const ServerSpecializeArgs *args) override {

    }

    void preSpecializeHandler(const char *process) {
        // Demonstrate connecting to to companion process
        // We ask the companion for a random number
        unsigned r = 0;
        int fd = api->connectCompanion();
        read(fd, &r, sizeof(r));
        close(fd);
        LOGD("example: process=[%s], r=[%u]\n", process, r);

        // Since we do not hook any functions, we should let Zygisk dlclose ourselves
        api->setOption(zygisk::Option::DLCLOSE_MODULE_LIBRARY);
    }

};


//这个函数是为注入的动态库提供root服务，通信方式为套接字通信
static void companion_handler(int i) {
    LOGD("this root process companion , uid = %d",getuid());
    static int fd = -1;
    if (fd < 0) fd = open("/dev/urandom", O_RDONLY);
    unsigned r;
    read(fd, &r, sizeof(unsigned));
    LOGD("example: companion r=[%u]\n", r);
    write(i, &r, sizeof(r));
}

REGISTER_ZYGISK_MODULE(CustomModule)
REGISTER_ZYGISK_COMPANION(companion_handler)
