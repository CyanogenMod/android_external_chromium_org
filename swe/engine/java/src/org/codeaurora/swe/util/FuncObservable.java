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

public class FuncObservable  extends Observable {

    private Functor functor;
    private Object [] mInputs;
    private Observable [] mObInputs;

    public FuncObservable(Functor f, Object... params) {
        functor = f;
        mObInputs = new Observable[params.length];
        mInputs = new Object[params.length];

        for (int i=0; i < params.length; ++i) {
            if (params[i] instanceof Observable)
                mObInputs[i] = (Observable)params[i];
            else
                mInputs[i] = params[i];
        }
    }

    @Override
    public Object get() {
        if (!mValid) {
            for (int i=0; i < mObInputs.length; ++i) {
                if (mObInputs != null) {
                    mInputs[i] = mObInputs[i].get();
                }
            }
            mValid = true;
            mValue = functor.func(mInputs);
        }
        return mValue;
    }

    @Override
    public void onOff(boolean isOn) {
        for (Observable ob : mObInputs) {
            if (isOn)
                ob.subscribe(this);
            else
                ob.unsubscribe(this);
        }
        //we are entering or leaving an un subscribed state, in which we do not
        //receive invalidations, so our state is not known.
        mValid = false;
    }
}
