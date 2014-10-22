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

import android.os.Handler;

import java.util.ArrayList;

public class Activator extends Observable {
    private boolean mScheduled = false;
    private ArrayList<Observable> mPending = new ArrayList<Observable>();
    static Activator sActivator = null;

    static Activator getInstance() {
        if (sActivator == null) {
            sActivator = new Activator();
        }
        return sActivator;
    }

    static public Observable activate(Observable ob) {
        Activator me = Activator.getInstance();
        me._activate(ob);
        return ob;
    }

    private Observable _activate(Observable ob) {
        ob.subscribe(this);
        ob.get();
        return ob;
    }

    private Observable _activate(final Observer observer, Object[] params) {
        FuncObservable fob = new FuncObservable(
                new Functor() {
                    @Override
                    public Object func(Object... params) {
                        observer.onChange(params);
                        return null;
                    }
                }, params);
        return _activate(fob);
    }

    static public Observable activate(Observer observer, Object... params) {
        Activator me = Activator.getInstance();
        return me._activate(observer, params);
    }

    @Override
    public void invalidate(Observable ob) {
        mPending.add(ob);

        if (!mScheduled) {
            mScheduled = true;

            (new Handler()).post(new Runnable() {
                @Override
                public void run() {
                    //Scheduler state is reset and pending list is copied over in order to
                    //accumulate new invalidates as result of calling get() below.
                    mScheduled = false;
                    ArrayList<Observable> toRun = new ArrayList<Observable>(mPending);
                    mPending.clear();

                    for (Observable ob: toRun) {
                        ob.get();
                    }
                }
            });
        }
    }
}
