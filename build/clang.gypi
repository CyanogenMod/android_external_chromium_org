{
  'conditions': [
    ['clang_use_integrated_as==1 and OS == "android"', {
      'cflags': [
         '-integrated-as',
      ],
    }],
    ['clang_use_integrated_as==0 and OS == "android"', {
      'cflags': [
        # Use GNU's 'as' to assemble
        '-no-integrated-as',
      ],
      'conditions': [
        ['clang_use_snapdragon==1 and OS == "android"', {
          'cflags': [
            # current GNU assembler is not able to assemble
            # integer division. This flag instructs LLVM to
            # use .word directive to assemble integer division
            '-mllvm -enable-hwdiv-encode=true',
          ],
        }],
      ],
    }],
  ],
}

