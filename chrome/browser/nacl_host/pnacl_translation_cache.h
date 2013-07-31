// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CHROME_BROWSER_NACL_HOST_PNACL_TRANSLATION_CACHE_H_
#define CHROME_BROWSER_NACL_HOST_PNACL_TRANSLATION_CACHE_H_

#include <map>

#include "base/callback.h"
#include "base/files/file_path.h"
#include "base/memory/weak_ptr.h"
#include "net/base/cache_type.h"

namespace base {
class MessageLoopProxy;
}

namespace disk_cache {
class Backend;
}

namespace nacl {
struct PnaclCacheInfo;
}

namespace net {
class DrainableIOBuffer;
}

namespace pnacl {
typedef base::Callback<void(int)> CompletionCallback;
typedef base::Callback<void(int, scoped_refptr<net::DrainableIOBuffer>)>
    GetNexeCallback;
class PnaclTranslationCacheEntry;
extern const int kMaxMemCacheSize;

class PnaclTranslationCache
    : public base::SupportsWeakPtr<PnaclTranslationCache> {
 public:
  PnaclTranslationCache();
  virtual ~PnaclTranslationCache();

  // Initialize the translation cache in |cache_dir| (or in memory if
  // |in_memory| is true). Call |callback| with a 0 argument on sucess and
  // <0 otherwise.
  int InitCache(const base::FilePath& cache_dir,
                bool in_memory,
                const CompletionCallback& callback);

  // Store the nexe in the translation cache. A reference to |nexe_data| is
  // held until completion or cancellation.
  void StoreNexe(const std::string& key, net::DrainableIOBuffer* nexe_data);

  // Store the nexe in the translation cache, and call |callback| with
  // the result. The result passed to the callback is 0 on success and
  // <0 otherwise. A reference to |nexe_data| is held until completion
  // or cancellation.
  void StoreNexe(const std::string& key,
                 net::DrainableIOBuffer* nexe_data,
                 const CompletionCallback& callback);

  // Retrieve the nexe from the translation cache. Write the data into |nexe|
  // and call |callback|, passing a result code (0 on success and <0 otherwise),
  // and a DrainableIOBuffer with the data.
  void GetNexe(const std::string& key, const GetNexeCallback& callback);

  // Return the number of entries in the cache backend.
  int Size();

  // Return the cache key for |info|
  static std::string GetKey(const nacl::PnaclCacheInfo& info);

 private:
  friend class PnaclTranslationCacheEntry;
  friend class PnaclTranslationCacheTest;
  // PnaclTranslationCacheEntry should only use the
  // OpComplete and backend methods on PnaclTranslationCache.
  void OpComplete(PnaclTranslationCacheEntry* entry);
  disk_cache::Backend* backend() { return disk_cache_; }

  int InitWithDiskBackend(const base::FilePath& disk_cache_dir,
                          int cache_size,
                          const CompletionCallback& callback);

  int InitWithMemBackend(int cache_size, const CompletionCallback& callback);

  int Init(net::CacheType,
           const base::FilePath& directory,
           int cache_size,
           const CompletionCallback& callback);

  void OnCreateBackendComplete(int rv);

  disk_cache::Backend* disk_cache_;
  CompletionCallback init_callback_;
  bool in_memory_;
  std::map<void*, scoped_refptr<PnaclTranslationCacheEntry> > open_entries_;

  DISALLOW_COPY_AND_ASSIGN(PnaclTranslationCache);
};

}  // namespace pnacl

#endif  // CHROME_BROWSER_NACL_HOST_PNACL_TRANSLATION_CACHE_H_
