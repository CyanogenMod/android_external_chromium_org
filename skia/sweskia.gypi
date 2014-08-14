#
{
  'variables' : {
    'variables': {
      'skia_build%': "shared",
      'prebuilt%': '<!(python <(DEPTH)/build/dir_exists.py ../third_party/skia/src/lib/target/)',
    },
    'prebuilt%': '<(prebuilt)',
    'skia_build%': '<(skia_build)',
    'component_skia': '',
    'product_name': '',
    'conditions' : [
     ['skia_build == "shared" and OS=="android"', {
       'component_skia': 'shared_library',
       'product_name': 'sweskia',
     }, {
       'component_skia': 'static_library',
       'product_name': 'skia',
     }],
     ['prebuilt == "True"', {
       'component_skia': 'none',
       'product_name': '',
     }],
     ],
  },
  'conditions': [
    ['prebuilt == "True"', {

       'targets' : [
         {
           'target_name': 'sweskia_cpfiles',
           'type': 'none',

           'copies': [
             {
               'destination': '<(PRODUCT_DIR)',
               'files': [
                 '<(DEPTH)/skia/swe_skia.py'
               ],
             },
           ],
         },
         {
           'target_name': 'sweskia',
           'type': 'none',

           'actions': [
              {
                'action_name': 'prebuilt_skia',
                'inputs': ['<(DEPTH)/third_party/skia/src/lib/target',],
                'outputs': ['<(PRODUCT_DIR)/lib/libsweskia.so'],
                'action': ['python', '<(PRODUCT_DIR)/swe_skia.py',
                             '<(DEPTH)/third_party/skia/src/lib/target',
                             '<(clang)'],
              },
            ],

          'dependencies': [
             '../build/android/setup.gyp:copy_system_libraries',
             'sweskia_cpfiles',
          ],
         }
       ],
     }, {
    }],
  ],
}
