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
        'swe_system_package_paks',
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
        'additional_input_paths': [
          '<!@pymod_do_main(swe_repack_locales -i -p <(OS) -g <(SHARED_INTERMEDIATE_DIR) -s <(SHARED_INTERMEDIATE_DIR) -x <(SHARED_INTERMEDIATE_DIR) <(locales))',
        ],
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
    {
      'target_name': 'swe_system_package_paks',
      'type': 'none',
      'dependencies': [
        '<(DEPTH)/components/components_strings.gyp:components_strings',
        '<(DEPTH)/net/net.gyp:net_resources',
        '<(DEPTH)/ui/resources/ui_resources.gyp:ui_resources',
        '<(DEPTH)/ui/strings/ui_strings.gyp:ui_strings',
        '<(DEPTH)/content/app/strings/content_strings.gyp:content_strings',
        '<(DEPTH)/content/content_resources.gyp:content_resources',
        '<(DEPTH)/third_party/WebKit/public/blink_resources.gyp:blink_resources',
        '<(DEPTH)/webkit/glue/resources/webkit_resources.gyp:webkit_resources',
      ],
      'variables': {
        'repack_path': '<(DEPTH)/tools/grit/grit/format/repack.py',
      },
      'actions': [
        {
          'action_name': 'repack_non_locale_paks',
          'variables': {
            # pak_inputs should be in sync with pak_inputs in the
            # repack_android_webview_pack action within android_webview.gyp
            'pak_inputs': [
              '<(SHARED_INTERMEDIATE_DIR)/blink/public/resources/blink_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/content_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/net/net_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/ui_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/webkit/webkit_resources_100_percent.pak',
              '<(SHARED_INTERMEDIATE_DIR)/components/component_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/ui/resources/webui_resources.pak',
              '<(SHARED_INTERMEDIATE_DIR)/content/browser/tracing/tracing_resources.pak',
            ],
          },
          'inputs': [
            '<(repack_path)',
            '<@(pak_inputs)',
          ],
          'outputs': [
            '<(PRODUCT_DIR)/swe_android_system_browser_apk/assets/webviewchromium.pak',
          ],
          'action': ['python', '<(repack_path)', '<@(_outputs)',
                     '<@(pak_inputs)'],
        },
        {
          'action_name': 'repack_locale_paks',
          'variables': {
            'repack_extra_flags%': [],
            'repack_output_dir%': '<(PRODUCT_DIR)/swe_android_system_browser_apk/assets',
            'repack_locales_cmd': ['python', '<(DEPTH)/swe/tools/build/swe_repack_locales.py'],
            'grit_out_dir': '<(SHARED_INTERMEDIATE_DIR)/android_webview',
          },
          'inputs': [
            'tools/build/swe_repack_locales.py',
            '<!@pymod_do_main(swe_repack_locales -i -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(repack_output_dir) <(repack_extra_flags) <(locales))',
          ],
          'outputs': [
            '<!@pymod_do_main(swe_repack_locales -o -p <(OS) -g <(grit_out_dir) -s <(SHARED_INTERMEDIATE_DIR) -x <(repack_output_dir) <(locales))',
          ],
          'action': [
            '<@(repack_locales_cmd)',
            '-p', '<(OS)',
            '-g', '<(grit_out_dir)',
            '-s', '<(SHARED_INTERMEDIATE_DIR)',
            '-x', '<(repack_output_dir)/.',
            '<@(repack_extra_flags)',
            '<@(locales)',
          ],
        },
      ],
    },
  ],
}

