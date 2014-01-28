// Copyright 2013 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/media/native_desktop_media_list.h"

#include <map>
#include <sstream>

#include "base/hash.h"
#include "base/logging.h"
#include "base/strings/utf_string_conversions.h"
#include "base/threading/sequenced_worker_pool.h"
#include "chrome/browser/media/desktop_media_list_observer.h"
#include "content/public/browser/browser_thread.h"
#include "grit/generated_resources.h"
#include "media/base/video_util.h"
#include "third_party/libyuv/include/libyuv/scale_argb.h"
#include "third_party/skia/include/core/SkBitmap.h"
#include "third_party/webrtc/modules/desktop_capture/desktop_frame.h"
#include "third_party/webrtc/modules/desktop_capture/screen_capturer.h"
#include "third_party/webrtc/modules/desktop_capture/window_capturer.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/gfx/skia_util.h"

using content::BrowserThread;
using content::DesktopMediaID;

namespace {

// Update the list every second.
const int kDefaultUpdatePeriod = 1000;

// Returns a hash of a DesktopFrame content to detect when image for a desktop
// media source has changed.
uint32 GetFrameHash(webrtc::DesktopFrame* frame) {
  int data_size = frame->stride() * frame->size().height();
  return base::SuperFastHash(reinterpret_cast<char*>(frame->data()), data_size);
}

gfx::ImageSkia ScaleDesktopFrame(scoped_ptr<webrtc::DesktopFrame> frame,
                                 gfx::Size size) {
  gfx::Rect scaled_rect = media::ComputeLetterboxRegion(
      gfx::Rect(0, 0, size.width(), size.height()),
      gfx::Size(frame->size().width(), frame->size().height()));

  SkBitmap result;
  result.setConfig(SkBitmap::kARGB_8888_Config,
                   scaled_rect.width(), scaled_rect.height(), 0,
                   kOpaque_SkAlphaType);
  result.allocPixels();
  result.lockPixels();

  uint8* pixels_data = reinterpret_cast<uint8*>(result.getPixels());
  libyuv::ARGBScale(frame->data(), frame->stride(),
                    frame->size().width(), frame->size().height(),
                    pixels_data, result.rowBytes(),
                    scaled_rect.width(), scaled_rect.height(),
                    libyuv::kFilterBilinear);

  // Set alpha channel values to 255 for all pixels.
  // TODO(sergeyu): Fix screen/window capturers to capture alpha channel and
  // remove this code. Currently screen/window capturers (at least some
  // implementations) only capture R, G and B channels and set Alpha to 0.
  // crbug.com/264424
  for (int y = 0; y < result.height(); ++y) {
    for (int x = 0; x < result.width(); ++x) {
      pixels_data[result.rowBytes() * y + x * result.bytesPerPixel() + 3] =
          0xff;
    }
  }

  result.unlockPixels();

  return gfx::ImageSkia::CreateFrom1xBitmap(result);
}

}  // namespace

NativeDesktopMediaList::SourceDescription::SourceDescription(
    DesktopMediaID id,
    const base::string16& name)
    : id(id),
      name(name) {
}

class NativeDesktopMediaList::Worker
    : public webrtc::DesktopCapturer::Callback {
 public:
  Worker(base::WeakPtr<NativeDesktopMediaList> media_list,
         scoped_ptr<webrtc::ScreenCapturer> screen_capturer,
         scoped_ptr<webrtc::WindowCapturer> window_capturer);
  virtual ~Worker();

  void Refresh(const gfx::Size& thumbnail_size,
               content::DesktopMediaID::Id view_dialog_id);

 private:
  typedef std::map<DesktopMediaID, uint32> ImageHashesMap;

  // webrtc::DesktopCapturer::Callback interface.
  virtual webrtc::SharedMemory* CreateSharedMemory(size_t size) OVERRIDE;
  virtual void OnCaptureCompleted(webrtc::DesktopFrame* frame) OVERRIDE;

  base::WeakPtr<NativeDesktopMediaList> media_list_;

  scoped_ptr<webrtc::ScreenCapturer> screen_capturer_;
  scoped_ptr<webrtc::WindowCapturer> window_capturer_;

  scoped_ptr<webrtc::DesktopFrame> current_frame_;

  ImageHashesMap image_hashes_;

  DISALLOW_COPY_AND_ASSIGN(Worker);
};

