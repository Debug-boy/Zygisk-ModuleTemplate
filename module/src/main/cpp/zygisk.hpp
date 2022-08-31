// This is the public API for Zygisk modules.
// DO NOT MODIFY ANY CODE IN THIS HEADER.

#pragma once

#include <jni.h>

#define ZYGISK_API_VERSION 2

/*

Define a class and inherit zygisk::ModuleBase to implement the functionality of your module.
Use the macro REGISTER_ZYGISK_MODULE(className) to register that class to Zygisk.

Please note that modules will only be loaded after zygote has forked the child process.
THIS MEANS ALL OF YOUR CODE RUNS IN THE APP/SYSTEM SERVER PROCESS, NOT THE ZYGOTE DAEMON!

Example code:

static jint (*orig_logger_entry_max)(JNIEnv *env);
static jint my_logger_entry_max(JNIEnv *env) { return orig_logger_entry_max(env); }

static void example_handler(int socket) { ... }

class ExampleModule : public zygisk::ModuleBase {
public:
    void onLoad(zygisk::Api *api, JNIEnv *env) override {
        this->api = api;
        this->env = env;
    }
    void preAppSpecialize(zygisk::AppSpecializeArgs *args) override {
        JNINativeMethod methods[] = {
            { "logger_entry_max_payload_native", "()I", (void*) my_logger_entry_max },
        };
        api->hookJniNativeMethods(env, "android/util/Log", methods, 1);
        *(void **) &orig_logger_entry_max = methods[0].fnPtr;
    }
private:
    zygisk::Api *api;
    JNIEnv *env;
};

REGISTER_ZYGISK_MODULE(ExampleModule)

REGISTER_ZYGISK_COMPANION(example_handler)

*/

namespace zygisk {

struct Api;
struct AppSpecializeArgs;
struct ServerSpecializeArgs;

class ModuleBase {
public:

    // This function is called when the module is loaded into the target process.
    // A Zygisk API handle will be sent as an argument; call utility functions or interface
    // with Zygisk through this handle.
    // 当模块加载到目标进程中时，将调用此函数。
    // Zygisk API句柄将作为参数发送；调用实用程序函数或接口
    // 通过这个把手和Zygisk在一起。
    virtual void onLoad([[maybe_unused]] Api *api, [[maybe_unused]] JNIEnv *env) {}

    // This function is called before the app process is specialized.
    // At this point, the process just got forked from zygote, but no app specific specialization
    // is applied. This means that the process does not have any sandbox restrictions and
    // still runs with the same privilege of zygote.
    //
    // All the arguments that will be sent and used for app specialization is passed as a single
    // AppSpecializeArgs object. You can read and overwrite these arguments to change how the app
    // process will be specialized.
    //
    // If you need to run some operations as superuser, you can call Api::connectCompanion() to
    // get a socket to do IPC calls with a root companion process.
    // See Api::connectCompanion() for more info.
    // 在专用化应用程序进程之前调用此函数。
    // 在这一点上，这个过程只是从受精卵分叉出来的，但没有特定于应用程序的专门化
    // 应用。这意味着该进程没有任何沙箱限制，并且
    // 仍然以相同的受精卵特权运行。
    //
    // 将发送并用于应用程序专门化的所有参数都作为单个参数传递
    // AppSpecializeArgs对象。您可以读取并覆盖这些参数以更改应用程序的方式
    // 过程将是专门化的。
    //
    // 如果需要以超级用户身份运行某些操作，可以调用Api:：connectCompanion（）来
    // 获取一个套接字，用根伴随进程进行IPC调用。
    // 有关详细信息，请参见Api:：connectCompanion（）。
    virtual void preAppSpecialize([[maybe_unused]] AppSpecializeArgs *args) {}

    // This function is called after the app process is specialized.
    // At this point, the process has all sandbox restrictions enabled for this application.
    // This means that this function runs as the same privilege of the app's own code.
    // 此函数在应用程序进程专用化后调用。
    // 此时，流程已为此应用程序启用了所有沙盒限制。
    // 这意味着该函数与应用程序自身代码的权限相同。
    virtual void postAppSpecialize([[maybe_unused]] const AppSpecializeArgs *args) {}

