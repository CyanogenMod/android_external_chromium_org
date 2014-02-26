/*
 *  Copyright (c) 2013, The Linux Foundation. All rights reserved.
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *      * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above
 *       copyright notice, this list of conditions and the following
 *       disclaimer in the documentation and/or other materials provided
 *       with the distribution.
 *     * Neither the name of The Linux Foundation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED "AS IS" AND ANY EXPRESS OR IMPLIED
 *  WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#include "android_webview/native/swe_client_cert_handler.h"

#include "base/android/jni_android.h"
#include "base/android/jni_array.h"
#include "base/android/jni_string.h"
#include "base/callback.h"
#include "base/callback_helpers.h"
#include "content/public/browser/browser_thread.h"
#include "jni/AwClientCertRequestHandler_jni.h"
#include "net/android/keystore_openssl.h"
#include "net/base/auth.h"
#include "net/cert/x509_certificate.h"
#include "net/ssl/openssl_client_key_store.h"


using base::android::ConvertJavaStringToUTF16;
using content::BrowserThread;

namespace android_webview {

typedef net::OpenSSLClientKeyStore::ScopedEVP_PKEY ScopedEVP_PKEY;

// Must be called on the I/O thread to record a client certificate
// and its private key in the OpenSSLClientKeyStore.
void RecordClientCertificateKey(
    const scoped_refptr<net::X509Certificate>& client_cert,
    ScopedEVP_PKEY private_key) {
  DCHECK(content::BrowserThread::CurrentlyOn(content::BrowserThread::IO));
  net::OpenSSLClientKeyStore::GetInstance()->RecordClientCertPrivateKey(
      client_cert.get(), private_key.get());
}

AwClientCertRequestHandler::AwClientCertRequestHandler(int request_id){
  mRequestId = request_id;
}

AwClientCertRequestHandler::~AwClientCertRequestHandler() {
}

void AwClientCertRequestHandler::Proceed(JNIEnv* env,
                                         jobject obj,
                                         jobjectArray encoded_chain_ref,
                                         jobject private_key_ref) {
    DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

    //Take back ownership of the request object.
    scoped_ptr<android_webview::SelectCertificateCallback> callback(
               reinterpret_cast<android_webview::SelectCertificateCallback*>(mRequestId));

    // Ensure that callback(NULL) is called in case of an error.
    base::Closure null_closure =
        base::Bind(*callback, scoped_refptr<net::X509Certificate>());

    base::ScopedClosureRunner guard(null_closure);

    if (encoded_chain_ref == NULL || private_key_ref == NULL) {
      LOG(ERROR) << "Client certificate request cancelled";
      return;
    }

    // Convert the encoded chain to a vector of strings.
    std::vector<std::string> encoded_chain_strings;
    if (encoded_chain_ref) {
      base::android::JavaArrayOfByteArrayToStringVector(
          env, encoded_chain_ref, &encoded_chain_strings);
    }

    std::vector<base::StringPiece> encoded_chain;
    for (size_t n = 0; n < encoded_chain_strings.size(); ++n)
      encoded_chain.push_back(encoded_chain_strings[n]);

    // Create the X509Certificate object from the encoded chain.
    scoped_refptr<net::X509Certificate> client_cert(
        net::X509Certificate::CreateFromDERCertChain(encoded_chain));
    if (!client_cert.get()) {
      LOG(ERROR) << "Could not decode client certificate chain";
      return;
    }

    // Create an EVP_PKEY wrapper for the private key JNI reference.
    ScopedEVP_PKEY private_key(
        net::android::GetOpenSSLPrivateKeyWrapper(private_key_ref));
    if (!private_key.get()) {
      LOG(ERROR) << "Could not create OpenSSL wrapper for private key";
      return;
    }

    guard.Release();

    // RecordClientCertificateKey() must be called on the I/O thread,
    // before the callback is called with the selected certificate on
    // the UI thread.
    content::BrowserThread::PostTaskAndReply(
        content::BrowserThread::IO,
        FROM_HERE,
        base::Bind(&RecordClientCertificateKey,
                   client_cert,
                   base::Passed(&private_key)),
        base::Bind(*callback, client_cert));
}

void AwClientCertRequestHandler::Cancel(JNIEnv* env, jobject obj) {
}

bool RegisterAwClientCertRequestHandler(JNIEnv* env) {
  return RegisterNativesImpl(env) >= 0;
}

}  // namespace android_webview
