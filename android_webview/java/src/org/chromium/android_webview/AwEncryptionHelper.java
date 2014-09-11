/*
 *  Copyright (c) 2014, The Linux Foundation. All rights reserved.
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
package org.chromium.android_webview;

import java.util.Arrays;
import java.util.Calendar;
import java.io.ByteArrayOutputStream;
import java.math.BigInteger;
import java.security.KeyPair;
import java.security.KeyStore;
import java.security.interfaces.RSAPrivateKey;
import java.security.interfaces.RSAPublicKey;
import java.security.KeyPairGenerator;

import javax.crypto.Cipher;
import javax.security.auth.x500.X500Principal;

import android.content.Context;
import android.content.SharedPreferences;
import android.security.KeyPairGeneratorSpec;
import android.util.Base64;
import android.util.Log;

/**
 * Creates and uses the key material to protect login form passwords per user
 * request.
 */
public final class AwEncryptionHelper {
    private final String TAG = "AwEncryptionHelper";
    private final String KEYSTORE_PROVIDER_ANDROID_KEYSTORE = "AndroidKeyStore";
    private final String ALGORITHM = "RSA/ECB/PKCS1Padding";
    private final String KEY_ALIAS = "AwEncryptionKey";
    private final int KEY_VERSION = 1;
    private final Context mContext;
    private RSAPrivateKey mPrivateKey;
    private RSAPublicKey mPublicKey;
    private boolean mKeysLoaded = false;

    public AwEncryptionHelper(Context context) {
        mContext = context;
    }

    /**
     * Generates and loads keys if necessary.
     */
    private void setupKeysIfNecessary(AwSettings awSettings) {
        try {
            KeyStore keyStore = KeyStore.getInstance(KEYSTORE_PROVIDER_ANDROID_KEYSTORE);
            keyStore.load(null);

            if (keyStore.containsAlias(KEY_ALIAS)) {
                if (!mKeysLoaded) {
                    KeyStore.PrivateKeyEntry keyEntry =
                            (KeyStore.PrivateKeyEntry) keyStore.getEntry(KEY_ALIAS, null);
                    if (keyEntry != null) {
                        mPublicKey = (RSAPublicKey) keyEntry.getCertificate().getPublicKey();
                        mPrivateKey = (RSAPrivateKey) keyEntry.getPrivateKey();
                        mKeysLoaded = true;
                    }
                }
            } else {
                // We are here because: (a) we have not generated keys yet, or
                // (b) keys were purged by the keystore because the user modified
                // the screen lock policy through system settings. We clear all
                // all saved passwords because of (b).
                awSettings.clearPasswords();
                mKeysLoaded = generatePasswordKeyPair();
            }
        } catch (Exception e) {
            Log.e(TAG, "Failed to setup keys: " + e);
            mKeysLoaded = false;
        }
    }

    /**
     * Generate password protection keys.
     * @return True if no exception is thrown.
     */
    private boolean generatePasswordKeyPair() {
        try {
            Calendar notBefore = Calendar.getInstance();
            Calendar notAfter = Calendar.getInstance();
            notAfter.add(1, Calendar.YEAR);
            KeyPairGeneratorSpec spec = new KeyPairGeneratorSpec.Builder(mContext)
                            .setAlias(KEY_ALIAS)
                            .setSubject(
                                    new X500Principal(String.format("CN=%s, OU=%s",
                                            KEY_ALIAS,
                                            mContext.getPackageName())))
                            .setSerialNumber(BigInteger.valueOf(KEY_VERSION))
                            .setStartDate(notBefore.getTime())
                            .setEndDate(notAfter.getTime())
                            .build();
            KeyPairGenerator kpGenerator = KeyPairGenerator.getInstance("RSA",
                KEYSTORE_PROVIDER_ANDROID_KEYSTORE);
            kpGenerator.initialize(spec);
            KeyPair kp = kpGenerator.generateKeyPair();
            mPrivateKey = (RSAPrivateKey) kp.getPrivate();
            mPublicKey = (RSAPublicKey) kp.getPublic();
            return true;
        } catch (Exception e) {
            Log.e(TAG, "Failed to generate key pair for password protection: " + e);
            return false;
        }
    }

    /**
     * Setup password protection keys if necessary and return the encrypted
     * input. The input array is zeroed out prior to returning to caller to
     * reduce the window of exposure.
     */
    public byte[] encrypt(byte[] input, AwSettings awSettings) {
        byte[] output = null;

        setupKeysIfNecessary(awSettings);

        if (mKeysLoaded && input != null && input.length != 0) {
            try {
                Cipher c = Cipher.getInstance(ALGORITHM);
                c.init(Cipher.ENCRYPT_MODE, mPublicKey);
                ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                int inputLength = input.length;
                int inputBlockSize = c.getBlockSize();
                int currentIndex = 0, inputChunk;
                byte[] inputBlock, outputBlock;
                while (currentIndex < inputLength) {
                    inputChunk = (inputLength - currentIndex) < inputBlockSize ?
                            inputLength % inputBlockSize : inputBlockSize;
                    inputBlock = Arrays.copyOfRange(input,
                            currentIndex, currentIndex + inputChunk);
                    outputBlock = c.doFinal(inputBlock);
                    // Zero out input block
                    Arrays.fill(inputBlock, (byte) 0);
                    outputStream.write(outputBlock);
                    currentIndex += inputBlockSize;
                }
                output = outputStream.toByteArray();
                // Zero out input data
                Arrays.fill(input, (byte) 0);
            } catch (Exception e) {
                Log.e(TAG, "Encryption failed: " + e);
            }
        }
        return output;
    }

    /**
     * Setup password protection keys if necessary and return the decrypted
     * input.
     */
    public byte[] decrypt(byte[] input, AwSettings awSettings) {
        byte[] output = null;

        setupKeysIfNecessary(awSettings);

        if (mKeysLoaded && input != null && input.length != 0) {
            try {
                Cipher c = Cipher.getInstance(ALGORITHM);
                c.init(Cipher.DECRYPT_MODE, mPrivateKey);
                ByteArrayOutputStream outputStream = new ByteArrayOutputStream();
                int inputLength = input.length;
                int inputBlockSize = c.getBlockSize();
                int currentIndex = 0, inputChunk;
                byte[] inputBlock, outputBlock;
                while (currentIndex < inputLength) {
                    inputChunk = (inputLength - currentIndex) < inputBlockSize ?
                            inputLength % inputBlockSize : inputBlockSize;
                    inputBlock = Arrays.copyOfRange(input,
                            currentIndex, currentIndex + inputChunk);
                    outputBlock = c.doFinal(inputBlock);
                    outputStream.write(outputBlock);
                    currentIndex += inputBlockSize;
                }
                output = outputStream.toByteArray();
            } catch (Exception e) {
                Log.e(TAG, "Decryption failed: " + e);
            }
        }
        return output;
    }

}
