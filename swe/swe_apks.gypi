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
        'extensions_to_not_compress': 'pak',
        'native_lib_target': 'libswewebviewchromium',
        'additional_native_libs': [
          '<@(libnetxt_native_libs)',
          '<@(libsweadrenoext_native_libs)'],
        'additional_input_paths': [
          '<(PRODUCT_DIR)/swe_test_apk/assets/webviewchromium.pak',
        ],
        'asset_location': '<(PRODUCT_DIR)/swe_test_apk/assets',
        'conditions': [
          ['icu_use_data_file_flag==1', {
            'additional_input_paths': [
              '<(PRODUCT_DIR)/icudtl.dat',
            ],
          }],
        ],
        #'asset_location': '<(ant_build_out)/swe_test_apk/assets',
      },

      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/assets',
          'files': [
            '<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
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
      'variables': {
        'conditions': [
          ['target_arch=="arm64"', {
            'arm_dir': '../libs/arm64-v8a',
          }, {
            'arm_dir': '../libs/armeabi-v7a',
          }],
        ],
      },
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
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/icudtl.dat',
              ],
            }],
          ],
        },

        #ui res
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/',
          'files': [
            '<(PRODUCT_DIR)/res.java/ui_java.zip',
            '<(PRODUCT_DIR)/res.java/ui_strings_grd.zip',
          ],
        },
        #content res
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/',
          'files': [
            '<(PRODUCT_DIR)/res.java/content_java.zip',
            '<(PRODUCT_DIR)/res.java/content_strings_grd.zip',
          ],
        },
        #swe_res.
        {
          'destination': '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/',
          'files': [
            '<(PRODUCT_DIR)/res.java/swe_engine_java.zip',
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
                      '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/project.properties',
                      '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/AndroidManifest.xml',
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
                       '<(PRODUCT_DIR)/res.java/ui_java.zip',
                       '<(PRODUCT_DIR)/res.java/ui_strings_grd.zip',
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
                       '<(PRODUCT_DIR)/res.java/content_java.zip',
                       '<(PRODUCT_DIR)/res.java/content_strings_grd.zip',
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
                       '<(PRODUCT_DIR)/res.java/swe_engine_java.zip',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/res/',
                     ],
           'message': 'Merging SWE Resources'
        },

        {
           'action_name': 'merge_swe_libs',
           'inputs': ['<(DEPTH)/swe/tools/merge_resources.py',
                      '<(PRODUCT_DIR)/apks/SWE_TestApp.apk',
                     ],
           'outputs': ['<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libswewebviewchromium.so',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libicui18n.cr.so',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libicuuc.cr.so',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libstlport_sh_521.so',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libsweskia.so',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libswev8.so',
                      ],
           'action': ['python', '<(DEPTH)/swe/tools/copy.py',
                       '<(PRODUCT_DIR)/swe_test_apk/libs/<(arm_dir)',
                       '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/',
                     ],
           'message': 'Merging SWE Libraries'
        },
      ],
    }
  ],
}
