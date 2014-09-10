{
  'variables': {
    'swe_manifest': '<!(python <(DEPTH)/build/dir_exists.py ../swe/browser/src/com/android/browser/)',
    'src_system': '<!(python <(DEPTH)/build/dir_exists.py ../swe/browser/src_system/com/android/browser/)',
  },
  'targets' : [
    {
      'target_name': 'swe_system_package_folder',
      'type': 'none',
      'dependencies': [
        'copy_apk',
        'copy_libs',
        'create_makefiles',
        'swe_android_system_browser_apk',
        'swe_android_browser_apk'
      ],
    },
    # this should be in sync with swe/browser/swe_android_browser.gypi
    # TODO: Try to do this via same gyp to avoid duplication._
    {
      'target_name': 'swe_android_system_browser_apk',
      'type': 'none',
      'dependencies': [
        'swe_engine_java',
        'android-support-v13',
      ],
      'variables': {
        'apk_name': 'Browser',
        'manifest_package_name': 'com.android.browser',
        'app_manifest_version_name': '<!(../swe/browser/tools/generate_about.sh --quiet --name --about)',
        'app_manifest_version_code': '<!(../swe/browser/tools/generate_about.sh --quiet --code)',
        'java_in_dir': './browser/',
        'resource_dir': '../swe/browser/res',
        'assets_dir': '../swe/browser/assets',
        'additional_input_paths': ['<(PRODUCT_DIR)/android_webview_apk/assets/webviewchromium.pak'],
        'additional_src_dirs': ['<(DEPTH)/android_webview/java/generated_src'],

        'conditions': [
          ['src_system == "True"', {
            'additional_src_dirs+': [ '<(DEPTH)/swe/browser/src_system/com/android/browser'],
          },],
          ['swe_manifest == "True"', {
            'android_manifest_path': '../swe/browser/src_system/AndroidManifest.xml',
          }, {
            'android_manifest_path': '../swe/browser/AndroidManifest.xml',
          }],
        ],
      },
      'copies': [
        {
              'destination': '<(PRODUCT_DIR)/swe_android_system_browser_apk/assets/',
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
        {
              'destination': '<(PRODUCT_DIR)/swe_android_system_browser_apk/assets/wml',
              'files': [
                '<(assets_dir)/wml/swe_wml.xsl',
                '<(assets_dir)/wml/swe_wml.js',
                '<(assets_dir)/wml/swe_wml.css',
              ],
        },
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'swe_android_system_browser_apk_java',
      'type': 'none',
      'dependencies': [
        'swe_android_system_browser_apk',
      ],
      'includes': [ '../build/apk_fake_jar.gypi' ],
    },

    {
      'target_name': 'copy_libs',
      'type': 'none',
      'variables': {
        'conditions': [
          ['target_arch=="arm64"', {
            #SWE-FIXME : GYP is adding prefix ../swe to vairable
            # to counter that adding ../libs.
            'arm_dir': '../libs/arm64-v8a',
          }, {
            'arm_dir': '../libs/armeabi-v7a',
          }],
        ],
      },
      'actions': [
        {
          'action_name': 'swe_libs',
          'inputs': ['<(DEPTH)/swe/tools/copy.py',
                     '<(PRODUCT_DIR)/apks/Browser.apk',
                     '<(PRODUCT_DIR)/apks/SWE_AndroidBrowser.apk',
                    ],
          'outputs': ['<(PRODUCT_DIR)/swe_system_package/libs/'],
          'action': ['python', '<(DEPTH)/swe/tools/copy.py',
                     '<(PRODUCT_DIR)/swe_android_browser_apk/libs/<(arm_dir)',
                     '<(PRODUCT_DIR)/swe_system_package/libs/',
                    ],
          'message': 'Copy Browser Libs<(arm_dir)',
        },
      ],
    },
    {
      'target_name': 'copy_apk',
      'type': 'none',
      'actions': [
        {
          'action_name': 'swe_apk',
          'inputs': ['<(DEPTH)/swe/tools/copy.py',
                     '<(PRODUCT_DIR)/apks/Browser.apk',
                    ],
          'outputs': ['<(PRODUCT_DIR)/swe_system_package/apps/Browser.apk'
                     ],
          'action': ['python', '<(DEPTH)/swe/tools/copy.py',
                     '<(PRODUCT_DIR)/apks/Browser.apk',
                     '<(PRODUCT_DIR)/swe_system_package/apps/',
                    ],
          'message': 'Copy Browser.apk to swe_system_package',
        },
      ],
    },
    {
      'target_name': 'create_makefiles',
      'type': 'none',
      'actions': [
        {
          'action_name': 'swe_makefile',
          'inputs': ['<(PRODUCT_DIR)/apks/SWE_AndroidBrowser.apk',
                     '<(PRODUCT_DIR)/swe_system_package/apps/Browser.apk',
                     '<(PRODUCT_DIR)/swe_system_package/libs/',
                    ],
          'outputs': ['<(PRODUCT_DIR)/swe_system_package/Android.mk',
                     ],
          'action': ['python', '<(DEPTH)/swe/tools/generate_makefile.py',
                     '<(PRODUCT_DIR)/swe_android_browser_apk/native_libraries.json',
                     '<(PRODUCT_DIR)/swe_system_package/',
                    ],
          'message': 'Create Android.mk for swe_system_package',
        },
      ],
    },
    {
      'target_name': 'swe_system_package',
      'type': 'none',
      'dependencies': [
        'swe_system_package_folder',
      ],
      'actions': [
        {
          'action_name': 'swe_zip',
          'inputs': ['<(DEPTH)/swe/tools/zipfolder.py',
                     '<(PRODUCT_DIR)/apks/SWE_AndroidBrowser.apk',
                    ],
          'outputs': ['<(PRODUCT_DIR)/swe_system_package.zip',
                     ],
          'action': ['python', '<(DEPTH)/swe/tools/zipfolder.py',
                     '<(PRODUCT_DIR)/swe_system_package/',
                     '<(PRODUCT_DIR)/swe_system_package.zip',
                    ],
          'message': 'Create swe_system_package.zip',
        },
      ],
    },
  ],
}

