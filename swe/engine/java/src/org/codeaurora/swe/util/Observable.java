/*
 * Copyright (c) 2014, The Linux Foundation. All rights reserved.
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

package org.codeaurora.swe.util;

import java.util.ArrayList;

public class Observable {
    protected boolean mValid = false;
    protected Object mValue;
    protected ArrayList<Observable> mSubs = new ArrayList<Observable>();

    public interface Functor {
        public abstract Object func(Object[] params);
    }

    public interface Observer {
        public abstract void onChange(Object[] params);
    }


    public void invalidate(Observable arg) {
        if (mValid) {
            for (Observable ob : mSubs) {
                ob.invalidate(this);
            }
            mValid = false;
        }
    }

    public void onOff(boolean bOnOff) {
        //nothing to do in base class
    }

    public void subscribe(Observable sub) {
        boolean bEmpty = mSubs.isEmpty();
        mSubs.add(sub);

        if (bEmpty)
            onOff(true);
    }

    public void unsubscribe(Observable sub) {
        mSubs.remove(sub);

        if (mSubs.isEmpty())
            onOff(false);
    }

    public boolean isSubscribed() {
        return !mSubs.isEmpty();
    }

    public void set(Object value) {
        if (value != mValue) {
            mValue = value;
            invalidate(null);
        }
    }

    public Object get() {
        if (!mValid) {
            mValid = true;
        }
        return mValue;
    }
}

