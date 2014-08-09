// Copyright 2014 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

package com.google.javascript.jscomp;

import java.util.List;

public class ChromePassConfig extends DefaultPassConfig {
    public ChromePassConfig(CompilerOptions options) {
        super(options);
    }

    @Override
    protected List<PassFactory> getChecks() {
        List<PassFactory> checks = super.getChecks();
        checks.add(0, chromePass);
        return checks;
    }

    final PassFactory chromePass = new PassFactory("chromePass", true) {
        @Override
        CompilerPass create(AbstractCompiler compiler) {
            return new ChromePass(compiler);
        }
    };
}