NativeDesktopMediaList::Worker::Worker(
    base::WeakPtr<NativeDesktopMediaList> media_list,
    scoped_ptr<webrtc::ScreenCapturer> screen_capturer,
    scoped_ptr<webrtc::WindowCapturer> window_capturer)
    : media_list_(media_list),
      screen_capturer_(screen_capturer.Pass()),
      window_capturer_(window_capturer.Pass()) {
  if (screen_capturer_)
    screen_capturer_->Start(this);
  if (window_capturer_)
    window_capturer_->Start(this);
}

NativeDesktopMediaList::Worker::~Worker() {}

void NativeDesktopMediaList::Worker::Refresh(
    const gfx::Size& thumbnail_size,
    content::DesktopMediaID::Id view_dialog_id) {
  std::vector<SourceDescription> sources;

  if (screen_capturer_) {
    webrtc::ScreenCapturer::ScreenList screens;
    if (screen_capturer_->GetScreenList(&screens)) {
      bool mutiple_screens = screens.size() > 1;
      base::string16 title;
      for (size_t i = 0; i < screens.size(); ++i) {
        if (mutiple_screens) {
          title = l10n_util::GetStringFUTF16Int(
              IDS_DESKTOP_MEDIA_PICKER_MULTIPLE_SCREEN_NAME,
              static_cast<int>(i + 1));
        } else {
          title = l10n_util::GetStringUTF16(
              IDS_DESKTOP_MEDIA_PICKER_SINGLE_SCREEN_NAME);
        }
        sources.push_back(SourceDescription(DesktopMediaID(
            DesktopMediaID::TYPE_SCREEN, screens[i].id), title));
      }
    }
  }

  if (window_capturer_) {
    webrtc::WindowCapturer::WindowList windows;
    if (window_capturer_->GetWindowList(&windows)) {
      for (webrtc::WindowCapturer::WindowList::iterator it = windows.begin();
           it != windows.end(); ++it) {
        // Skip the picker dialog window.
        if (it->id != view_dialog_id) {
          sources.push_back(SourceDescription(
              DesktopMediaID(DesktopMediaID::TYPE_WINDOW, it->id),
              base::UTF8ToUTF16(it->title)));
        }
      }
    }
  }

  // Sort the list of sources so that they appear in a predictable order.
  std::sort(sources.begin(), sources.end(), CompareSources);

  // Update list of windows before updating thumbnails.
  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&NativeDesktopMediaList::OnSourcesList,
                 media_list_, sources));

  ImageHashesMap new_image_hashes;

  // Get a thumbnail for each source.
  for (size_t i = 0; i < sources.size(); ++i) {
    SourceDescription& source = sources[i];
    switch (source.id.type) {
      case DesktopMediaID::TYPE_SCREEN:
        if (!screen_capturer_->SelectScreen(source.id.id))
          continue;
        screen_capturer_->Capture(webrtc::DesktopRegion());
        DCHECK(current_frame_);
        break;

      case DesktopMediaID::TYPE_WINDOW:
        if (!window_capturer_->SelectWindow(source.id.id))
          continue;
        window_capturer_->Capture(webrtc::DesktopRegion());
        break;

      default:
        NOTREACHED();
    }

    // Expect that DesktopCapturer to always captures frames synchronously.
    // |current_frame_| may be NULL if capture failed (e.g. because window has
    // been closed).
    if (current_frame_) {
      uint32 frame_hash = GetFrameHash(current_frame_.get());
      new_image_hashes[source.id] = frame_hash;

      // Scale the image only if it has changed.
      ImageHashesMap::iterator it = image_hashes_.find(source.id);
      if (it == image_hashes_.end() || it->second != frame_hash) {
        gfx::ImageSkia thumbnail =
            ScaleDesktopFrame(current_frame_.Pass(), thumbnail_size);
        BrowserThread::PostTask(
            BrowserThread::UI, FROM_HERE,
            base::Bind(&NativeDesktopMediaList::OnSourceThumbnail,
                        media_list_, i, thumbnail));
      }
    }
  }

  image_hashes_.swap(new_image_hashes);

  BrowserThread::PostTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&NativeDesktopMediaList::OnRefreshFinished, media_list_));
}

webrtc::SharedMemory* NativeDesktopMediaList::Worker::CreateSharedMemory(
    size_t size) {
  return NULL;
}

