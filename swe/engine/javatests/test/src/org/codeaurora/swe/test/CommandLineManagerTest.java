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
 *  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND mpN-INFRINGEMENT
 *  ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS
 *  BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 *  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 *  SUBSTITUTE GOODS OR SERVICES; LOSS OFUcSE, DATA, OR PROFITS; OR
 *  BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 *  WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE
 *  OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 *  IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
package org.codeaurora.swe.test;

import android.content.Context;
import android.os.Environment;


import org.chromium.base.test.util.Feature;
import org.chromium.net.test.util.TestWebServer;

import org.codeaurora.swe.CommandLineManager;
import org.chromium.base.CommandLine;

import java.io.ByteArrayInputStream;
import java.io.FileOutputStream;
import java.io.File;
import java.io.FileWriter;
import java.io.InputStream;
import java.nio.charset.Charset;

import junit.framework.Assert;


public class CommandLineManagerTest extends SWETestBase {
    private static final String LOGTAG = "WebViewTest";
    private static final String SWE_WEBVIEW_HTML = "<html><body> SWE WebView </body></html>";
    public static final String SINGLE_PROCESS_SWITCH = "single-process";
    public static final String AUTOCOMPLETE_SWITCH ="enable-interactive-autocomplete";
    public static final String SERVER_URL = "http://127.0.0.1";
    public static final String USER_URL1 = "http://127.0.0.3";
    public static final String USER_URL2 = "http://127.0.0.5";
    public static final String FPS_COUNTER = "enable-fps-counter";


    public String userCommandLineFile = "";

    private InputStream convertStringToInputStream(String commandLine) {
        InputStream stream = new ByteArrayInputStream(commandLine.getBytes(Charset.forName("UTF-8")));
        return stream;
    }

    private void validateAndReset() {
        assertTrue(CommandLine.getInstance().hasSwitch(AUTOCOMPLETE_SWITCH));
        assertTrue(CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH));
        assertEquals(CommandLine.getInstance().getSwitchValue("crash-log-server"), SERVER_URL);
        CommandLine.getInstance().reset();
    }

    @Feature({"SWECommandLine"})
    public void testCommandLineManager() {

        String testCommandLineString = "swe-browser --crash-log-server=\""+SERVER_URL+"\" --enable-interactive-autocomplete --single-process";


        // if no command-line switches are passed
        CommandLineManager.init(null,null);
        assertFalse(CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH));

        // test single line string
        CommandLineManager.init(convertStringToInputStream(testCommandLineString), null);
        validateAndReset();


         String testMultiLineString = "swe-browser\n"+
         "--crash-log-server=\""+SERVER_URL+"\"\n"+
         "--enable-interactive-autocomplete\n"+
         "--single-process\n";

         //test multiline commands in default file
        CommandLineManager.init(convertStringToInputStream(testMultiLineString), null);
        validateAndReset();

        String testMultiLineStringWithComments = "# Browser Name\n"+
        "swe-browser\n"+
        "# Enable Crash log server\n"+
        "--crash-log-server=\""+SERVER_URL+"\"\n"+
        "# Enable Interactive Autocomplete\n"+
        "--enable-interactive-autocomplete\n"+
        "# Run the browser in single process mode\n"+
        "--single-process\n";

        //test multiline commands wiht comments in default file
        CommandLineManager.init(convertStringToInputStream(testMultiLineStringWithComments), null);
        validateAndReset();

        String testMultiLineStringWithCommentsInSameLine = "# Browser Name\n"+
        "swe-browser "+
        "--crash-log-server=\""+SERVER_URL+"\" # Enable Crash log server\n\n"+
        "--enable-interactive-autocomplete # Enable Interactive Autocomplete\n"+
        "--single-process # Run the browser in single process mode\n";

        //test multiline commands wiht comments in default file
        CommandLineManager.init(convertStringToInputStream(testMultiLineStringWithCommentsInSameLine), null);
        validateAndReset();

        String userCommandLineString = "swe-user --crash-log-server=\""+USER_URL1+
                "\" --crash-log-server=\""+USER_URL2+"\" --enable-fps-counter";

        // check only with user command line string
        CommandLineManager.init(null, convertStringToInputStream(userCommandLineString));
        assertFalse(CommandLine.getInstance().hasSwitch(AUTOCOMPLETE_SWITCH));
        assertFalse(CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH));
        assertEquals(CommandLine.getInstance().getSwitchValue("crash-log-server"), USER_URL2);
        assertTrue(CommandLine.getInstance().hasSwitch(FPS_COUNTER));
        CommandLine.getInstance().reset();

        // test with user command line and default commandline
        // assure that user command arguements overrides default
        CommandLineManager.init(convertStringToInputStream(testMultiLineStringWithComments), convertStringToInputStream(userCommandLineString));
        assertTrue(CommandLine.getInstance().hasSwitch(AUTOCOMPLETE_SWITCH));
        assertTrue(CommandLine.getInstance().hasSwitch(SINGLE_PROCESS_SWITCH));
        assertTrue(CommandLine.getInstance().hasSwitch(FPS_COUNTER));
        assertNotSame(CommandLine.getInstance().getSwitchValue("crash-log-server") , USER_URL1);
        assertEquals(CommandLine.getInstance().getSwitchValue("crash-log-server"), USER_URL2);
        assertNotSame(CommandLine.getInstance().getSwitchValue("crash-log-server") , SERVER_URL);

        //reset all the commandline flags
        CommandLine.getInstance().reset();
    }
}