    // This function is called before the system server process is specialized.
    // See preAppSpecialize(args) for more info.
    // 在系统服务器进程专用化之前调用此函数。
    // 有关详细信息，请参阅预应用程序专用化（参数）。
    virtual void preServerSpecialize([[maybe_unused]] ServerSpecializeArgs *args) {}

    // This function is called after the system server process is specialized.
    // At this point, the process runs with the privilege of system_server.
    // 此函数在系统服务器进程专用化后调用。
    // 此时，进程以system_server权限运行。
    virtual void postServerSpecialize([[maybe_unused]] const ServerSpecializeArgs *args) {}
};

struct AppSpecializeArgs {
    // Required arguments. These arguments are guaranteed to exist on all Android versions.
    // 必需的参数。这些参数保证在所有Android版本上都存在。
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jint &mount_external;
    jstring &se_info;
    jstring &nice_name;
    jstring &instruction_set;
    jstring &app_data_dir;

    // Optional arguments. Please check whether the pointer is null before de-referencing
    // 可选参数。请在取消引用之前检查指针是否为空
    jboolean *const is_child_zygote;
    jboolean *const is_top_app;
    jobjectArray *const pkg_data_info_list;
    jobjectArray *const whitelisted_data_info_list;
    jboolean *const mount_data_dirs;
    jboolean *const mount_storage_dirs;

    AppSpecializeArgs() = delete;
};

struct ServerSpecializeArgs {
    jint &uid;
    jint &gid;
    jintArray &gids;
    jint &runtime_flags;
    jlong &permitted_capabilities;
    jlong &effective_capabilities;

    ServerSpecializeArgs() = delete;
};

namespace internal {
struct api_table;
template <class T> void entry_impl(api_table *, JNIEnv *);
}

// These values are used in Api::setOption(Option)
enum Option : int {
    // Force Magisk's denylist unmount routines to run on this process.
    //
    // Setting this option only makes sense in preAppSpecialize.
    // The actual unmounting happens during app process specialization.
    //
    // Set this option to force all Magisk and modules' files to be unmounted from the
    // mount namespace of the process, regardless of the denylist enforcement status.
    // 强制Magisk的denylist卸载例程在此进程上运行。
    //
    // 设置此选项仅在preAppSpecialize中有意义。
    // 实际卸载发生在应用程序进程专门化期间。
    //
    // 设置此选项可强制从中卸载所有Magisk和模块文件
    // 装入进程的命名空间，而不管denylist强制状态如何。
    FORCE_DENYLIST_UNMOUNT = 0,

    // When this option is set, your module's library will be dlclose-ed after post[XXX]Specialize.
    // Be aware that after dlclose-ing your module, all of your code will be unmapped from memory.
    // YOU MUST NOT ENABLE THIS OPTION AFTER HOOKING ANY FUNCTIONS IN THE PROCESS.
    // 设置此选项后，post[XXX]专门化后，您的模块库将被关闭。
    // 请注意，关闭模块后，所有代码都将从内存中取消映射。
    // 挂钩(HOOK)进程中的任何函数后，不得启用此选项。
    DLCLOSE_MODULE_LIBRARY = 1,
};

// Bit masks of the return value of Api::getFlags()
enum StateFlag : uint32_t {
    // The user has granted root access to the current process
    PROCESS_GRANTED_ROOT = (1u << 0),

    // The current process was added on the denylist
    PROCESS_ON_DENYLIST = (1u << 1),
};

// All API functions will stop working after post[XXX]Specialize as Zygisk will be unloaded
// from the specialized process afterwards.
struct Api {

    // Connect to a root companion process and get a Unix domain socket for IPC.
    //
    // This API only works in the pre[XXX]Specialize functions due to SELinux restrictions.
    //
    // The pre[XXX]Specialize functions run with the same privilege of zygote.
    // If you would like to do some operations with superuser permissions, register a handler
    // function that would be called in the root process with REGISTER_ZYGISK_COMPANION(func).
    // Another good use case for a companion process is that if you want to share some resources
    // across multiple processes, hold the resources in the companion process and pass it over.
    //
    // The root companion process is ABI aware; that is, when calling this function from a 32-bit
    // process, you will be connected to a 32-bit companion process, and vice versa for 64-bit.
    //
    // Returns a file descriptor to a socket that is connected to the socket passed to your
    // module's companion request handler. Returns -1 if the connection attempt failed.
    //连接到根伙伴进程并获取IPC的Unix域套接字。
    //
    //由于SELinux限制，此API仅适用于pre[XXX]Specialize函数。
    //
    //pre[XXX]专门化函数以与合子相同的权限运行。
    //如果要使用超级用户权限执行某些操作，请注册处理程序
    //将在根进程中使用REGISTER_ZYGISK_COMPANION（func）调用的函数。
    //伙伴流程的另一个好用例是，如果您想共享一些资源
    //跨多个进程，将资源保存在伴随进程中并将其传递。
    //
    //根伴随进程是ABI感知的；也就是说，当从32位调用此函数时
    //进程，您将连接到32位伴随进程，反之亦然。
    //
    //将文件描述符返回到连接到传递给的套接字的套接字
    //模块的伴随请求处理程序。如果连接尝试失败，则返回-1。
    int connectCompanion();