void NativeDesktopMediaList::Worker::OnCaptureCompleted(
    webrtc::DesktopFrame* frame) {
  current_frame_.reset(frame);
}

NativeDesktopMediaList::NativeDesktopMediaList(
    scoped_ptr<webrtc::ScreenCapturer> screen_capturer,
    scoped_ptr<webrtc::WindowCapturer> window_capturer)
    : screen_capturer_(screen_capturer.Pass()),
      window_capturer_(window_capturer.Pass()),
      update_period_(base::TimeDelta::FromMilliseconds(kDefaultUpdatePeriod)),
      thumbnail_size_(100, 100),
      view_dialog_id_(-1),
      observer_(NULL),
      weak_factory_(this) {
  base::SequencedWorkerPool* worker_pool = BrowserThread::GetBlockingPool();
  capture_task_runner_ = worker_pool->GetSequencedTaskRunner(
      worker_pool->GetSequenceToken());
}

NativeDesktopMediaList::~NativeDesktopMediaList() {
  capture_task_runner_->DeleteSoon(FROM_HERE, worker_.release());
}

void NativeDesktopMediaList::SetUpdatePeriod(base::TimeDelta period) {
  DCHECK(!observer_);
  update_period_ = period;
}

void NativeDesktopMediaList::SetThumbnailSize(
    const gfx::Size& thumbnail_size) {
  thumbnail_size_ = thumbnail_size;
}

void NativeDesktopMediaList::SetViewDialogWindowId(
    content::DesktopMediaID::Id dialog_id) {
  view_dialog_id_ = dialog_id;
}

void NativeDesktopMediaList::StartUpdating(DesktopMediaListObserver* observer) {
  DCHECK(!observer_);
  DCHECK(screen_capturer_ || window_capturer_);

  observer_ = observer;

  worker_.reset(new Worker(weak_factory_.GetWeakPtr(),
                           screen_capturer_.Pass(), window_capturer_.Pass()));
  Refresh();
}

int NativeDesktopMediaList::GetSourceCount() const {
  return sources_.size();
}

const DesktopMediaList::Source& NativeDesktopMediaList::GetSource(
    int index) const {
  return sources_[index];
}

// static
bool NativeDesktopMediaList::CompareSources(const SourceDescription& a,
                                            const SourceDescription& b) {
  return a.id < b.id;
}

void NativeDesktopMediaList::Refresh() {
  capture_task_runner_->PostTask(
      FROM_HERE, base::Bind(&Worker::Refresh, base::Unretained(worker_.get()),
                            thumbnail_size_, view_dialog_id_));
}

void NativeDesktopMediaList::OnSourcesList(
    const std::vector<SourceDescription>& new_sources) {
  // Step through |new_sources| adding and removing entries from |sources_|, and
  // notifying the |observer_|, until two match. Requires that |sources| and
  // |sources_| have the same ordering.
  size_t pos = 0;
  while (pos < sources_.size() || pos < new_sources.size()) {
    // If |sources_[pos]| is not in |new_sources| then remove it.
    if (pos < sources_.size() &&
        (pos == new_sources.size() || sources_[pos].id < new_sources[pos].id)) {
      sources_.erase(sources_.begin() + pos);
      observer_->OnSourceRemoved(pos);
      continue;
    }

    if (pos == sources_.size() || !(sources_[pos].id == new_sources[pos].id)) {
      sources_.insert(sources_.begin() + pos, Source());
      sources_[pos].id = new_sources[pos].id;
      sources_[pos].name = new_sources[pos].name;
      observer_->OnSourceAdded(pos);
    } else if (sources_[pos].name != new_sources[pos].name) {
      sources_[pos].name = new_sources[pos].name;
      observer_->OnSourceNameChanged(pos);
    }

    ++pos;
  }

  DCHECK_EQ(new_sources.size(), sources_.size());
}

void NativeDesktopMediaList::OnSourceThumbnail(
    int index,
    const gfx::ImageSkia& image) {
  DCHECK_LT(index, static_cast<int>(sources_.size()));
  sources_[index].thumbnail = image;
  observer_->OnSourceThumbnailChanged(index);
}

void NativeDesktopMediaList::OnRefreshFinished() {
  BrowserThread::PostDelayedTask(
      BrowserThread::UI, FROM_HERE,
      base::Bind(&NativeDesktopMediaList::Refresh,
                 weak_factory_.GetWeakPtr()),
      update_period_);
}
