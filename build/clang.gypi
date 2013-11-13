{
  'conditions': [
    ['clang_use_integrated_as==1', {
      'cflags': [
         '-integrated-as',
      ],
    }],
    ['clang_use_integrated_as==0', {
      'cflags': [
        # Use GNU's 'as' to assemble
        '-no-integrated-as',
      ],
      'conditions': [
        ['clang_use_snapdragon==1', {
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

