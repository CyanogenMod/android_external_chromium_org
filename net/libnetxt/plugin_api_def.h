/*
* Copyright (c) 2013-2014, The Linux Foundation. All rights reserved.
*
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are
* met:
*     * Redistributions of source code must retain the above copyright
*       notice, this list of conditions and the following disclaimer.
*     * Redistributions in binary form must reproduce the above
*       copyright notice, this list of conditions and the following
*       disclaimer in the documentation and/or other materials provided
*       with the distribution.
*     * Neither the name of The Linux Foundation nor the names of its
*       contributors may be used to endorse or promote products derived
*       from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
* WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
* ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
* BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
* WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
* OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
* IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#ifndef PLUGIN_API_DEF_H_
#define PLUGIN_API_DEF_H_

#if defined(LIBNETXT_API_BY_PROXY)
    #define LIBNETXT_API_NAME(prefix, name) Proxy##prefix##name
    #define LIBNETXT_API_CPP_NAME(prefix, namesp, type, name) Proxy##prefix##namesp##type##name
    #define LIBNETXT_API_CPP_CON_NAME(prefix, namesp, type) Proxy##prefix##namesp##type##constructor
    #define LIBNETXT_API_CPP_DES_NAME(prefix, namesp, type) Proxy##prefix##namesp##type##destructor
#elif defined(LIBNETXT_API_BY_PTR)
    #define LIBNETXT_API_NAME(prefix, name) LibnetxtPluginApi::GetInstance()->prefix##name
    #define LIBNETXT_API_CPP_NAME(prefix, namesp, type, name) LibnetxtPluginApi::GetInstance()->prefix##namesp##type##name
    #define LIBNETXT_API_CPP_CON_NAME(prefix, namesp, type) LibnetxtPluginApi::GetInstance()->prefix##namesp##type##constructor
    #define LIBNETXT_API_CPP_DES_NAME(prefix, namesp, type) LibnetxtPluginApi::GetInstance()->prefix##namesp##type##destructor
    #define LIBNETXT_API_CPP_DES_SP_NAME(prefix, namesp, type) LibnetxtPluginApi::GetInstance()->prefix##namesp##type##destructor
#elif defined(LIBNETXT_API_BY_IPC)
    #define LIBNETXT_API_NAME(prefix, name) Ipc##prefix##name
    #define LIBNETXT_API_CPP_NAME(prefix, namesp, type, name) Ipc##prefix##namesp##type##name
    #define LIBNETXT_API_CPP_CON_NAME(prefix, namesp, type) Ipc##prefix##namesp##type##constructor
    #define LIBNETXT_API_CPP_DES_NAME(prefix, namesp, type) Ipc##prefix##namesp##type##destructor
#else
    #define LIBNETXT_API_NAME(prefix, name) prefix##name
    #define LIBNETXT_API_CPP_NAME(prefix, namesp, type, name) prefix##namesp##type##name
    #define LIBNETXT_API_CPP_CON_NAME(prefix, namesp, type) prefix##namesp##type##constructor
    #define LIBNETXT_API_CPP_DES_NAME(prefix, namesp, type) prefix##namesp##type##destructor
#endif

// ================================ LibNetXt global ====================================
#define LIBNETXT_API_PREFIX LibNetXt

#define LIBNETXT_API(name) \
    LIBNETXT_API_NAME(LibNetXt, name)
#define LIBNETXT_API_CPP(namesp, type, name) \
    LIBNETXT_API_CPP_NAME(LibNetXt, namesp, type, name)
#define LIBNETXT_API_CPP_CON(namesp, type) \
    LIBNETXT_API_CPP_CON_NAME(LibNetXt, namesp, type)
#define LIBNETXT_API_CPP_SP_CON(namesp, type) \
    LibnetxtPluginApi::GetInstance()->LibNetXtscoped_refptr_##namesp##type##constructor
#define LIBNETXT_API_CPP_SP_DES(namesp, type) \
    LibnetxtPluginApi::GetInstance()->LibNetXtscoped_refptr_##namesp##type##destructor
#define LIBNETXT_API_CPP_DES(namesp, type) \
    LIBNETXT_API_CPP_DES_NAME(LibNetXt, namesp, type)

// ================================ "C" interface ====================================
#define LIBNETXT_API_DECL_0(prefix, name, ret)  LIBNETXT_API_DEF_0(prefix, name, ret)
#define LIBNETXT_API_DEF_0(prefix, name, ret) \
    extern ret prefix##name() \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name() \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name() \
        __attribute__ ((visibility ("default")));

#define LIBNETXT_API_DEF_1(prefix, name, ret, type1) \
    extern ret prefix##name(type1 param1) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name(type1 param1) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name(type1 param1) \
        __attribute__ ((visibility ("default"))); \

#define LIBNETXT_API_DEF_2(prefix, name, ret, type1, type2) \
    extern ret prefix##name(type1 param1, type2 param2) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name(type1 param1, type2 param2) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name(type1 param1, type2 param2) \
        __attribute__ ((visibility ("default")));

#define LIBNETXT_API_DEF_3(prefix, name, ret, type1, type2, type3) \
    extern ret prefix##name(type1 param1, type2 param2, type3 param3) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3) \
        __attribute__ ((visibility ("default"), used));

#define LIBNETXT_API_DEF_4(prefix, name, ret, type1, type2, type3, type4) \
    extern ret prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        __attribute__ ((visibility ("default"), used));

#define LIBNETXT_API_DEF_5(prefix, name, ret, type1, type2, type3, type4, type5) \
    extern ret prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
    __attribute__ ((visibility ("default"), used)); \
    extern ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
    __attribute__ ((visibility ("default"), used)); \
    extern ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
    __attribute__ ((visibility ("default"), used));

// ================================ "CPP" interface ====================================
#define LIBNETXT_API_CPP_DEF_0(prefix, namesp, type, name, ret) \
    LIBNETXT_API_DEF_1(prefix, namesp##type##name, ret, namesp::type*)

#define LIBNETXT_API_CPP_DEF_1(prefix, namesp, type, name, ret, type1) \
    LIBNETXT_API_DEF_2(prefix, namesp##type##name, ret, namesp::type*, type1)

#define LIBNETXT_API_CPP_DEF_2(prefix, namesp, type, name, ret, type1, type2) \
    LIBNETXT_API_DEF_3(prefix, namesp##type##name, ret, namesp::type*, type1, type2)

#define LIBNETXT_API_CPP_DEF_3(prefix, namesp, type, name, ret, type1, type2, type3) \
    LIBNETXT_API_DEF_4(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3)

#define LIBNETXT_API_CPP_DEF_4(prefix, namesp, type, name, ret, type1, type2, type3, type4) \
    LIBNETXT_API_DEF_5(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3, type4)

#define LIBNETXT_API_CPP_FORWARDER_0(prefix, namesp, type, name, ret) \
    ret prefix##namesp##type##name(namesp::type* this_ptr) {return this_ptr->name();}
#define LIBNETXT_API_CPP_FORWARDER_0V(prefix, namesp, type, name, ret) \
    ret prefix##namesp##type##name(namesp::type* this_ptr) {this_ptr->name();}

#define LIBNETXT_API_CPP_FORWARDER_1(prefix, namesp, type, name, ret, type1) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1) \
        {return this_ptr->name(param1);}
#define LIBNETXT_API_CPP_FORWARDER_1V(prefix, namesp, type, name, ret, type1) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1) \
        {this_ptr->name(param1);}

#define LIBNETXT_API_CPP_FORWARDER_2(prefix, namesp, type, name, ret, type1, type2) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2) \
        {return this_ptr->name(param1, param2);}
#define LIBNETXT_API_CPP_FORWARDER_2V(prefix, namesp, type, name, ret, type1, type2) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2) \
        {this_ptr->name(param1, param2);}

#define LIBNETXT_API_CPP_FORWARDER_3(prefix, namesp, type, name, ret, type1, type2, type3) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2, type3 param3) \
        {return this_ptr->name(param1, param2, param3);}
#define LIBNETXT_API_CPP_FORWARDER_3V(prefix, namesp, type, name, ret, type1, type2, type3) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2, type3 param3) \
        {this_ptr->name(param1, param2, param3);}

#define LIBNETXT_API_CPP_FORWARDER_4(prefix, namesp, type, name, ret, type1, type2, type3, type4) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2, type3 param3, type4 param4) \
        {return this_ptr->name(param1, param2, param3, param4);}

#define LIBNETXT_API_CPP_FORWARDER_4V(prefix, namesp, type, name, ret, type1, type2, type3, type4) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2, type3 param3, type4 param4) \
        {this_ptr->name(param1, param2, param3, param4);}

#define LIBNETXT_API_CPP_FORWARDER_5(prefix, namesp, type, name, ret, type1, type2, type3, type4, type5) \
    ret prefix##namesp##type##name(namesp::type* this_ptr, type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
        {return this_ptr->name(param1, param2, param3, param4, param5);}

#define LIBNETXT_API_CPP_DEF_CON_0(prefix, namesp, type) \
    LIBNETXT_API_DEF_0(prefix, namesp##type##constructor, namesp::type*)
#define LIBNETXT_API_CPP_DEF_CON_1(prefix, namesp, type, type1) \
    LIBNETXT_API_DEF_1(prefix, namesp##type##constructor, namesp::type*, type1)
#define LIBNETXT_API_CPP_DEF_CON_2(prefix, namesp, type, type1, type2) \
    LIBNETXT_API_DEF_2(prefix, namesp##type##constructor, namesp::type*, type1, type2)
#define LIBNETXT_API_CPP_DEF_CON_3(prefix, namesp, type, type1, type2, type3) \
    LIBNETXT_API_DEF_3(prefix, namesp##type##constructor, namesp::type*, type1, type2, type3)

#define LIBNETXT_API_CPP_DEF_DES(prefix, namesp, type) \
    LIBNETXT_API_DEF_1(prefix, namesp##type##destructor, void, namesp::type*)

#define LIBNETXT_API_CPP_FORWARDER_CON_0(prefix, namesp, type) \
    namesp::type* prefix##namesp##type##constructor() {return new namesp::type;}
#define LIBNETXT_API_CPP_FORWARDER_CON_1(prefix, namesp, type, type1) \
    namesp::type* prefix##namesp##type##constructor(type1 param1) {return new namesp::type(param1);}
#define LIBNETXT_API_CPP_FORWARDER_CON_2(prefix, namesp, type, type1, type2) \
    namesp::type* prefix##namesp##type##constructor(type1 param1, type2 param2) {return new namesp::type(param1, param2);}
#define LIBNETXT_API_CPP_FORWARDER_CON_3(prefix, namesp, type, type1, type2, type3) \
    namesp::type* prefix##namesp##type##constructor(type1 param1, type2 param2, type3 param3) {return new namesp::type(param1, param2, param3);}

#define LIBNETXT_API_CPP_FORWARDER_DES(prefix, namesp, type) \
    void prefix##namesp##type##destructor(namesp::type* param1) {delete param1;}

// ================================ Proxy interface ====================================
//TODO:    static ret name##_Proxy(type1 param1, type2 param2, type3 param3)
//TODO:        __attribute__ ((weakref ("Proxy"#name), used));

#define LIBNETXT_API_PROXY_IMP_0(prefix, name, ret) \
    ret Proxy##prefix##name() {return prefix##name();}
#define LIBNETXT_API_PROXY_IMP_0V(prefix, name, ret) \
    ret Proxy##prefix##name() {prefix##name();}

#define LIBNETXT_API_PROXY_IMP_1(prefix, name, ret, type1) \
    ret Proxy##prefix##name(type1 param1) {return prefix##name(param1);}
#define LIBNETXT_API_PROXY_IMP_1V(prefix, name, ret, type1) \
    ret Proxy##prefix##name(type1 param1) {prefix##name(param1);}

#define LIBNETXT_API_PROXY_IMP_2(prefix, name, ret, type1, type2) \
    ret Proxy##prefix##name(type1 param1, type2 param2) \
        {return prefix##name(param1, param2);}
#define LIBNETXT_API_PROXY_IMP_2V(prefix, name, ret, type1, type2) \
    ret Proxy##prefix##name(type1 param1, type2 param2) \
        {prefix##name(param1, param2);}

#define LIBNETXT_API_PROXY_IMP_3(prefix, name, ret, type1, type2, type3) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3) \
        {return prefix##name(param1, param2, param3);}
#define LIBNETXT_API_PROXY_IMP_3V(prefix, name, ret, type1, type2, type3) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3) \
        {prefix##name(param1, param2, param3);}

#define LIBNETXT_API_PROXY_IMP_4(prefix, name, ret, type1, type2, type3, type4) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        {return prefix##name(param1, param2, param3, param4);}
#define LIBNETXT_API_PROXY_IMP_4V(prefix, name, ret, type1, type2, type3, type4) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        {prefix##name(param1, param2, param3, param4);}

#define LIBNETXT_API_PROXY_IMP_5(prefix, name, ret, type1, type2, type3, type4, type5) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
        {return prefix##name(param1, param2, param3, param4, param5);}
#define LIBNETXT_API_PROXY_IMP_5V(prefix, name, ret, type1, type2, type3, type4, type5) \
    ret Proxy##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
        {prefix##name(param1, param2, param3, param4, param5);}

#define LIBNETXT_API_CPP_PROXY_IMP_0(prefix, namesp, type, name, ret) \
    LIBNETXT_API_PROXY_IMP_1(prefix, namesp##type##name, ret, namesp::type*)
#define LIBNETXT_API_CPP_PROXY_IMP_0V(prefix, namesp, type, name, ret) \
    LIBNETXT_API_PROXY_IMP_1V(prefix, namesp##type##name, ret, namesp::type*)

#define LIBNETXT_API_CPP_PROXY_IMP_1(prefix, namesp, type, name, ret, type1) \
    LIBNETXT_API_PROXY_IMP_2(prefix, namesp##type##name, ret, namesp::type*, type1)
#define LIBNETXT_API_CPP_PROXY_IMP_1V(prefix, namesp, type, name, ret, type1) \
    LIBNETXT_API_PROXY_IMP_2V(prefix, namesp##type##name, ret, namesp::type*, type1)

#define LIBNETXT_API_CPP_PROXY_IMP_2(prefix, namesp, type, name, ret, type1, type2) \
    LIBNETXT_API_PROXY_IMP_3(prefix, namesp##type##name, ret, namesp::type*, type1, type2)
#define LIBNETXT_API_CPP_PROXY_IMP_2V(prefix, namesp, type, name, ret, type1, type2) \
    LIBNETXT_API_PROXY_IMP_3V(prefix, namesp##type##name, ret, namesp::type*, type1, type2)

#define LIBNETXT_API_CPP_PROXY_IMP_3(prefix, namesp, type, name, ret, type1, type2, type3) \
    LIBNETXT_API_PROXY_IMP_4(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3)

#define LIBNETXT_API_CPP_PROXY_IMP_CON_0(prefix, namesp, type) \
    LIBNETXT_API_PROXY_IMP_0(prefix, namesp##type##constructor, namesp::type*)
#define LIBNETXT_API_CPP_PROXY_IMP_CON_1(prefix, namesp, type, type1) \
    LIBNETXT_API_PROXY_IMP_1(prefix, namesp##type##constructor, namesp::type*, type1)
#define LIBNETXT_API_CPP_PROXY_IMP_DES(prefix, namesp, type) \
    LIBNETXT_API_PROXY_IMP_1V(prefix, namesp##type##destructor, void, namesp::type*)

// ================================ IPC interface ====================================
#define LIBNETXT_API_IPC_FORWARDER_0(prefix, name, ret) \
    ret Ipc##prefix##name() {return prefix##name();}

#define LIBNETXT_API_IPC_FORWARDER_1(prefix, name, ret, type1) \
    ret Ipc##prefix##name(type1 param1) {return prefix##name(param1);}

#define LIBNETXT_API_IPC_FORWARDER_2(prefix, name, ret, type1, type2) \
    ret Ipc##prefix##name(type1 param1, type2 param2) \
        {return prefix##name(param1, param2);}

#define LIBNETXT_API_IPC_FORWARDER_3(prefix, name, ret, type1, type2, type3) \
    ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3) \
        {return prefix##name(param1, param2, param3);}

#define LIBNETXT_API_IPC_FORWARDER_4(prefix, name, ret, type1, type2, type3, type4) \
    ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4) \
        {return prefix##name(param1, param2, param3, param4);}

#define LIBNETXT_API_IPC_FORWARDER_5(prefix, name, ret, type1, type2, type3, type4, type5) \
    ret Ipc##prefix##name(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5) \
        {return prefix##name(param1, param2, param3, param4, param5);}

// ================================ PTR interface ====================================
#define LIBNETXT_API_PTR_IMP(api, prefix, name) \
    api->prefix##name = prefix##name;
#define LIBNETXT_API_CPP_PTR_IMP(api, prefix, namesp, type, name) \
    api->prefix##namesp##type##name = prefix##namesp##type##name;

#define LIBNETXT_API_CPP_PTR_DEF_CON_0(prefix, namesp, type) \
    LIBNETXT_API_PTR_DEF_0(prefix, namesp##type##constructor, namesp::type*)
#define LIBNETXT_API_CPP_PTR_DEF_SP_CON_0(prefix, namesp, type) \
        LIBNETXT_API_PTR_DEF_0(prefix, scoped_refptr_##namesp##type##constructor, scoped_refptr< namesp :: type >*)
#define LIBNETXT_API_CPP_PTR_DEF_SP_DES(prefix, namesp, type) \
        LIBNETXT_API_PTR_DEF_1(prefix, scoped_refptr_##namesp##type##destructor, void, scoped_refptr< namesp :: type >*)


#define LIBNETXT_API_CPP_PTR_DEF_CON_1(prefix, namesp, type, type1) \
    LIBNETXT_API_PTR_DEF_1(prefix, namesp##type##constructor, namesp::type*, type1)
#define LIBNETXT_API_CPP_PTR_DEF_CON_2(prefix, namesp, type, type1, type2) \
    LIBNETXT_API_PTR_DEF_2(prefix, namesp##type##constructor, namesp::type*, type1, type2)
#define LIBNETXT_API_CPP_PTR_DEF_CON_3(prefix, namesp, type, type1, type2, type3) \
    LIBNETXT_API_PTR_DEF_3(prefix, namesp##type##constructor, namesp::type*, type1, type2, type3)

#define LIBNETXT_API_CPP_PTR_DEF_DES(prefix, namesp, type) \
    LIBNETXT_API_PTR_DEF_1(prefix, namesp##type##destructor, void, namesp::type*)

#define LIBNETXT_API_CPP_PTR_IMP_CON(api, prefix, namesp, type) \
    LIBNETXT_API_CPP_PTR_IMP(api, prefix, namesp, type, constructor)

#define LIBNETXT_API_CPP_PTR_IMP_SP_CON(api, prefix, namesp, type) \
    LIBNETXT_API_CPP_PTR_IMP(api, prefix, scoped_refptr_##namesp, type, constructor)
#define LIBNETXT_API_CPP_PTR_IMP_SP_DES(api, prefix, namesp, type) \
    LIBNETXT_API_CPP_PTR_IMP(api, prefix, scoped_refptr_##namesp, type, destructor)

#define LIBNETXT_API_CPP_PTR_IMP_DES(api, prefix, namesp, type) \
    LIBNETXT_API_CPP_PTR_IMP(api, prefix, namesp, type, destructor)

#define LIBNETXT_API_PTR_DEF_0(prefix, name, ret) \
    typedef ret (*prefix##name##FunPtr)(); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_0(prefix, namesp, type, name, ret) \
    LIBNETXT_API_PTR_DEF_1(prefix, namesp##type##name, ret, namesp::type*)

#define LIBNETXT_API_PTR_DEF_1(prefix, name, ret, type1) \
    typedef ret (*prefix##name##FunPtr)(type1 param1); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_1(prefix, namesp, type, name, ret, type1) \
    LIBNETXT_API_PTR_DEF_2(prefix, namesp##type##name, ret, namesp::type*, type1)

#define LIBNETXT_API_CPP_PTR_DEF_Const_1(prefix, namesp, type, name, ret, type1) \
    LIBNETXT_API_PTR_DEF_2(prefix, namesp##type##name, ret, const namesp::type*, type1)

#define LIBNETXT_API_PTR_DEF_2(prefix, name, ret, type1, type2) \
    typedef ret (*prefix##name##FunPtr)(type1 param1, type2 param2); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_2(prefix, namesp, type, name, ret, type1, type2) \
    LIBNETXT_API_PTR_DEF_3(prefix, namesp##type##name, ret, namesp::type*, type1, type2)

#define LIBNETXT_API_CPP_PTR_DEF_Const_2(prefix, namesp, type, name, ret, type1, type2) \
    LIBNETXT_API_PTR_DEF_3(prefix, namesp##type##name, ret, const namesp::type*, type1, type2)
#define LIBNETXT_API_PTR_DEF_3(prefix, name, ret, type1, type2, type3) \
    typedef ret (*prefix##name##FunPtr)(type1 param1, type2 param2, type3 param3); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_3(prefix, namesp, type, name, ret, type1, type2, type3) \
    LIBNETXT_API_PTR_DEF_4(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3)

#define LIBNETXT_API_PTR_DEF_4(prefix, name, ret, type1, type2, type3, type4) \
    typedef ret (*prefix##name##FunPtr)(type1 param1, type2 param2, type3 param3, type4 param4); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_4(prefix, namesp, type, name, ret, type1, type2, type3, type4) \
    LIBNETXT_API_PTR_DEF_5(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3, type4)

#define LIBNETXT_API_PTR_DEF_5(prefix, name, ret, type1, type2, type3, type4, type5) \
    typedef ret (*prefix##name##FunPtr)(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5); \
    prefix##name##FunPtr prefix##name;
#define LIBNETXT_API_CPP_PTR_DEF_5(prefix, namesp, type, name, ret, type1, type2, type3, type4, type5) \
    LIBNETXT_API_PTR_DEF_6(prefix, namesp##type##name, ret, namesp::type*, type1, type2, type3, type4, type5)

#define LIBNETXT_API_PTR_DEF_6(prefix, name, ret, type1, type2, type3, type4, type5, type6) \
    typedef ret (*prefix##name##FunPtr)(type1 param1, type2 param2, type3 param3, type4 param4, type5 param5, type6 param6); \
    prefix##name##FunPtr prefix##name;

#endif /* PLUGIN_API_DEF_H_ */
