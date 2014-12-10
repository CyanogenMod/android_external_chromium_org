{
  'variables' : {
      'prebuiltjar': '<!(python <(DEPTH)/build/dir_exists.py ../swe/fast-webview/target/)',
      #'debug': "<!(echo <(prebuiltjar) 1>&2)",
      'srcbuild': '<!(python <(DEPTH)/build/dir_exists.py ../swe/fast-webview/java/src/)',
  },
  'targets' : [
    {
      'target_name': 'fast_webview_java',
      'type': 'none',
      'conditions': [
        ['prebuiltjar == "True"', {
          'dependencies': [
            'swe_engine_java',
          ],
          'variables': {
            'jar_path': './fast-webview/target/fast_webview_java.jar',
          },
          'includes':['../build/java_prebuilt.gypi']
        }],
        ['srcbuild == "True"', {
          'dependencies': [
            '../swe/fast-webview/java/sweet.gypi:fast_webview_java',
          ],
        }],
      ],
    },
    {
      'target_name': 'sweet_test_apk',
      'type': 'none',
      'conditions': [
        ['srcbuild == "True"', {
          'dependencies': [
            'swe_test_app_apk_java',
          ],
          'variables': {
            'apk_name': 'SWEETTest',
            'java_in_dir': '../swe/fast-webview/javatest/test',
            'is_test_apk': 1,
          },
          'includes': [ '../build/java_apk.gypi' ],
        }],
      ],
    },
  ],
}