vars = {
  "webkit_trunk":
    "http://svn.webkit.org/repository/webkit/trunk",
  "webkit_revision": "46741",
}


deps = {
  "src/breakpad/src":
    "http://google-breakpad.googlecode.com/svn/trunk/src@346",

  "src/build/util/support":
    "/trunk/deps/support@19914",

  "src/googleurl":
    "http://google-url.googlecode.com/svn/trunk@110",

  "src/sdch/open-vcdiff":
    "http://open-vcdiff.googlecode.com/svn/trunk@26",

  "src/testing/gtest":
    "http://googletest.googlecode.com/svn/trunk@267",

  "src/third_party/WebKit":
    "/trunk/deps/third_party/WebKit@20601",

  "src/third_party/icu38":
    "/trunk/deps/third_party/icu38@20192",

  "src/third_party/protobuf2/src":
    "http://protobuf.googlecode.com/svn/trunk@154",

  "src/tools/gyp":
    "http://gyp.googlecode.com/svn/trunk@573",

  "src/v8":
    "http://v8.googlecode.com/svn/trunk@2592",

  "src/native_client":
    "http://nativeclient.googlecode.com/svn/trunk/src/native_client@385",

  "src/third_party/skia":
    "http://skia.googlecode.com/svn/trunk@293",

  "src/third_party/WebKit/LayoutTests":
    Var("webkit_trunk") + "/LayoutTests@" + Var("webkit_revision"),

  "src/third_party/WebKit/JavaScriptCore":
    Var("webkit_trunk") + "/JavaScriptCore@" + Var("webkit_revision"),

  "src/third_party/WebKit/WebCore":
    Var("webkit_trunk") + "/WebCore@" + Var("webkit_revision"),

  "src/third_party/tcmalloc/tcmalloc":
    "http://google-perftools.googlecode.com/svn/trunk@74",

  "src/tools/page_cycler/acid3":
    "/trunk/deps/page_cycler/acid3@19546",

  # TODO(jianli): Remove this once we do not need to run worker's layout tests
  # in ui test.
  "src/chrome/test/data/workers/LayoutTests/fast/events":
    Var("webkit_trunk") + "/LayoutTests/fast/events@" + Var("webkit_revision"),
  "src/chrome/test/data/workers/LayoutTests/fast/workers":
    Var("webkit_trunk") + "/LayoutTests/fast/workers@" + Var("webkit_revision"),
  "src/chrome/test/data/workers/LayoutTests/http/tests/resources":
    Var("webkit_trunk") + "/LayoutTests/http/tests/resources@" + Var("webkit_revision"),
  "src/chrome/test/data/workers/LayoutTests/http/tests/workers":
    Var("webkit_trunk") + "/LayoutTests/http/tests/workers@" + Var("webkit_revision"),
  "src/chrome/test/data/workers/LayoutTests/http/tests/xmlhttprequest":
    Var("webkit_trunk") + "/LayoutTests/http/tests/xmlhttprequest@" + Var("webkit_revision"),
}


deps_os = {
  "win": {
    "src/third_party/cygwin":
      "/trunk/deps/third_party/cygwin@11984",

    "src/third_party/python_24":
      "/trunk/deps/third_party/python_24@19441",
  },
  "mac": {
    "src/third_party/GTM":
      "http://google-toolbox-for-mac.googlecode.com/svn/trunk@163",
    "src/third_party/pdfsqueeze":
      "http://pdfsqueeze.googlecode.com/svn/trunk@2",
    "src/third_party/WebKit/WebKit/mac":
      Var("webkit_trunk") + "/WebKit/mac@" + Var("webkit_revision"),
    "src/third_party/WebKit/WebKitLibraries":
      Var("webkit_trunk") + "/WebKitLibraries@" + Var("webkit_revision"),
  },
  "unix": {
    # Linux, really.
    "src/third_party/xdg-utils":
      "/trunk/deps/third_party/xdg-utils@22207",
  },
}


include_rules = [
  # Everybody can use some things.
  "+base",
  "+build",
  "+ipc",

  # For now, we allow ICU to be included by specifying "unicode/...", although
  # this should probably change.
  "+unicode",
  "+testing",

  # Allow anybody to include files from the "public" Skia directory in the
  # webkit port. This is shared between the webkit port and Chrome.
  "+webkit/port/platform/graphics/skia/public",
]


# checkdeps.py shouldn't check include paths for files in these dirs:
skip_child_includes = [
   "breakpad",
   "gears",
   "native_client",
   "o3d",
   "sdch",
   "skia",
   "testing",
   "third_party",
   "v8",
]


hooks = [
  {
    # A change to a .gyp, .gypi, or to GYP itself shound run the generator.
    "pattern": "\\.gypi?$|[/\\\\]src[/\\\\]tools[/\\\\]gyp[/\\\\]",
    "action": ["python", "src/tools/gyp/gyp_chromium"],
  },
]
