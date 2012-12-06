// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Multiply-included file, hence no include guard.

#include "content/common/child_process_messages.h"

#include "content/common/accessibility_messages.h"
#include "content/common/appcache_messages.h"
#include "content/common/browser_plugin_messages.h"
#include "content/common/cc_messages.h"
#include "content/common/clipboard_messages.h"
#include "content/common/database_messages.h"
#include "content/common/desktop_notification_messages.h"
#include "content/common/device_motion_messages.h"
#include "content/common/device_orientation_messages.h"
#include "content/common/devtools_messages.h"
#include "content/common/dom_storage_messages.h"
#include "content/common/drag_messages.h"
#include "content/common/drag_traits.h"
#include "content/common/file_utilities_messages.h"
#include "content/common/fileapi/file_system_messages.h"
#include "content/common/fileapi/webblob_messages.h"
#include "content/common/gamepad_messages.h"
#include "content/common/geolocation_messages.h"
#include "content/common/gpu/gpu_messages.h"
#include "content/common/hyphenator_messages.h"
#include "content/common/icon_messages.h"
#include "content/common/indexed_db/indexed_db_messages.h"
#include "content/common/intents_messages.h"
#include "content/common/java_bridge_messages.h"
#include "content/common/media/audio_messages.h"
#include "content/common/media/media_player_messages.h"
#include "content/common/media/media_stream_messages.h"
#include "content/common/media/video_capture_messages.h"
#include "content/common/mime_registry_messages.h"
#include "content/common/p2p_messages.h"
#include "content/common/pepper_messages.h"
#include "content/common/plugin_messages.h"
#include "content/common/quota_messages.h"
#include "content/common/resource_messages.h"
#include "content/common/socket_stream_messages.h"
#include "content/common/speech_recognition_messages.h"
#include "content/common/text_input_client_messages.h"
#include "content/common/utility_messages.h"
#include "content/common/view_messages.h"
#include "content/common/worker_messages.h"
