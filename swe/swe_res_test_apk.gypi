{
  'targets' : [
    {
      'target_name': 'swe_res_test_apk',
      'type': 'none',
      'dependencies': [
        'swe_engine_perbuilt_java',
      ],
      'variables': {
        'apk_name': 'SWE_TestApp_res',
        'manifest_package_name': 'org.codeaurora.swe.testapp',
        'java_in_dir': 'SWESampleApp',
        'resource_dir': '../swe/SWESampleApp/res',
        'extensions_to_not_compress': 'pak',
        'is_swe_res_apk': 1,
        'additional_input_paths': [
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/assets/webviewchromium.pak',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/assets/icudtl.dat',
        ],
        'asset_location': '<(PRODUCT_DIR)/swe_res_test_apk/assets',

        'additional_res_packages': [
          'org.codeaurora.swe',
          'org.chromium.content',
          'org.chromium.ui',
        ],

        'additional_R_text_files': [
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/R.txt',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/R.txt',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/R.txt',
        ],

        'dependencies_res_zip_paths': [
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/swe_res/swe_engine_java.zip',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/content_java.zip',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/content_res/content_strings_grd.zip',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/ui_java.zip',
          '<(PRODUCT_DIR)/swe_test_apk/swe_res/ui_res/ui_strings_grd.zip',
        ],
      },

      'copies': [
        {
          'destination': '<(PRODUCT_DIR)/swe_res_test_apk/assets',
          'files': [
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/assets/webviewchromium.pak',
          ],
          'conditions': [
            ['icu_use_data_file_flag==1', {
              'files': [
                '<(PRODUCT_DIR)/swe_test_apk/swe_res/assets/icudtl.dat',
              ],
            }],
          ],
        },
        {
          'destination': '<(PRODUCT_DIR)/swe_res_test_apk/libs/<(android_app_abi)',
          'files': [
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libicui18n.cr.so',
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libicuuc.cr.so',
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libstlport_sh_521.so',
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libsweskia.so',
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libswev8.so',
            '<(PRODUCT_DIR)/swe_test_apk/swe_res/lib/libswewebviewchromium.so',
          ],
        }
      ],
      'includes': [ '../build/java_apk.gypi' ],
    },
    {
      'target_name': 'swe_engine_perbuilt_java',
      'type': 'none',
      'variables': {
        'jar_path': '<(PRODUCT_DIR)/swe_test_apk/swe_res/jar/swe_engine.jar',
      },
      'includes': ['../build/java_prebuilt.gypi'],
    },
  ],
}
