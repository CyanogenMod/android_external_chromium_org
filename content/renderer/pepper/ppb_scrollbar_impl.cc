// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "content/renderer/pepper/ppb_scrollbar_impl.h"

#include "base/bind.h"
#include "base/logging.h"
#include "base/message_loop/message_loop.h"
#include "content/renderer/pepper/common.h"
#include "content/renderer/pepper/event_conversion.h"
#include "content/renderer/pepper/host_globals.h"
#include "content/renderer/pepper/pepper_plugin_instance_impl.h"
#include "content/renderer/pepper/plugin_module.h"
#include "content/renderer/pepper/ppb_image_data_impl.h"
#include "ppapi/c/dev/ppp_scrollbar_dev.h"
#include "ppapi/c/dev/ppp_widget_dev.h"
#include "ppapi/shared_impl/ppb_input_event_shared.h"
#include "ppapi/thunk/enter.h"
#include "ppapi/thunk/ppb_input_event_api.h"
#include "ppapi/thunk/thunk.h"
#include "skia/ext/platform_canvas.h"
#include "third_party/WebKit/public/platform/WebCanvas.h"
#include "third_party/WebKit/public/platform/WebRect.h"
#include "third_party/WebKit/public/platform/WebVector.h"
#include "third_party/WebKit/public/web/WebInputEvent.h"
#include "third_party/WebKit/public/web/WebPluginScrollbar.h"

#if defined(OS_WIN)
#include "base/win/windows_version.h"
#endif

using ppapi::thunk::EnterResourceNoLock;
using ppapi::thunk::PPB_ImageData_API;
using ppapi::thunk::PPB_InputEvent_API;
using ppapi::thunk::PPB_Scrollbar_API;
using ppapi::thunk::PPB_Widget_API;
using WebKit::WebInputEvent;
using WebKit::WebRect;
using WebKit::WebScrollbar;
using WebKit::WebPluginScrollbar;

