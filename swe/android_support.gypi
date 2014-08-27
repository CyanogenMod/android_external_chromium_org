{
    'targets': [
        ## v4
        {
          'target_name': 'android-support-v4',
          'type': 'none',
          'dependencies': [
            'android-support-v4-jellybean-mr2',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/java',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-jellybean-mr2',
          'type': 'none',
          'dependencies': [
            'android-support-v4-jellybean-mr1'
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/jellybean-mr2',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-jellybean-mr1',
          'type': 'none',
          'dependencies': [
            'android-support-v4-jellybean'
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/jellybean-mr1',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-jellybean',
          'type': 'none',
          'dependencies': [
            'android-support-v4-ics-mr1',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/jellybean',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-ics-mr1',
          'type': 'none',
          'dependencies': [
            'android-support-v4-ics',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/ics-mr1',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-ics',
          'type': 'none',
          'dependencies': [
            'android-support-v4-honeycomb-mr2',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/ics',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-honeycomb-mr2',
          'type': 'none',
          'dependencies': [
            'android-support-v4-honeycomb',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/honeycomb_mr2',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-honeycomb',
          'type': 'none',
          'dependencies': [
            'android-support-v4-gingerbread',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/honeycomb',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-gingerbread',
          'type': 'none',
          'dependencies': [
            'android-support-v4-froyo',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/gingerbread',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-froyo',
          'type': 'none',
          'dependencies': [
            'android-support-v4-eclair',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/froyo',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v4-eclair',
          'type': 'none',
          'dependencies': [
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v4/eclair',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },


        ## v13
        {
          'target_name': 'android-support-v13',
          'type': 'none',
          'dependencies': [
             'android-support-v4',
             'android-support-v13-ics-mr1',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v13/java',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },

        {
          'target_name': 'android-support-v13-ics-mr1',
          'type': 'none',
          'dependencies': [
            'android-support-v13-ics',
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v13/ics-mr1',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },
        {
          'target_name': 'android-support-v13-ics',
          'type': 'none',
          'dependencies': [
          ],
          'variables': {
            'java_in_dir': '../swe/android/support/src/v13/ics',
          },
          'includes': [ '../build/swe_java.gypi' ],
        },

    ],
}