    // Get the file descriptor of the root folder of the current module.
    //
    // This API only works in the pre[XXX]Specialize functions.
    // Accessing the directory returned is only possible in the pre[XXX]Specialize functions
    // or in the root companion process (assuming that you sent the fd over the socket).
    // Both restrictions are due to SELinux and UID.
    //
    // Returns -1 if errors occurred.
    //获取当前模块根文件夹的文件描述符。
    //
    //此API仅适用于pre[XXX]Specialize函数。
    //只有在pre[XXX]Specialize函数中才能访问返回的目录
    //或者在根伴随进程中（假设您通过套接字发送了fd）。
    //这两种限制都是由于SELinux和UID造成的。
    //
    //如果发生错误，则返回-1。
    int getModuleDir();

    // Set various options for your module.
    // Please note that this function accepts one single option at a time.
    // Check zygisk::Option for the full list of options available.
    //为您的模块设置各种选项。
    //请注意，此函数一次只接受一个选项。
    // 有关可用选项的完整列表，请查看zygisk:：Option。
    void setOption(Option opt);

    // Get information about the current process.
    // Returns bitwise-or'd zygisk::StateFlag values.
    //获取有关当前进程的信息。返回按位或d的zygisk:：StateFlag值。
    uint32_t getFlags();

    // Hook JNI native methods for a class
    //
    // Lookup all registered JNI native methods and replace it with your own functions.
    // The original function pointer will be saved in each JNINativeMethod's fnPtr.
    // If no matching class, method name, or signature is found, that specific JNINativeMethod.fnPtr
    // will be set to nullptr.
    //钩子类的JNI本机方法查找所有注册的JNI本机方法，并用您自己的函数替换它。
    //原始函数指针将保存在每个JnativeMethod的fnPtr中。
    //如果找不到匹配的类、方法名或签名，则为该特定的JnativeMethod。
    //fnPtr将设置为nullptr。
    void hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods);

    // For ELFs loaded in memory matching `regex`, replace function `symbol` with `newFunc`.
    // If `oldFunc` is not nullptr, the original function pointer will be saved to `oldFunc`.
    //对于在内存中加载的与“regex”匹配的ELF，将函数“symbol”替换为“newFunc”。
    //如果“oldFunc”不是null ptr，则原始函数指针将保存到“oldfnc”。
    void pltHookRegister(const char *regex, const char *symbol, void *newFunc, void **oldFunc);

    // For ELFs loaded in memory matching `regex`, exclude hooks registered for `symbol`.
    // If `symbol` is nullptr, then all symbols will be excluded.
    //对于在内存中加载的与“regex”匹配的ELF，排除为“symbol”注册的钩子。
    //如果“符号”为nullptr，则将排除所有符号。
    void pltHookExclude(const char *regex, const char *symbol);

    // Commit all the hooks that was previously registered.
    // Returns false if an error occurred.
    //提交以前注册的所有钩子。
    //如果发生错误，则返回false。
    bool pltHookCommit();

private:
    internal::api_table *impl;
    template <class T> friend void internal::entry_impl(internal::api_table *, JNIEnv *);
};

// Register a class as a Zygisk module

#define REGISTER_ZYGISK_MODULE(clazz) \
void zygisk_module_entry(zygisk::internal::api_table *table, JNIEnv *env) { \
    zygisk::internal::entry_impl<clazz>(table, env);                        \
}

