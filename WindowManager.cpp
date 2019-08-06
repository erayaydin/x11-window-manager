#include "WindowManager.h"

using ::std::mutex;
using ::std::ostringstream;
using ::std::string;
using ::std::max;
using ::std::unique_ptr;

bool WindowManager::wm_detected;
mutex WindowManager::wm_detected_mutex;

unique_ptr<WindowManager> WindowManager::Create(const string& title) {
    const char* title_str = title.empty() ? nullptr : title.c_str();
    Display* display = XOpenDisplay(title_str);

    if(display == nullptr){
        spdlog::error("Failed to open X display: {}", XDisplayName(title_str));
        return nullptr;
    }

    return unique_ptr<WindowManager>(new WindowManager(display));
}

WindowManager::WindowManager(Display* display)
    : root(DefaultRootWindow(display)),
    WM_PROTOCOLS(XInternAtom(display, "WM_PROTOCOLS", false)),
    WM_DELETE_WINDOW(XInternAtom(display, "WM_DELETE_WINDOW", false)) {
    this->display = display;
    spdlog::info("Init!");
}

WindowManager::~WindowManager() {
    spdlog::info("Closing....");
    XCloseDisplay(display);
}

void WindowManager::Run() {
    ::std::lock_guard<mutex> lock(wm_detected_mutex);

    wm_detected = false;
    spdlog::info("Before error handler");
    XSetErrorHandler(&WindowManager::OnWMDetected);
    spdlog::info("After error handler");
    XSelectInput(display, root, SubstructureRedirectMask | SubstructureNotifyMask);
    spdlog::info("After select input");
    XSync(display, false);

    if(wm_detected){
        spdlog::error("Detected another window manager on Display {}", XDisplayString(display));
        return;
    }

    XSetErrorHandler(&WindowManager::OnXError);
    XGrabServer(display);
    Window returned_root, returned_parent;
    Window* top_level_windows;
    unsigned int num_top_level_windows;
    XQueryTree(display, root, &returned_root, &returned_parent, &top_level_windows, &num_top_level_windows);
    for(unsigned int i=0; i < num_top_level_windows; ++i) {
        Frame(top_level_windows[i], true);
    }
    XFree(top_level_windows);
    XUngrabServer(display);

    static const char* const X_EVENT_TYPE_NAMES[] = {
            "",
            "",
            "KeyPress",
            "KeyRelease",
            "ButtonPress",
            "ButtonRelease",
            "MotionNotify",
            "EnterNotify",
            "LeaveNotify",
            "FocusIn",
            "FocusOut",
            "KeymapNotify",
            "Expose",
            "GraphicsExpose",
            "NoExpose",
            "VisibilityNotify",
            "CreateNotify",
            "DestroyNotify",
            "UnmapNotify",
            "MapNotify",
            "MapRequest",
            "ReparentNotify",
            "ConfigureNotify",
            "ConfigureRequest",
            "GravityNotify",
            "ResizeRequest",
            "CirculateNotify",
            "CirculateRequest",
            "PropertyNotify",
            "SelectionClear",
            "SelectionRequest",
            "SelectionNotify",
            "ColormapNotify",
            "ClientMessage",
            "MappingNotify",
            "GeneralEvent",
    };

    for(;;) {
        XEvent e;
        XNextEvent(display, &e);

        switch(e.type) {
            case CreateNotify:
                OnCreateNotify(e.xcreatewindow);
                break;
            case DestroyNotify:
                OnDestroyNotify(e.xdestroywindow);
                break;
            case ReparentNotify:
                OnReparentNotify(e.xreparent);
                break;
            case MapNotify:
                OnMapNotify(e.xmap);
                break;
            case UnmapNotify:
                OnUnmapNotify(e.xunmap);
                break;
            case ConfigureNotify:
                OnConfigureNotify(e.xconfigure);
                break;
            case MapRequest:
                OnMapRequest(e.xmaprequest);
                break;
            case ConfigureRequest:
                OnConfigureRequest(e.xconfigurerequest);
                break;
            case ButtonPress:
                OnButtonPress(e.xbutton);
                break;
            case ButtonRelease:
                OnButtonRelease(e.xbutton);
                break;
            case MotionNotify:
                while(XCheckTypedWindowEvent(display, e.xmotion.window, MotionNotify, &e)) {}
                OnMotionNotify(e.xmotion);
                break;
            case KeyPress:
                OnKeyPress(e.xkey);
                break;
            case KeyRelease:
                OnKeyRelease(e.xkey);
                break;
            default:
                spdlog::warn("Ignoring event!");
        }
    }
}

int WindowManager::OnWMDetected(Display* display, XErrorEvent* e) {
    wm_detected = true;

    return 0;
}

