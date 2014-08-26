/**
 * Copyright (c) 2012, 2013, The Linux Foundation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *   * Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above
 *     copyright notice, this list of conditions and the following
 *     disclaimer in the documentation and/or other *materials provided
 *     with the distribution.
 *   * Neither the name of The Linux Foundation nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.

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
 **/

#include "net/http/http_getzip_factory.h"
#include "net/http/http_getzip_bridge.h"
#include "net/libnetxt/dyn_lib_loader.h"

namespace net
{

const char* kPluginName = "libgetzip_plugin";

HttpGetZipFactory* HttpGetZipFactory::s_pFactory = NULL;

typedef IGetZipManager* mngr_create_();

HttpGetZipFactory::HttpGetZipFactory() :
  m_pMngr(NULL), libHandle(NULL)
{
}

HttpGetZipFactory::~HttpGetZipFactory()
{
  delete m_pMngr;
  m_pMngr = NULL;

  if (NULL != libHandle) {
    LibraryManager::ReleaseLibraryHandle(kPluginName);
    libHandle = NULL;
  }
}

void HttpGetZipFactory::InitGETZipManager()
{
  if (NULL != s_pFactory)
    return;

  s_pFactory = new HttpGetZipFactory();

  s_pFactory->libHandle = LibraryManager::GetLibraryHandle(kPluginName);

  GetHttpResponseCode(NULL);
  if (s_pFactory->libHandle) {
    mngr_create_* mngrCreate = (mngr_create_*) LibraryManager::GetLibrarySymbol(
        s_pFactory->libHandle, "createGETZipManager", false);
    if (mngrCreate) {
      s_pFactory->m_pMngr = (IGetZipManager*) mngrCreate();
      if( NULL == s_pFactory->m_pMngr) {
        s_pFactory->m_pMngr = new GetZipManager();
      }
      return;
    }
    LibraryManager::ReleaseLibraryHandle(kPluginName);
    s_pFactory->libHandle = NULL;
  }
  s_pFactory->m_pMngr = new GetZipManager();
}

IGetZipManager* HttpGetZipFactory::GetGETZipManager()
{
  if (s_pFactory==NULL) {
    InitGETZipManager();
  }
  return s_pFactory->m_pMngr;
}

void HttpGetZipFactory::StopGETZipManager()
{
  if (libHandle == NULL) {
    return;
  }

  delete m_pMngr;
  m_pMngr = new GetZipManager();
  LibraryManager::ReleaseLibraryHandle(kPluginName);
  libHandle = NULL;
}

GetZipManager::GetZipManager()
{
}

GETZipDecompressionStatus GetZipManager::DecompressResponseHeaders(HttpResponseHeaders*, StreamSocket*)
{
  return NO_GETZIP_CONNECTION;
}

}
; //end network
