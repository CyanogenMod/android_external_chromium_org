# Copyright (c) 2010 The Chromium Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

{
  'includes': [
    '../appcache/webkit_appcache.gypi',
    '../blob/webkit_blob.gypi',
    '../database/webkit_database.gypi',
    '../glue/webkit_glue.gypi',
    # TODO(tkent): Merge npapi_layout_test_plugin into TestNetscapePlugIn
    # of WebKit.
    '../tools/npapi_layout_test_plugin/npapi_layout_test_plugin.gypi',
    'webkit_support.gypi',
  ],
}

# Local Variables:
# tab-width:2
# indent-tabs-mode:nil
# End:
# vim: set expandtab tabstop=2 shiftwidth=2:
