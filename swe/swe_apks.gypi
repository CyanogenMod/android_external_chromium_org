{
  'targets' : [
    {
      'target_name': 'swe_test_apk',
      'type': 'none',
      'dependencies': [
        'swe_engine_java',
        '<@(libnetxt_dependencies)',
        '<@(libsweadrenoext_dependencies)',
      ],
      'variables': {
        'apk_name': 'SWE_TestApp',
        'manifest_package_name': 'org.codeaurora.swe.testapp',
        'java_in_dir': 'SWESampleApp',
        'resource_dir': '../swe/SWESampleApp/res',
        'native_lib_target': 'libswewebviewchromium',
        'additional_input_paths': [
          '<(PRODUCT_DIR)/swe_test_apk/assets/webviewchromium.pak',
        ],
        #'asset_location': '<(ant_build_out)/swe_test_apk/assets',
        'additional_native_libs': ['<@(libnetxt_native_libs)', '<@(libsweadrenoext_native_libs)']
      },
      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/assets',
          'files': [
            '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak',
          ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },

    {
      'target_name': 'swe_res',
      'type': 'none',
       'dependencies': [
        'swe_test_apk',
        'swe_engine',
      ],
      'copies' : [
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/swe_res/jar/',
          'files': [
            '<(PRODUCT_DIR)/lib.java/swe_engine.jar'
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/swe_res/assets',
          'files': [
            '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak'
          ],
        },
      ],
      'actions': [
        {
          'action_name': 'create_lib_projects',
          'inputs': [ '<(DEPTH)/swe/tools/createAppRes.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                    ],
          'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/project.properties',
                      '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/AndroidManifest.xml',
                      '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/project.properties',
                      '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/AndroidManifest.xml',
                     ],
          'action': ['python', '<(DEPTH)/swe/tools/createAppRes.py',
                     '<(DEPTH)/swe/tools/createAppResources.sh',
                     '<(PRODUCT_DIR)/swe_test_apk/swe_res/'],
        },
        {
           'action_name': 'merge_ui_res',
           'inputs': ['<(DEPTH)/swe/tools/merge_resources.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                     ],
           'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/res/values/strings.xml'],
           'action': ['python', '<(DEPTH)/swe/tools/merge_resources.py',
                       '<(DEPTH)/ui/android/java/res/',
                       '<(PRODUCT_DIR)/gen/ui_java/res_grit/',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/res/',
                     ],
           'message': 'Merging UI Resources'
        },
        {
           'action_name': 'merge_content_res',
           'inputs': ['<(DEPTH)/swe/tools/merge_resources.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                     ],
           'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/res/values/strings.xml'],
           'action': ['python', '<(DEPTH)/swe/tools/merge_resources.py',
                       '<(DEPTH)/content/public/android/java/res/',
                       '<(PRODUCT_DIR)/gen/content_java/res_grit/',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/res/',
                     ],
           'message': 'Merging Content Resources'
        },
        {
           'action_name': 'merge_swe_res',
           'inputs': ['<(DEPTH)/swe/tools/merge_resources.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                     ],
           'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/res/values/strings.xml'],
           'action': ['python', '<(DEPTH)/swe/tools/merge_resources.py',
                       '<(DEPTH)/swe/engine/java/res/',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/res/',
                     ],
           'message': 'Merging SWE Resources'
        },
        {
           'action_name': 'merge_swe_libs',
           'inputs': ['<(DEPTH)/swe/tools/merge_resources.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                     ],
           'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libswewebviewchromium.so'],
           'action': ['python', '<(DEPTH)/swe/tools/merge_resources.py',
                       '<(PRODUCT_DIR)/swe_test_apk/libs/armeabi-v7a',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/',
                     ],
           'message': 'Merging SWE Libraries'
        },
      ],
    }
  ],
}
