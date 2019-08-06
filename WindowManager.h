#ifndef WM_WINDOWMANAGER_H
#define WM_WINDOWMANAGER_H

extern "C" {
#include <X11/Xlib.h>
#include <X11/Xutil.h>
}
#include <memory>
#include <string>
#include <mutex>
#include <ostream>
#include <sstream>
#include "spdlog/spdlog.h"

template <typename T>
struct Position {
    T x, y;

    Position() = default;
    Position(T _x, T _y)
            : x(_x), y(_y) {
    }
};

template <typename T>
struct Vector2D {
    T x, y;

    Vector2D() = default;
    Vector2D(T _x, T _y)
            : x(_x), y(_y) {
    }
};

template <typename T>
struct Size {
    T width, height;

    Size() = default;
    Size(T w, T h)
            : width(w), height(h) {
    }
};

template <typename T>
Vector2D<T> operator - (const Position<T>& a, const Position<T>& b) {
    return Vector2D<T>(a.x - b.x, a.y - b.y);
}

template <typename T>
Position<T> operator + (const Position<T>& a, const Vector2D<T> &v) {
    return Position<T>(a.x + v.x, a.y + v.y);
}

template <typename T>
Size<T> operator + (const Size<T>& a, const Vector2D<T> &v) {
    return Size<T>(a.width + v.x, a.height + v.y);
}

class WindowManager {
public:
    static ::std::unique_ptr<WindowManager> Create(const std::string& title = std::string());

    ~WindowManager();

    void Run();
private:
    Display* display;
    const Window root;
    const Atom WM_PROTOCOLS;
    const Atom WM_DELETE_WINDOW;

    static bool wm_detected;
    static ::std::mutex wm_detected_mutex;

    ::std::unordered_map<Window, Window> clients;

    Position<int> drag_start_pos;
    Position<int> drag_start_frame_pos;
    Size<int> drag_start_frame_size;

    void OnCreateNotify(const XCreateWindowEvent& e);
    void OnDestroyNotify(const XDestroyWindowEvent& e);
    void OnReparentNotify(const XReparentEvent& e);
    void OnMapNotify(const XMapEvent& e);
    void OnUnmapNotify(const XUnmapEvent& e);
    void OnConfigureNotify(const XConfigureEvent& e);
    void OnMapRequest(const XMapRequestEvent& e);
    void OnConfigureRequest(const XConfigureRequestEvent& e);
    void OnButtonPress(const XButtonEvent& e);
    void OnButtonRelease(const XButtonEvent& e);
    void OnMotionNotify(const XMotionEvent& e);
    void OnKeyPress(const XKeyEvent& e);
    void OnKeyRelease(const XKeyEvent& e);

    static int OnXError(Display* display, XErrorEvent* e);
    static int OnWMDetected(Display* display, XErrorEvent* e);

    explicit WindowManager(Display* display);
    void Frame(Window w, bool older_from_wm);

};

#endif //WM_WINDOWMANAGER_H