// Register a root companion request handler function for your module
//
// The function runs in a superuser daemon process and handles a root companion request from
// your module running in a target process. The function has to accept an integer value,
// which is a socket that is connected to the target process.
// See Api::connectCompanion() for more info.
//
// NOTE: the function can run concurrently on multiple threads.
// Be aware of race conditions if you have a globally shared resource.
// 为您的模块注册根伴随请求处理程序函数
//
// 该函数在超级用户守护进程中运行，并处理来自
// 您的模块正在目标进程中运行。函数必须接受整数值，
// 它是连接到目标进程的套接字。
// 有关详细信息，请参见Api:：connectCompanion（）。
//
// 注意：该函数可以在多个线程上同时运行。
// 如果您拥有全局共享资源，请注意竞赛条件。
#define REGISTER_ZYGISK_COMPANION(func) \
void zygisk_companion_entry(int client) { func(client); }

/************************************************************************************
 * All the code after this point is internal code used to interface with Zygisk
 * and guarantee ABI stability. You do not have to understand what it is doing.
 ************************************************************************************/

namespace internal {

struct module_abi {
    long api_version;
    ModuleBase *_this;

    void (*preAppSpecialize)(ModuleBase *, AppSpecializeArgs *);
    void (*postAppSpecialize)(ModuleBase *, const AppSpecializeArgs *);
    void (*preServerSpecialize)(ModuleBase *, ServerSpecializeArgs *);
    void (*postServerSpecialize)(ModuleBase *, const ServerSpecializeArgs *);

    module_abi(ModuleBase *module) : api_version(ZYGISK_API_VERSION), _this(module) {
        preAppSpecialize = [](auto self, auto args) { self->preAppSpecialize(args); };
        postAppSpecialize = [](auto self, auto args) { self->postAppSpecialize(args); };
        preServerSpecialize = [](auto self, auto args) { self->preServerSpecialize(args); };
        postServerSpecialize = [](auto self, auto args) { self->postServerSpecialize(args); };
    }
};

struct api_table {
    // These first 2 entries are permanent, shall never change
    void *_this;
    bool (*registerModule)(api_table *, module_abi *);

    // Utility functions
    void (*hookJniNativeMethods)(JNIEnv *, const char *, JNINativeMethod *, int);
    void (*pltHookRegister)(const char *, const char *, void *, void **);
    void (*pltHookExclude)(const char *, const char *);
    bool (*pltHookCommit)();

    // Zygisk functions
    int  (*connectCompanion)(void * /* _this */);
    void (*setOption)(void * /* _this */, Option);
    int  (*getModuleDir)(void * /* _this */);
    uint32_t (*getFlags)(void * /* _this */);
};

template <class T>
void entry_impl(api_table *table, JNIEnv *env) {
    ModuleBase *module = new T();
    if (!table->registerModule(table, new module_abi(module)))
        return;
    auto api = new Api();
    api->impl = table;
    module->onLoad(api, env);
}

} // namespace internal

inline int Api::connectCompanion() {
    return impl->connectCompanion ? impl->connectCompanion(impl->_this) : -1;
}
inline int Api::getModuleDir() {
    return impl->getModuleDir ? impl->getModuleDir(impl->_this) : -1;
}
inline void Api::setOption(Option opt) {
    if (impl->setOption) impl->setOption(impl->_this, opt);
}
inline uint32_t Api::getFlags() {
    return impl->getFlags ? impl->getFlags(impl->_this) : 0;
}
inline void Api::hookJniNativeMethods(JNIEnv *env, const char *className, JNINativeMethod *methods, int numMethods) {
    if (impl->hookJniNativeMethods) impl->hookJniNativeMethods(env, className, methods, numMethods);
}
inline void Api::pltHookRegister(const char *regex, const char *symbol, void *newFunc, void **oldFunc) {
    if (impl->pltHookRegister) impl->pltHookRegister(regex, symbol, newFunc, oldFunc);
}
inline void Api::pltHookExclude(const char *regex, const char *symbol) {
    if (impl->pltHookExclude) impl->pltHookExclude(regex, symbol);
}
inline bool Api::pltHookCommit() {
    return impl->pltHookCommit != nullptr && impl->pltHookCommit();
}

} // namespace zygisk

[[gnu::visibility("default")]] [[gnu::used]]
extern "C" void zygisk_module_entry(zygisk::internal::api_table *, JNIEnv *);

[[gnu::visibility("default")]] [[gnu::used]]
extern "C" void zygisk_companion_entry(int);
