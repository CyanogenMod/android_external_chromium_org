/** ---------------------------------------------------------------------------
 Copyright (c) 2011-2014 The Linux Foundation. All rights reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:
     * Redistributions of source code must retain the above copyright
       notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
       copyright notice, this list of conditions and the following
       disclaimer in the documentation and/or other materials provided
       with the distribution.
     * Neither the name of The Linux Foundation nor the names of its
       contributors may be used to endorse or promote products derived
       from this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 -----------------------------------------------------------------------------**/
#include "base/files/file_path.h"
#include "net/libnetxt/dyn_lib_loader.h"
#include "net/libnetxt/libnetxt_base.h"
#include "net/libnetxt/plugin_api_ptr.h"

LibraryManager* LibraryManager::GetInstance() {
    CR_DEFINE_STATIC_LOCAL(LibraryManager, mgr, ());
    return &mgr;
}

LibraryManager::LibraryManager() {
}

LibraryManager::~LibraryManager() {
    ReleaseAll();
}

LibraryManager::MODULE_HANDLE_TYPE LibraryManager::GetLibraryHandleInternal(const std::string& libname) {
    char value[PROPERTY_VALUE_MAX] = {'\0'};
    std::string tmp_libname = libname;

    LIBNETXT_PROPERTY_GET("net.lmgr.enabled", value, "1");
    if (!atoi(value)) {
        LIBNETXT_LOGW("LIB_MGR - Disabled");
        return NULL;
    }

    LibHandle& handle = libdict[tmp_libname];
    if (NULL == handle) {
        //load module
        handle = LoadLibraryModule(tmp_libname);
    }

#ifdef LIBNETXT_CMB_PLUGIN_NAME
    if (NULL == handle) {
        if (LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGI("LIB_MGR - Unable to load %s, try %s", tmp_libname.c_str(), LIBNETXT_VAL_TO_STR(LIBNETXT_CMB_PLUGIN_NAME));
        }
        tmp_libname = LIBNETXT_VAL_TO_STR(LIBNETXT_CMB_PLUGIN_NAME);
        handle = libdict[tmp_libname];
        if (NULL == handle) {
            //load module
            handle = LoadLibraryModule(tmp_libname);
        }
    }
#endif

    if (NULL != handle) {
        bool (*init_plugin_api)(const char* version, LibnetxtPluginApi* plugin_api) = NULL;
        *(void **)(&init_plugin_api) = LibraryManager::GetLibrarySymbol(handle, "InitPluginApi", false);
        if(init_plugin_api) {
            if (!init_plugin_api(LIBNETXT_API_VERSION, LibnetxtPluginApi::GetInstance())) {
                LIBNETXT_LOGE("LIB_MGR - Unable to initialized PluginAPI of %s", tmp_libname.c_str());
                ReleaseLibraryHandle(tmp_libname.c_str());
                return NULL;
            }
            LIBNETXT_LOGI("LIB_MGR - PluginAPI initialized (%p)", LibnetxtPluginApi::GetInstance());
        }
        handle.IncRefCount();
        LIBNETXT_LOGI("LIB_MGR - Library loaded: %s", tmp_libname.c_str());
    }
    else {
        LIBNETXT_LOGE("LIB_MGR - Failed to load library %s (%s)", tmp_libname.c_str(), ::dlerror());
    }
    return handle;
}

int LibraryManager::ReleaseLibraryHandleInternal(const std::string& libname) {
    int refcount = 0;
    LibHandle& handle = libdict[libname];
    if (NULL != handle) {
        refcount = handle.DecRefCount();
        if (0 == refcount) {
            //release module
            ReleaseLibraryModule(handle);
            LIBNETXT_LOGI("LIB_MGR - Library %s unloaded", libname.c_str());
            libdict.erase(libname);
        }
    }
    return refcount;
}

void* LibraryManager::GetSymbolInternal(const std::string& libname,    const std::string& symbol, bool optional) {
    return LoadLibrarySymbol(GetLibraryHandleInternal(libname), symbol, optional);
}

void* LibraryManager::LoadLibrarySymbol(const MODULE_HANDLE_TYPE& lh, const std::string& symname, bool optional) {
    void* symptr = NULL;
    if (lh) {
        const char *error;

        ::dlerror(); //see man dlopen
        symptr = ::dlsym(lh, symname.c_str());
        if (NULL != (error = ::dlerror())) {
            symptr = NULL;
            if (!optional) {
                LIBNETXT_LOGE("LIB_MGR - Failed to load symbol %s (%s)", symname.c_str(), error);
            }
        }
    }
    return symptr;
}

LibraryManager::MODULE_HANDLE_TYPE LibraryManager::LoadLibraryModule(const std::string& libname) {
    std::string libpath, datapath, suffix;
    LibraryManager::MODULE_HANDLE_TYPE handle;
    char value[PROPERTY_VALUE_MAX] = {'\0'};

    //try user provided path
    LIBNETXT_PROPERTY_GET("net.lmgr.path", value, "");
    if (*value) {
        if (LIBNETXT_IS_VERBOSE) {
            LIBNETXT_LOGI("LIB_MGR - Try User Path (%s): %s", libname.c_str(), value);
        }
        libpath += value;
        libpath += "/";
        libpath += libname;
        libpath += ".so";
        handle = ::dlopen(libpath.c_str(), RTLD_NOW);
        if (handle) {
            return handle;
        }
    }

    //try to load from data directory
    GetLibDirectory(libpath);
    libpath += "/";
    if (LIBNETXT_IS_VERBOSE) {
        LIBNETXT_LOGI("LIB_MGR - Try LibDirectory Path (%s): %s", libname.c_str(), libpath.c_str());
    }
    libpath += libname;
    libpath += ".so";
    handle = ::dlopen(libpath.c_str(), RTLD_NOW);
    return handle;
}

void LibraryManager::ReleaseLibraryModule(const MODULE_HANDLE_TYPE& lh) {
    if (lh) {
        ::dlclose(lh);
    }
}

void LibraryManager::ReleaseAll() {
    for (LibDictionary::iterator it = libdict.begin(); it != libdict.end(); it++) {
        if (it->second) {
            ReleaseLibraryModule(it->second);
        }
    }
}

void LibraryManager::GetLibDirectory(std::string& path) {
    base::FilePath file_path;

    LIBNETXT_LIBDIR_GET(&file_path);
    file_path = file_path.StripTrailingSeparators();
    path = file_path.value();
}
