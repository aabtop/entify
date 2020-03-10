#include "platform_window/platform_window.h"

#include <cassert>
#include <condition_variable>
#include <mutex>
#include <thread>
#include <windows.h>
#include <iostream>

namespace {
const int kWindowWidth = 1920;
const int kWindowHeight = 1080;

class Window {
 public:
  Window(const char* title, PlatformWindowEventCallback event_callback,
         void* event_callback_context);
  ~Window();

  bool error() { return hwnd() == NULL; }
  HWND hwnd() {
    {
      std::unique_lock<std::mutex> lock(initialized_mutex_);
      while (!initialized_) {
        initialized_condition_.wait(lock);
      }
    }

    return hwnd_;
  }

 private:
  void Run(const char* title);
  void Start(const char* title);
  void Shutdown();

  void OnQuitRequest();

  bool initialized_ = false;
  PlatformWindowEventCallback event_callback_;
  void* context_;

  std::mutex initialized_mutex_;
  std::condition_variable initialized_condition_;
  std::thread thread_;

  HWND hwnd_ = NULL;

  friend long __stdcall HandleWindowEvent(HWND, unsigned int, WPARAM, LPARAM);
};

thread_local Window* s_window = nullptr;

long __stdcall HandleWindowEvent(
    HWND window, unsigned int msg, WPARAM wp, LPARAM lp) {
  switch(msg) {
    case WM_CLOSE: {
      s_window->OnQuitRequest();
      return 0L;
    } break;
    default: {
      return DefWindowProc(window, msg, wp, lp);
    }
  }
}

bool PumpNextWindowEvent() {
  MSG msg;
  if (!GetMessage(&msg, 0, 0, 0)) {
    return false;
  } else {
    DispatchMessage(&msg);
    return true;
  }
}

Window::Window(const char* title, PlatformWindowEventCallback event_callback,
               void* event_callback_context)
    : event_callback_(event_callback),
      context_(event_callback_context),
      thread_(&Window::Run, this, title) {}

Window::~Window() {
  if (!error()) {
    PostMessageA(hwnd(), WM_QUIT, 0, 0);
  }

  thread_.join();
}

void Window::Run(const char* title) {
  Start(title);
  if (hwnd_ == NULL) {
    return;
  }

  while (PumpNextWindowEvent()) {}

  Shutdown();
}

namespace {
const char* const CreateWindowClass() {
  const char* const myclass = "myclass" ;
  WNDCLASSEX wndclass = {
      sizeof(WNDCLASSEX),
      CS_DBLCLKS,
      HandleWindowEvent,
      0,
      0,
      GetModuleHandle(0),
      LoadIcon(0, IDI_APPLICATION),
      LoadCursor(0, IDC_ARROW),
      HBRUSH(COLOR_WINDOW + 1),
      0,
      myclass,
      LoadIcon(0,IDI_APPLICATION)
  };

  ATOM registered_class = RegisterClassEx(&wndclass);
  assert(registered_class);

  return myclass;
}
}  // namespace

void Window::Start(const char* title) {
  static auto window_class = CreateWindowClass();

  s_window = this;

  DWORD style = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX;

  hwnd_ = CreateWindowEx(
      0, window_class, title, style, CW_USEDEFAULT,
      CW_USEDEFAULT, kWindowWidth, kWindowHeight, 0, 0, GetModuleHandle(0),
      NULL);

  if (hwnd_ != NULL) {
    ShowWindow(hwnd_, SW_SHOWDEFAULT);
  }

  std::lock_guard<std::mutex> lock(initialized_mutex_);
  initialized_ = true;
  initialized_condition_.notify_all();
}

void Window::Shutdown() {
  DestroyWindow(hwnd_);
}

void Window::OnQuitRequest() {
  event_callback_(context_, kPlatformWindowEventQuitRequest, 0);
}

}  // namespace


PlatformWindow PlatformWindowMakeDefaultWindow(
    const char* title, PlatformWindowEventCallback event_callback,
    void* context) {
  auto window = std::make_unique<Window>(title, event_callback, context);
  if (window->error()) {
    return NULL;
  } else {
    return window.release();
  }
}

void PlatformWindowDestroyWindow(PlatformWindow platform_window) {
  assert(platform_window != NULL);
  delete static_cast<Window*>(platform_window);
}

NativeWindow PlatformWindowGetNativeWindow(PlatformWindow platform_window) {
  return static_cast<NativeWindow>(
      static_cast<Window*>(platform_window)->hwnd());
}

int32_t PlatformWindowGetWidth(PlatformWindow window) {
  return kWindowWidth;
}

int32_t PlatformWindowGetHeight(PlatformWindow window) {
  return kWindowHeight;
}