namespace content {

// static
PP_Resource PPB_Scrollbar_Impl::Create(PP_Instance instance,
                                       bool vertical) {
  scoped_refptr<PPB_Scrollbar_Impl> scrollbar(
      new PPB_Scrollbar_Impl(instance));
  scrollbar->Init(vertical);
  return scrollbar->GetReference();
}

PPB_Scrollbar_Impl::PPB_Scrollbar_Impl(PP_Instance instance)
    : Resource(ppapi::OBJECT_IS_IMPL, instance),
      scale_(1.0f),
      weak_ptr_factory_(this) {
  memset(&location_, 0, sizeof(location_));
}

PPB_Scrollbar_Impl::~PPB_Scrollbar_Impl() {
}

void PPB_Scrollbar_Impl::Init(bool vertical) {
  PepperPluginInstanceImpl* plugin_instance =
      HostGlobals::Get()->GetInstance(pp_instance());
  if (!plugin_instance)
    return;
  scrollbar_.reset(WebPluginScrollbar::createForPlugin(
      vertical ? WebScrollbar::Vertical : WebScrollbar::Horizontal,
      plugin_instance->container(),
      static_cast<WebKit::WebPluginScrollbarClient*>(this)));
}

PPB_Scrollbar_API* PPB_Scrollbar_Impl::AsPPB_Scrollbar_API() {
  return this;
}

PPB_Widget_API* PPB_Scrollbar_Impl::AsPPB_Widget_API() {
  return this;
}

void PPB_Scrollbar_Impl::InstanceWasDeleted() {
  scrollbar_.reset();
}

uint32_t PPB_Scrollbar_Impl::GetThickness() {
  return WebPluginScrollbar::defaultThickness();
}

bool PPB_Scrollbar_Impl::IsOverlay() {
  return scrollbar_->isOverlay();
}

uint32_t PPB_Scrollbar_Impl::GetValue() {
  return scrollbar_->value();
}

void PPB_Scrollbar_Impl::SetValue(uint32_t value) {
  if (scrollbar_)
    scrollbar_->setValue(value);
}

void PPB_Scrollbar_Impl::SetDocumentSize(uint32_t size) {
  if (scrollbar_)
    scrollbar_->setDocumentSize(size);
}

void PPB_Scrollbar_Impl::SetTickMarks(const PP_Rect* tick_marks,
                                      uint32_t count) {
  if (!scrollbar_)
    return;
  tickmarks_.resize(count);
  for (uint32 i = 0; i < count; ++i) {
    tickmarks_[i] = WebRect(tick_marks[i].point.x,
                            tick_marks[i].point.y,
                            tick_marks[i].size.width,
                            tick_marks[i].size.height);;
  }
  Invalidate(location_);
}

void PPB_Scrollbar_Impl::ScrollBy(PP_ScrollBy_Dev unit, int32_t multiplier) {
  if (!scrollbar_)
    return;

  WebScrollbar::ScrollDirection direction = multiplier >= 0 ?
      WebScrollbar::ScrollForward : WebScrollbar::ScrollBackward;
  float fmultiplier = 1.0;

  WebScrollbar::ScrollGranularity granularity;
  if (unit == PP_SCROLLBY_LINE) {
    granularity = WebScrollbar::ScrollByLine;
  } else if (unit == PP_SCROLLBY_PAGE) {
    granularity = WebScrollbar::ScrollByPage;
  } else if (unit == PP_SCROLLBY_DOCUMENT) {
    granularity = WebScrollbar::ScrollByDocument;
  } else {
    granularity = WebScrollbar::ScrollByPixel;
    fmultiplier = static_cast<float>(multiplier);
    if (fmultiplier < 0)
      fmultiplier *= -1;
  }
  scrollbar_->scroll(direction, granularity, fmultiplier);
}

PP_Bool PPB_Scrollbar_Impl::Paint(const PP_Rect* pp_rect,
                                  PP_Resource image_id) {
  EnterResourceNoLock<PPB_ImageData_API> enter(image_id, true);
  if (enter.failed())
    return PP_FALSE;
  PPB_ImageData_Impl* image = static_cast<PPB_ImageData_Impl*>(enter.object());

  gfx::Rect rect(pp_rect->point.x, pp_rect->point.y,
                 pp_rect->size.width, pp_rect->size.height);
  ImageDataAutoMapper mapper(image);
  skia::PlatformCanvas* canvas = image->GetPlatformCanvas();
  if (!canvas || !scrollbar_)
    return PP_FALSE;
  canvas->save();
  canvas->scale(scale_, scale_);
  scrollbar_->paint(canvas, rect);
  canvas->restore();

#if defined(OS_WIN)
  if (base::win::GetVersion() == base::win::VERSION_XP)
    skia::MakeOpaque(canvas, rect.x(), rect.y(), rect.width(), rect.height());
#endif

  return PP_TRUE;
}

PP_Bool PPB_Scrollbar_Impl::HandleEvent(
    PP_Resource pp_input_event) {
  EnterResourceNoLock<PPB_InputEvent_API> enter(pp_input_event, true);
  if (enter.failed())
    return PP_FALSE;
  ppapi::InputEventData data = enter.object()->GetInputEventData();
  scoped_ptr<WebInputEvent> web_input_event(CreateWebInputEvent(data));
  if (!web_input_event.get() || !scrollbar_)
    return PP_FALSE;

  return PP_FromBool(scrollbar_->handleInputEvent(*web_input_event.get()));
}

PP_Bool PPB_Scrollbar_Impl::GetLocation(PP_Rect* location) {
  *location = location_;
  return PP_TRUE;
}

void PPB_Scrollbar_Impl::SetLocation(const PP_Rect* location) {
  if (!scrollbar_)
    return;
  scrollbar_->setLocation(WebRect(location->point.x,
                                  location->point.y,
                                  location->size.width,
                                  location->size.height));
}

void PPB_Scrollbar_Impl::SetScale(float scale) {
  scale_ = scale;
}

void PPB_Scrollbar_Impl::Invalidate(const PP_Rect& dirty) {
  PepperPluginInstanceImpl* plugin_instance =
      HostGlobals::Get()->GetInstance(pp_instance());
  if (!plugin_instance)
    return;
  const PPP_Widget_Dev* widget = static_cast<const PPP_Widget_Dev*>(
      plugin_instance->module()->GetPluginInterface(PPP_WIDGET_DEV_INTERFACE));
  if (!widget)
    return;
  widget->Invalidate(pp_instance(), pp_resource(), &dirty);
}

void PPB_Scrollbar_Impl::valueChanged(WebKit::WebPluginScrollbar* scrollbar) {
  PluginModule* plugin_module =
      HostGlobals::Get()->GetInstance(pp_instance())->module();
  if (!plugin_module)
    return;

  const PPP_Scrollbar_Dev* ppp_scrollbar =
      static_cast<const PPP_Scrollbar_Dev*>(plugin_module->GetPluginInterface(
          PPP_SCROLLBAR_DEV_INTERFACE));
  if (!ppp_scrollbar) {
    // Try the old version. This is ok because the old interface is a subset of
    // the new one, and ValueChanged didn't change.
    ppp_scrollbar =
        static_cast<const PPP_Scrollbar_Dev*>(plugin_module->GetPluginInterface(
            PPP_SCROLLBAR_DEV_INTERFACE_0_2));
    if (!ppp_scrollbar)
      return;
  }
  ppp_scrollbar->ValueChanged(pp_instance(), pp_resource(),
                              scrollbar_->value());
}

void PPB_Scrollbar_Impl::overlayChanged(WebPluginScrollbar* scrollbar) {
  PluginModule* plugin_module =
      HostGlobals::Get()->GetInstance(pp_instance())->module();
  if (!plugin_module)
    return;

  const PPP_Scrollbar_Dev* ppp_scrollbar =
      static_cast<const PPP_Scrollbar_Dev*>(plugin_module->GetPluginInterface(
          PPP_SCROLLBAR_DEV_INTERFACE));
  if (!ppp_scrollbar)
    return;
  ppp_scrollbar->OverlayChanged(pp_instance(), pp_resource(),
                                PP_FromBool(IsOverlay()));
}

void PPB_Scrollbar_Impl::invalidateScrollbarRect(
    WebKit::WebPluginScrollbar* scrollbar,
    const WebKit::WebRect& rect) {
  gfx::Rect gfx_rect(rect.x,
                     rect.y,
                     rect.width,
                     rect.height);
  dirty_.Union(gfx_rect);
  // Can't call into the client to tell them about the invalidate right away,
  // since the PPB_Scrollbar_Impl code is still in the middle of updating its
  // internal state.
  // Note: we use a WeakPtrFactory here so that a lingering callback can not
  // modify the lifetime of this object. Otherwise, WebKit::WebPluginScrollbar
  // could outlive WebKit::WebPluginContainer, which is against its contract.
  base::MessageLoop::current()->PostTask(
      FROM_HERE,
      base::Bind(&PPB_Scrollbar_Impl::NotifyInvalidate,
                 weak_ptr_factory_.GetWeakPtr()));
}

void PPB_Scrollbar_Impl::getTickmarks(
    WebKit::WebPluginScrollbar* scrollbar,
    WebKit::WebVector<WebKit::WebRect>* tick_marks) const {
  if (tickmarks_.empty()) {
    WebRect* rects = NULL;
    tick_marks->assign(rects, 0);
  } else {
    tick_marks->assign(&tickmarks_[0], tickmarks_.size());
  }
}

void PPB_Scrollbar_Impl::NotifyInvalidate() {
  if (dirty_.IsEmpty())
    return;
  PP_Rect pp_rect;
  pp_rect.point.x = dirty_.x();
  pp_rect.point.y = dirty_.y();
  pp_rect.size.width = dirty_.width();
  pp_rect.size.height = dirty_.height();
  dirty_ = gfx::Rect();
  Invalidate(pp_rect);
}

}  // namespace content
