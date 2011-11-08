// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ui/gfx/gl/gl_surface_wgl.h"

#include "base/logging.h"
#include "ui/gfx/gl/gl_bindings.h"

namespace gfx {

namespace {
const PIXELFORMATDESCRIPTOR kPixelFormatDescriptor = {
  sizeof(kPixelFormatDescriptor),    // Size of structure.
  1,                       // Default version.
  PFD_DRAW_TO_WINDOW |     // Window drawing support.
  PFD_SUPPORT_OPENGL |     // OpenGL support.
  PFD_DOUBLEBUFFER,        // Double buffering support (not stereo).
  PFD_TYPE_RGBA,           // RGBA color mode (not indexed).
  24,                      // 24 bit color mode.
  0, 0, 0, 0, 0, 0,        // Don't set RGB bits & shifts.
  8, 0,                    // 8 bit alpha
  0,                       // No accumulation buffer.
  0, 0, 0, 0,              // Ignore accumulation bits.
  0,                       // no z-buffer.
  0,                       // no stencil buffer.
  0,                       // No aux buffer.
  PFD_MAIN_PLANE,          // Main drawing plane (not overlay).
  0,                       // Reserved.
  0, 0, 0,                 // Layer masks ignored.
};

LRESULT CALLBACK IntermediateWindowProc(HWND window,
                                        UINT message,
                                        WPARAM w_param,
                                        LPARAM l_param) {
  switch (message) {
    case WM_ERASEBKGND:
      // Prevent windows from erasing the background.
      return 1;
    case WM_PAINT:
      // Do not paint anything.
      PAINTSTRUCT paint;
      if (BeginPaint(window, &paint))
        EndPaint(window, &paint);
      return 0;
    default:
      return DefWindowProc(window, message, w_param, l_param);
  }
}

class DisplayWGL {
 public:
  DisplayWGL()
      : module_handle_(0),
        window_class_(0),
        window_handle_(0),
        device_context_(0),
        pixel_format_(0) {
  }

  ~DisplayWGL() {
    if (window_handle_)
      DestroyWindow(window_handle_);
    if (window_class_)
      UnregisterClass(reinterpret_cast<wchar_t*>(window_class_),
                      module_handle_);
  }

  bool Init() {
    // We must initialize a GL context before we can bind to extension entry
    // points. This requires the device context for a window.
    if (!GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT |
                           GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
                           reinterpret_cast<wchar_t*>(IntermediateWindowProc),
                           &module_handle_)) {
      LOG(ERROR) << "GetModuleHandleEx failed.";
      return false;
    }

    WNDCLASS intermediate_class;
    intermediate_class.style = CS_OWNDC;
    intermediate_class.lpfnWndProc = IntermediateWindowProc;
    intermediate_class.cbClsExtra = 0;
    intermediate_class.cbWndExtra = 0;
    intermediate_class.hInstance = module_handle_;
    intermediate_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    intermediate_class.hCursor = LoadCursor(NULL, IDC_ARROW);
    intermediate_class.hbrBackground = NULL;
    intermediate_class.lpszMenuName = NULL;
    intermediate_class.lpszClassName = L"Intermediate GL Window";
    window_class_ = RegisterClass(&intermediate_class);
    if (!window_class_) {
      LOG(ERROR) << "RegisterClass failed.";
      return false;
    }

    window_handle_ = CreateWindow(
        reinterpret_cast<wchar_t*>(window_class_),
        L"",
        WS_OVERLAPPEDWINDOW,
        0, 0,
        100, 100,
        NULL,
        NULL,
        NULL,
        NULL);
    if (!window_handle_) {
      LOG(ERROR) << "CreateWindow failed.";
      return false;
    }

    device_context_ = GetDC(window_handle_);
    pixel_format_ = ChoosePixelFormat(device_context_,
                                      &kPixelFormatDescriptor);
    if (pixel_format_ == 0) {
      LOG(ERROR) << "Unable to get the pixel format for GL context.";
      return false;
    }
    if (!SetPixelFormat(device_context_,
                        pixel_format_,
                        &kPixelFormatDescriptor)) {
      LOG(ERROR) << "Unable to set the pixel format for temporary GL context.";
      return false;
    }

    return true;
  }

  ATOM window_class() const { return window_class_; }
  HDC device_context() const { return device_context_; }
  int pixel_format() const { return pixel_format_; }

 private:
  HINSTANCE module_handle_;
  ATOM window_class_;
  HWND window_handle_;
  HDC device_context_;
  int pixel_format_;
};
DisplayWGL* g_display;
}  // namespace

GLSurfaceWGL::GLSurfaceWGL() {
}

GLSurfaceWGL::~GLSurfaceWGL() {
}

void* GLSurfaceWGL::GetDisplay() {
  return GetDisplayDC();
}

bool GLSurfaceWGL::InitializeOneOff() {
  static bool initialized = false;
  if (initialized)
    return true;

  DCHECK(g_display == NULL);
  g_display = new DisplayWGL;
  if (!g_display->Init()) {
    delete g_display;
    g_display = NULL;
    return false;
  }

  initialized = true;
  return true;
}