int WindowManager::OnXError(Display* display, XErrorEvent* e) {

    static const char* const X_REQUEST_CODE_NAMES[] = {
            "",
            "CreateWindow",
            "ChangeWindowAttributes",
            "GetWindowAttributes",
            "DestroyWindow",
            "DestroySubwindows",
            "ChangeSaveSet",
            "ReparentWindow",
            "MapWindow",
            "MapSubwindows",
            "UnmapWindow",
            "UnmapSubwindows",
            "ConfigureWindow",
            "CirculateWindow",
            "GetGeometry",
            "QueryTree",
            "InternAtom",
            "GetAtomName",
            "ChangeProperty",
            "DeleteProperty",
            "GetProperty",
            "ListProperties",
            "SetSelectionOwner",
            "GetSelectionOwner",
            "ConvertSelection",
            "SendEvent",
            "GrabPointer",
            "UngrabPointer",
            "GrabButton",
            "UngrabButton",
            "ChangeActivePointerGrab",
            "GrabKeyboard",
            "UngrabKeyboard",
            "GrabKey",
            "UngrabKey",
            "AllowEvents",
            "GrabServer",
            "UngrabServer",
            "QueryPointer",
            "GetMotionEvents",
            "TranslateCoords",
            "WarpPointer",
            "SetInputFocus",
            "GetInputFocus",
            "QueryKeymap",
            "OpenFont",
            "CloseFont",
            "QueryFont",
            "QueryTextExtents",
            "ListFonts",
            "ListFontsWithInfo",
            "SetFontPath",
            "GetFontPath",
            "CreatePixmap",
            "FreePixmap",
            "CreateGC",
            "ChangeGC",
            "CopyGC",
            "SetDashes",
            "SetClipRectangles",
            "FreeGC",
            "ClearArea",
            "CopyArea",
            "CopyPlane",
            "PolyPoint",
            "PolyLine",
            "PolySegment",
            "PolyRectangle",
            "PolyArc",
            "FillPoly",
            "PolyFillRectangle",
            "PolyFillArc",
            "PutImage",
            "GetImage",
            "PolyText8",
            "PolyText16",
            "ImageText8",
            "ImageText16",
            "CreateColormap",
            "FreeColormap",
            "CopyColormapAndFree",
            "InstallColormap",
            "UninstallColormap",
            "ListInstalledColormaps",
            "AllocColor",
            "AllocNamedColor",
            "AllocColorCells",
            "AllocColorPlanes",
            "FreeColors",
            "StoreColors",
            "StoreNamedColor",
            "QueryColors",
            "LookupColor",
            "CreateCursor",
            "CreateGlyphCursor",
            "FreeCursor",
            "RecolorCursor",
            "QueryBestSize",
            "QueryExtension",
            "ListExtensions",
            "ChangeKeyboardMapping",
            "GetKeyboardMapping",
            "ChangeKeyboardControl",
            "GetKeyboardControl",
            "Bell",
            "ChangePointerControl",
            "GetPointerControl",
            "SetScreenSaver",
            "GetScreenSaver",
            "ChangeHosts",
            "ListHosts",
            "SetAccessControl",
            "SetCloseDownMode",
            "KillClient",
            "RotateProperties",
            "ForceScreenSaver",
            "SetPointerMapping",
            "GetPointerMapping",
            "SetModifierMapping",
            "GetModifierMapping",
            "NoOperation",
    };

    char error_text[1024];
    XGetErrorText(display, e->error_code, error_text, sizeof(error_text));
    spdlog::error("Received X error:\n\tRequest: {} - {}\n\tError Code: {} - {}\n\tResource ID: {}",
            int(e->request_code), X_REQUEST_CODE_NAMES[e->request_code], int(e->error_code), error_text, e->resourceid);

    return 0;
}

void WindowManager::OnCreateNotify(const XCreateWindowEvent &e) {

}

void WindowManager::OnDestroyNotify(const XDestroyWindowEvent &e) {

}

void WindowManager::OnReparentNotify(const XReparentEvent &e) {

}

void WindowManager::OnMapNotify(const XMapEvent &e) {

}

void WindowManager::OnUnmapNotify(const XUnmapEvent &e) {
    if(!clients.count(e.window)){
        spdlog::warn("Ignore UnmapNotify for non-client window {}", e.window);
        return;
    }

    if(e.event == root) {
        spdlog::info("Ignore Unmapnotify for reparented pre-existing window {}", e.window);
        return;
    }

    const Window frame = clients[e.window];
    XUnmapWindow(display, frame);
    XReparentWindow(display, e.window, root, 0, 0);
    XRemoveFromSaveSet(display, e.window);
    XDestroyWindow(display, frame);
    clients.erase(e.window);

    spdlog::info("Unframed window {} [{}]", e.window, frame);
}

