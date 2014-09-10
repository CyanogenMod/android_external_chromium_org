// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "android_webview/lib/main/aw_main_delegate.h"

#include "android_webview/browser/aw_content_browser_client.h"
#include "android_webview/browser/browser_view_renderer.h"
#include "android_webview/browser/gpu_memory_buffer_factory_impl.h"
#include "android_webview/browser/scoped_allow_wait_for_legacy_web_view_api.h"
#include "android_webview/lib/aw_browser_dependency_factory_impl.h"
#include "android_webview/native/aw_media_url_interceptor.h"
#include "android_webview/native/aw_quota_manager_bridge_impl.h"
#include "android_webview/native/aw_web_contents_view_delegate.h"
#include "android_webview/native/aw_web_preferences_populater_impl.h"
#include "android_webview/native/external_video_surface_container_impl.h"
#include "android_webview/renderer/aw_content_renderer_client.h"
#include "base/command_line.h"
#include "base/cpu.h"
#include "base/lazy_instance.h"
#include "base/logging.h"
#include "base/memory/scoped_ptr.h"
#include "base/threading/thread_restrictions.h"
#include "content/browser/media/android/browser_media_player_manager.h"
#include "content/public/browser/browser_main_runner.h"
#include "content/public/browser/browser_thread.h"
#include "content/public/common/content_switches.h"
#include "gpu/command_buffer/client/gl_in_process_context.h"
#include "media/base/media_switches.h"
#include "webkit/common/gpu/webgraphicscontext3d_in_process_command_buffer_impl.h"
#include "android_webview/native/aw_contents.h"
#include "content/shell/android/shell_descriptors.h"
#include "ui/base/resource/resource_bundle.h"
#include "ui/base/l10n/l10n_util_android.h"
#include "base/path_service.h"