HDC GLSurfaceWGL::GetDisplayDC() {
  return g_display->device_context();
}

NativeViewGLSurfaceWGL::NativeViewGLSurfaceWGL(gfx::PluginWindowHandle window)
    : window_(window),
      child_window_(NULL),
      device_context_(NULL) {
  DCHECK(window);
}

NativeViewGLSurfaceWGL::~NativeViewGLSurfaceWGL() {
  Destroy();
}

bool NativeViewGLSurfaceWGL::Initialize() {
  DCHECK(!device_context_);

  RECT rect;
  if (!GetClientRect(window_, &rect)) {
    LOG(ERROR) << "GetClientRect failed.\n";
    Destroy();
    return false;
  }

  // Create a child window. WGL has problems using a window handle owned by
  // another process.
  child_window_ = CreateWindow(
      reinterpret_cast<wchar_t*>(g_display->window_class()),
      L"",
      WS_CHILDWINDOW | WS_DISABLED | WS_VISIBLE,
      0, 0,
      rect.right - rect.left,
      rect.bottom - rect.top,
      window_,
      NULL,
      NULL,
      NULL);
  if (!child_window_) {
    LOG(ERROR) << "CreateWindow failed.\n";
    Destroy();
    return false;
  }

  // The GL context will render to this window.
  device_context_ = GetDC(child_window_);
  if (!device_context_) {
    LOG(ERROR) << "Unable to get device context for window.";
    Destroy();
    return false;
  }

  if (!SetPixelFormat(device_context_,
                      g_display->pixel_format(),
                      &kPixelFormatDescriptor)) {
    LOG(ERROR) << "Unable to set the pixel format for GL context.";
    Destroy();
    return false;
  }

  return true;
}

void NativeViewGLSurfaceWGL::Destroy() {
  if (child_window_ && device_context_)
    ReleaseDC(child_window_, device_context_);

  if (child_window_)
    DestroyWindow(child_window_);

  child_window_ = NULL;
  device_context_ = NULL;
}

bool NativeViewGLSurfaceWGL::IsOffscreen() {
  return false;
}

bool NativeViewGLSurfaceWGL::SwapBuffers() {
  // Resize the child window to match the parent before swapping. Do not repaint
  // it as it moves.
  RECT rect;
  if (!GetClientRect(window_, &rect))
    return false;
  if (!MoveWindow(child_window_,
                  0, 0,
                  rect.right - rect.left,
                  rect.bottom - rect.top,
                  FALSE)) {
    return false;
  }

  DCHECK(device_context_);
  return ::SwapBuffers(device_context_) == TRUE;
}

gfx::Size NativeViewGLSurfaceWGL::GetSize() {
  RECT rect;
  BOOL result = GetClientRect(child_window_, &rect);
  DCHECK(result);
  return gfx::Size(rect.right - rect.left, rect.bottom - rect.top);
}

void* NativeViewGLSurfaceWGL::GetHandle() {
  return device_context_;
}

PbufferGLSurfaceWGL::PbufferGLSurfaceWGL(const gfx::Size& size)
    : size_(size),
      device_context_(NULL),
      pbuffer_(NULL) {
}

PbufferGLSurfaceWGL::~PbufferGLSurfaceWGL() {
  Destroy();
}

bool PbufferGLSurfaceWGL::Initialize() {
  DCHECK(!device_context_);

  if (!wglCreatePbufferARB) {
    LOG(ERROR) << "wglCreatePbufferARB not available.";
    Destroy();
    return false;
  }

  const int kNoAttributes[] = { 0 };
  pbuffer_ = wglCreatePbufferARB(g_display->device_context(),
                                 g_display->pixel_format(),
                                 size_.width(), size_.height(),
                                 kNoAttributes);

  if (!pbuffer_) {
    LOG(ERROR) << "Unable to create pbuffer.";
    Destroy();
    return false;
  }

  device_context_ = wglGetPbufferDCARB(static_cast<HPBUFFERARB>(pbuffer_));
  if (!device_context_) {
    LOG(ERROR) << "Unable to get pbuffer device context.";
    Destroy();
    return false;
  }

  return true;
}

void PbufferGLSurfaceWGL::Destroy() {
  if (pbuffer_ && device_context_)
    wglReleasePbufferDCARB(static_cast<HPBUFFERARB>(pbuffer_), device_context_);

  device_context_ = NULL;

  if (pbuffer_) {
    wglDestroyPbufferARB(static_cast<HPBUFFERARB>(pbuffer_));
    pbuffer_ = NULL;
  }
}

bool PbufferGLSurfaceWGL::IsOffscreen() {
  return true;
}

bool PbufferGLSurfaceWGL::SwapBuffers() {
  NOTREACHED() << "Attempted to call SwapBuffers on a pbuffer.";
  return false;
}

gfx::Size PbufferGLSurfaceWGL::GetSize() {
  return size_;
}

void* PbufferGLSurfaceWGL::GetHandle() {
  return device_context_;
}

}  // namespace gfx
