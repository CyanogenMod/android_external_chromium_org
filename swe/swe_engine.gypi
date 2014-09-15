{
  'includes': [
    'swe_apks.gypi',
    #'./engine/javatests/swe_webview_tests.gypi',
    #'./browser/test/swe_browser_test_apk.gypi',
    './browser/swe_android_browser.gypi',
    #'./swe_fast_webview.gypi',
    './android_support.gypi',
    './swe_system_package.gypi',
  ],
  'targets' : [
    {
      'target_name': 'swe_engine_java',
      'type': 'none',
      'dependencies': [
        '../android_webview/android_webview.gyp:android_webview_java',
        '../android_webview/android_webview.gyp:libswewebviewchromium',
        '../android_webview/android_webview.gyp:android_webview_pak',
      ],
      'variables': {
        'java_in_dir': 'engine/java',
        'R_package': 'org.codeaurora.swe',
        'R_package_relpath': 'org/codeaurora/swe/',
        'has_java_resources': 1,
      },
      'includes': [ '../build/java.gypi' ],
    },
    {
      'target_name': 'swe_engine_native_java',
      'type': 'none',
      'variables': {
        'java_in_dir': '../android_webview/java/generated_src/',
      },
      'includes': [ '../build/swe_java.gypi' ],
    },
    {
      'target_name': 'swe_engine',
      'type' : 'none',
      'dependencies': [
        'swe_engine_java',
        'swe_engine_native_java',
      ],
      'variables': {
        'input_jar_dir': '<(PRODUCT_DIR)/lib.java/',
        'input_jar_files': [
          '<(input_jar_dir)/swe_engine_java.jar',
          '<(input_jar_dir)/android_webview_java.jar',
          '<(input_jar_dir)/navigation_interception_java.jar',
          #'<(input_jar_dir)/autofill_java.jar',
          '<(input_jar_dir)/net_java.jar',
          '<(input_jar_dir)/base_java.jar',
          '<(input_jar_dir)/content_java.jar',
          '<(input_jar_dir)/swe_engine_native_java.jar',
          '<(input_jar_dir)/eyesfree_java.jar',
          '<(input_jar_dir)/ui_java.jar',
          '<(input_jar_dir)/media_java.jar',
          '<(input_jar_dir)/web_contents_delegate_android_java.jar',
        ],
        'pattern': '*',
      },
      'includes': [ '../third_party/jarjar/jarjar_multiple.gypi' ],
    },
  ],
}