namespace android_webview {

namespace {

// TODO(boliu): Remove this global Allow once the underlying issues are
// resolved - http://crbug.com/240453. See AwMainDelegate::RunProcess below.
base::LazyInstance<scoped_ptr<ScopedAllowWaitForLegacyWebViewApi> >
    g_allow_wait_in_ui_thread = LAZY_INSTANCE_INITIALIZER;

}

AwMainDelegate::AwMainDelegate()
    : gpu_memory_buffer_factory_(new GpuMemoryBufferFactoryImpl) {
}

AwMainDelegate::~AwMainDelegate() {
}

bool AwMainDelegate::BasicStartupComplete(int* exit_code) {
  content::SetContentClient(&content_client_);

  CommandLine* cl = CommandLine::ForCurrentProcess();
  bool zero_copy_disabled_by_switch = cl->HasSwitch(switches::kDisableZeroCopy);
  bool use_zero_copy = !zero_copy_disabled_by_switch &&
                       cl->HasSwitch(switches::kEnableZeroCopy) &&
                       gpu_memory_buffer_factory_.get()->Initialize();

  if (use_zero_copy) {
    cl->AppendSwitch(switches::kEnableZeroCopy);
  } else if (!zero_copy_disabled_by_switch) {
    cl->AppendSwitch(switches::kDisableZeroCopy);
  }

  content::BrowserMediaPlayerManager::RegisterMediaUrlInterceptor(
      new AwMediaUrlInterceptor());

  BrowserViewRenderer::CalculateTileMemoryPolicy(use_zero_copy);

  cl->AppendSwitch(switches::kEnableBeginFrameScheduling);
  cl->AppendSwitch(switches::kEnableImplSidePainting);

// SWE-feature-surfaceview
// SWE-feature-multiprocess
  // additional switches for non-SWE usage of AwContents
  if(!AwContents::isRunningMultiProcess() || !AwContents::isUsingSurfaceView()) {
    // WebView uses the Android system's scrollbars and overscroll glow.
    cl->AppendSwitch(switches::kDisableOverscrollEdgeEffect);

    // Not yet supported in single-process mode.
    cl->AppendSwitch(switches::kDisableSharedWorkers);
  }
// SWE-feature-surfaceview
// SWE-feature-multiprocess

  // File system API not supported (requires some new API; internal bug 6930981)
  cl->AppendSwitch(switches::kDisableFileSystem);

#if defined(VIDEO_HOLE)
  // Support EME/L1 with hole-punching.
  cl->AppendSwitch(switches::kMediaDrmEnableNonCompositing);
#endif

  // WebRTC hardware decoding is not supported, internal bug 15075307
  cl->AppendSwitch(switches::kDisableWebRtcHWDecoding);

  return false;
}

void AwMainDelegate::PreSandboxStartup() {
  // TODO(torne): When we have a separate renderer process, we need to handle
  // being passed open FDs for the resource paks here.
#if defined(ARCH_CPU_ARM_FAMILY)
  // Create an instance of the CPU class to parse /proc/cpuinfo and cache
  // cpu_brand info.
  base::CPU cpu_info;
#endif
// SWE-feature-multiprocess
  if (AwContents::isRunningMultiProcess())
    InitializeResourceBundle();
// SWE-feature-multiprocess
}

// SWE-feature-multiprocess
void AwMainDelegate::InitializeResourceBundle() {
#if defined(OS_ANDROID)
  // In the Android case, the renderer runs with a different UID and can never
  // access the file system.  So we are passed a file descriptor to the
  // ResourceBundle pak at launch time.
  int pak_fd =
      base::GlobalDescriptors::GetInstance()->MaybeGet(kShellPakDescriptor);
  if (pak_fd >= 0) {
    // This is clearly wrong. See crbug.com/330930
    ui::ResourceBundle::InitSharedInstanceWithPakFileRegion(
        base::File(pak_fd), base::MemoryMappedFile::Region::kWholeFile);
    ResourceBundle::GetSharedInstance().AddDataPackFromFile(
        base::File(pak_fd), ui::SCALE_FACTOR_100P);
    return;
  }
#endif

  base::FilePath pak_file;
#if defined(OS_MACOSX)
  pak_file = GetResourcesPakFilePath();
#else
  base::FilePath pak_dir;

#if defined(OS_ANDROID)
  bool got_path = PathService::Get(base::DIR_ANDROID_APP_DATA, &pak_dir);
  DCHECK(got_path);
  pak_dir = pak_dir.Append(FILE_PATH_LITERAL("paks"));
#else
  PathService::Get(base::DIR_MODULE, &pak_dir);
#endif

  pak_file = pak_dir.Append(FILE_PATH_LITERAL("webviewchromium.pak"));
#endif
  ui::ResourceBundle::InitSharedInstanceWithPakPath(pak_file);
}
// SWE-feature-multiprocess

void AwMainDelegate::SandboxInitialized(const std::string& process_type) {
  // TODO(torne): Adjust linux OOM score here.
}

int AwMainDelegate::RunProcess(
    const std::string& process_type,
    const content::MainFunctionParams& main_function_params) {
  if (process_type.empty()) {
    AwBrowserDependencyFactoryImpl::InstallInstance();

    browser_runner_.reset(content::BrowserMainRunner::Create());
    int exit_code = browser_runner_->Initialize(main_function_params);
    DCHECK(exit_code < 0);

    g_allow_wait_in_ui_thread.Get().reset(
        new ScopedAllowWaitForLegacyWebViewApi);

    // Return 0 so that we do NOT trigger the default behavior. On Android, the
    // UI message loop is managed by the Java application.
    return 0;
  }

  return -1;
}

void AwMainDelegate::ProcessExiting(const std::string& process_type) {
  // TODO(torne): Clean up resources when we handle them.

  logging::CloseLogFile();
}

content::ContentBrowserClient*
    AwMainDelegate::CreateContentBrowserClient() {
  content_browser_client_.reset(new AwContentBrowserClient(this));
  return content_browser_client_.get();
}

content::ContentRendererClient*
    AwMainDelegate::CreateContentRendererClient() {
  content_renderer_client_.reset(new AwContentRendererClient());
  return content_renderer_client_.get();
}

scoped_refptr<AwQuotaManagerBridge> AwMainDelegate::CreateAwQuotaManagerBridge(
    AwBrowserContext* browser_context) {
  return AwQuotaManagerBridgeImpl::Create(browser_context);
}

content::WebContentsViewDelegate* AwMainDelegate::CreateViewDelegate(
    content::WebContents* web_contents) {
  return AwWebContentsViewDelegate::Create(web_contents);
}

AwWebPreferencesPopulater* AwMainDelegate::CreateWebPreferencesPopulater() {
  return new AwWebPreferencesPopulaterImpl();
}

#if defined(VIDEO_HOLE)
content::ExternalVideoSurfaceContainer*
AwMainDelegate::CreateExternalVideoSurfaceContainer(
    content::WebContents* web_contents) {
  return new ExternalVideoSurfaceContainerImpl(web_contents);
}
#endif

}  // namespace android_webview
