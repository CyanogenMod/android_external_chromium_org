{
  'variables' : {
      'prebuiltjar': '<!(python <(DEPTH)/build/dir_exists.py ../swe/fast-webview/target/)',
      #'debug': "<!(echo <(prebuiltjar) 1>&2)",
      'srcbuild': '<!(python <(DEPTH)/build/dir_exists.py ../swe/fast-webview/src/)',
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
            '../swe/fast-webview/sweet.gypi:fast_webview_java',
          ]
        }],
      ],
    },
  ],
}