void WindowManager::Frame(Window w, bool older_from_wm) {
    const unsigned int BORDER_WIDTH = 3;
    const unsigned long BORDER_COLOR = 0xff0000;
    const unsigned long BG_COLOR = 0x0000ff;

    if(clients.count(w))
        return;

    XWindowAttributes x_window_attrs;
    if(!XGetWindowAttributes(display, w, &x_window_attrs))
        return;

    if(older_from_wm && (x_window_attrs.override_redirect || x_window_attrs.map_state != IsViewable))
        return;

    const Window frame = XCreateSimpleWindow(display, root, x_window_attrs.x, x_window_attrs.y, x_window_attrs.width, x_window_attrs.height, BORDER_WIDTH, BORDER_COLOR, BG_COLOR);

    XSelectInput(display, frame, SubstructureRedirectMask | SubstructureNotifyMask);
    XAddToSaveSet(display, w);
    XReparentWindow(display, w, frame, 0, 0);
    XMapWindow(display, frame);
    clients[w] = frame;
    // Alt + Left Button = Move
    XGrabButton(display, Button1, Mod1Mask, w, false, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    // Alt + Right Button = Resize
    XGrabButton(display, Button3, Mod1Mask, w, false, ButtonPressMask | ButtonReleaseMask | ButtonMotionMask, GrabModeAsync, GrabModeAsync, None, None);
    // Alt + F4 = Kill
    XGrabKey(display, XKeysymToKeycode(display, XK_F4), Mod1Mask, w, false, GrabModeAsync, GrabModeAsync);
    // Alt + Tab = Switch Windows
    XGrabKey(display, XKeysymToKeycode(display, XK_Tab), Mod1Mask, w, false, GrabModeAsync, GrabModeAsync);

    spdlog::info("Framed Window {} [{}]", w, frame);
}

void WindowManager::OnConfigureNotify(const XConfigureEvent &e) {

}

void WindowManager::OnMapRequest(const XMapRequestEvent &e) {
    Frame(e.window, false);
    XMapWindow(display, e.window);
}

void WindowManager::OnConfigureRequest(const XConfigureRequestEvent &e) {
    XWindowChanges changes;
    changes.x = e.x;
    changes.y = e.y;
    changes.width = e.width;
    changes.height = e.height;
    changes.border_width = e.border_width;
    changes.sibling = e.above;
    changes.stack_mode = e.detail;

    Window target = e.window;
    if(clients.count(e.window)){
        target = clients[e.window];
    }

    XConfigureWindow(display, target, e.value_mask, &changes);
    spdlog::info("Resize [{}] to {}x{}", target, e.width, e.height);
}

void WindowManager::OnButtonPress(const XButtonEvent &e) {
    if(!clients.count(e.window))
        return;

    const Window frame = clients[e.window];

    drag_start_pos = Position<int>(e.x_root, e.y_root);

    Window returned_root;
    int x, y;
    unsigned width, height, border_width, depth;
    if(!XGetGeometry(display, frame, &returned_root, &x, &y, &width, &height, &border_width, &depth))
        return;

    drag_start_frame_pos = Position<int>(x, y);
    drag_start_frame_size = Size<int>(width, height);

    XRaiseWindow(display, frame);
}

void WindowManager::OnButtonRelease(const XButtonEvent &e) {

}

void WindowManager::OnMotionNotify(const XMotionEvent &e) {
    if(!clients.count(e.window))
        return;

    const Window frame = clients[e.window];
    const Position<int> drag_pos(e.x_root, e.y_root);
    const Vector2D<int> delta = drag_pos - drag_start_pos;

    if (e.state & Button1Mask) {
        // alt + Left Button: Move
        const Position<int> dest_frame_pos = drag_start_frame_pos + delta;
        XMoveWindow(display, frame, dest_frame_pos.x, dest_frame_pos.y);
    } else if (e.state & Button3Mask) {
        // Alt + Right Button: Resize
        const Vector2D<int> size_delta(
                max(delta.x, -drag_start_frame_size.width),
                max(delta.y, -drag_start_frame_size.height));
        const Size<int> dest_frame_size = drag_start_frame_size + size_delta;
        XResizeWindow(display, frame, dest_frame_size.width, dest_frame_size.height);
        XResizeWindow(display, e.window, dest_frame_size.width, dest_frame_size.height);
    }
}

void WindowManager::OnKeyPress(const XKeyEvent &e) {
    if ((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(display, XK_F4))){
        // Kill IT!
        Atom* supported_protocols;
        int num_supported_protocols;
        if(XGetWMProtocols(display, e.window, &supported_protocols, &num_supported_protocols) && (::std::find(supported_protocols, supported_protocols + num_supported_protocols, WM_DELETE_WINDOW) != supported_protocols + num_supported_protocols)) {
            spdlog::info("Gracefully deleting window {}", e.window);
            XEvent killMsg;
            memset(&killMsg, 0, sizeof(killMsg));
            killMsg.xclient.type = ClientMessage;
            killMsg.xclient.message_type = WM_PROTOCOLS;
            killMsg.xclient.window = e.window;
            killMsg.xclient.format = 32;
            killMsg.xclient.data.l[0] = WM_DELETE_WINDOW;
            if(!XSendEvent(display, e.window, false, 0, &killMsg))
                return;
        } else {
            spdlog::info("Killing window {}", e.window);
            XKillClient(display, e.window);
        }
    } else if((e.state & Mod1Mask) && (e.keycode == XKeysymToKeycode(display, XK_Tab))) {
        // Alt + Tab
        auto i = clients.find(e.window);
        ++i;
        if(i == clients.end())
            i = clients.begin();
        XRaiseWindow(display, i->second);
        XSetInputFocus(display, i->first, RevertToPointerRoot, CurrentTime);
    }

    spdlog::info("Key Press: {}", e.keycode);
}

void WindowManager::OnKeyRelease(const XKeyEvent &e) {

}
