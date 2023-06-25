/* ppMac.c -- https://github.com/takeiteasy/pp
 
 The MIT License (MIT)

 Copyright (c) 2022 George Watson

 Permission is hereby granted, free of charge, to any person
 obtaining a copy of this software and associated documentation
 files (the "Software"), to deal in the Software without restriction,
 including without limitation the rights to use, copy, modify, merge,
 publish, distribute, sublicense, and/or sell copies of the Software,
 and to permit persons to whom the Software is furnished to do so,
 subject to the following conditions:

 The above copyright notice and this permission notice shall be
 included in all copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
 MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
 IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
 CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
 TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
 SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE. */

#include "ppCommon.c"
#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>
#include <mach/mach_time.h>

// maybe this is available somewhere in objc runtime?
#if __LP64__ || (TARGET_OS_EMBEDDED && !TARGET_OS_IPHONE) || TARGET_OS_WIN32 || NS_BUILD_32_LIKE_64
#define NSIntegerEncoding "q"
#define NSUIntegerEncoding "L"
#else
#define NSIntegerEncoding "i"
#define NSUIntegerEncoding "I"
#endif

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#else
// this is how they are defined originally
#include <CoreGraphics/CGBase.h>
#include <CoreGraphics/CGGeometry.h>
#include <CoreGraphics/CGContext.h>

typedef CGPoint NSPoint;
typedef CGSize NSSize;
typedef CGRect NSRect;

extern id const NSApp;
extern id const NSDefaultRunLoopMode;

typedef enum {
    NSWindowStyleMaskBorderless          = 0,
    NSWindowStyleMaskTitled              = 1 << 0,
    NSWindowStyleMaskClosable            = 1 << 1,
    NSWindowStyleMaskMiniaturizable      = 1 << 2,
    NSWindowStyleMaskResizable           = 1 << 3,
    NSWindowStyleMaskFullScreen          = 1 << 14,
    NSWindowStyleMaskFullSizeContentView = 1 << 15,
} NSWindowStyleMask;

typedef enum {
    NSWindowCloseButton,
    NSWindowMiniaturizeButton,
    NSWindowZoomButton
} NSWindowButton;

typedef enum {
    NSEventTypeLeftMouseDown      = 1,
    NSEventTypeLeftMouseUp        = 2,
    NSEventTypeRightMouseDown     = 3,
    NSEventTypeRightMouseUp       = 4,
    NSEventTypeMouseMoved         = 5,
    NSEventTypeLeftMouseDragged   = 6,
    NSEventTypeRightMouseDragged  = 7,
    NSEventTypeMouseEntered       = 8,
    NSEventTypeMouseExited        = 9,
    NSEventTypeKeyDown            = 10,
    NSEventTypeKeyUp              = 11,
    NSEventTypeFlagsChanged       = 12,
    NSEventTypeCursorUpdate       = 17,
    NSEventTypeScrollWheel        = 22,
    NSEventTypeOtherMouseDown     = 25,
    NSEventTypeOtherMouseUp       = 26,
    NSEventTypeOtherMouseDraggedd = 27
} NSEventType;

typedef enum {
    NSEventModifierFlagCapsLock = 1 << 16,
    NSEventModifierFlagShift    = 1 << 17,
    NSEventModifierFlagControl  = 1 << 18,
    NSEventModifierFlagOption   = 1 << 19,
    NSEventModifierFlagCommand  = 1 << 20
} NSEventModifierFlags;

typedef enum {
    NSTrackingMouseEnteredAndExited = 0x01,
    NSTrackingActiveInKeyWindow     = 0x20,
    NSTrackingInVisibleRect         = 0x200
} NSTrackingAreaOptions;

#define NSBackingStoreBuffered 2
#define NSFloatingWindowLevel 5
#define NSWindowCollectionBehaviorFullScreenPrimary 1 << 7
#define NSApplicationActivationPolicyRegular 0
#endif

// ABI is a bit different between platforms
#ifdef __arm64__
#define abi_objc_msgSend_stret objc_msgSend
#else
#define abi_objc_msgSend_stret objc_msgSend_stret
#endif
#ifdef __i386__
#define abi_objc_msgSend_fpret objc_msgSend_fpret
#else
#define abi_objc_msgSend_fpret objc_msgSend
#endif

#define __STRINGIFY(s) #s

#define sel(NAME) sel_registerName(__STRINGIFY(NAME))
#define class(NAME) ((id)objc_getClass(__STRINGIFY(NAME)))
#define protocol(NAME) objc_getProtocol(__STRINGIFY(NAME))

#define ObjC_Super(RET, ...) ((RET(*)(struct objc_super*, SEL, ##__VA_ARGS__))objc_msgSendSuper)
#define ObjC_Set(OBJ, VAR, VAL) object_setInstanceVariable(OBJ, __STRINGIFY(VAR), (void*)VAL)
#define ObjC_Get(OBJ, VAR, OUT) object_getInstanceVariable(self, __STRINGIFY(VAR), (void**)&OUT)
#define ObjC_SelfSet(VAR, VAL) ObjC_Set(self, VAR, VAL)
#define ObjC_SelfGet(VAR, OUT) ObjC_Get(self, VAR, OUT)
#define ObjC(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
// ObjC functions that return regular structs (e.g. NSRect) must use this
#define ObjC_Struct(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))abi_objc_msgSend_stret)

#define ObjC_Class(NAME, SUPER) \
    objc_allocateClassPair((Class)objc_getClass(__STRINGIFY(SUPER)), __STRINGIFY(NAME), 0)
#define ObjC_AddMethod(CLASS, SEL, IMPL, SIGNATURE)                  \
    if (!class_addMethod(CLASS, sel(SEL), (IMP)(IMPL), (SIGNATURE))) \
        assert(false)
#define ObjC_AddIVar(CLASS, NAME, SIZE, SIGNATURE)                                   \
    if (!class_addIvar(CLASS, __STRINGIFY(NAME), SIZE, rint(log2(SIZE)), SIGNATURE)) \
        assert(false)
#define ObjC_AddProtocol(CLASS, PROTOCOL)                           \
    if (!class_addProtocol(CLASS, protocol(__STRINGIFY(PROTOCOL)))) \
        assert(false);
#define ObjC_SubClass(NAME) objc_registerClassPair(NAME)

#if defined(__OBJC__) && __has_feature(objc_arc) && !defined(OBJC_NO_ARC)
#define OBJC_ARC_AVAILABLE
#endif

#if defined(OBJC_ARC_AVAILABLE)
#define ObjC_Autorelease(CLASS)
#define AutoreleasePool(...) \
    do                       \
    {                        \
        @autoreleasepool     \
        {                    \
            __VA_ARGS__      \
        }                    \
    } while (0)
#else
#define ObjC_Autorelease(CLASS) ObjC(void)(class(CLASS), sel(autorelease))
#define AutoreleasePool(...)                                \
    do                                                      \
    {                                                       \
        id __OBJC_POOL = ObjC_Initalize(NSAutoreleasePool); \
        __VA_ARGS__                                         \
        ObjC(void)(__OBJC_POOL, sel(drain));                \
    } while (0)
#endif
#define ObjC_Alloc(CLASS) ObjC(id)(class(CLASS), sel(alloc))
#define ObjC_Init(CLASS) ObjC(id)(CLASS, sel(init))
#define ObjC_Initalize(CLASS) ObjC(id)(ObjC_Alloc(CLASS), sel(init))
#define ObjC_Release(CLASS) ObjC(void)(CLASS, sel(release))

static struct {
    id window;
    mach_timebase_info_data_t info;
    uint64_t timestamp;
    bool mouseInWindow;
} ppMacInternal = {0};

static NSUInteger applicationShouldTerminate(id self, SEL _sel, id sender) {
    ppInternal.running = false;
    return 0;
}

static void windowWillClose(id self, SEL _sel, id notification) {
    if (ppInternal.ClosedCallback)
        ppInternal.ClosedCallback(ppInternal.userdata);
    ppInternal.running = false;
}

static BOOL windowShouldClose(id self, SEL _sel, id sender) {
    ObjC(void, id)(NSApp, sel(terminate:), nil);
    return 1;
}

static void windowDidBecomeKey(id self, SEL _sel, id notification) {
    ppCallCallback(Focus, true);
}

static void windowDidResignKey(id self, SEL _sel, id notification) {
    ppCallCallback(Focus, false);
}

static void windowDidResize(id self, SEL _sel, id notification) {
    CGRect frame = ObjC(CGRect)(ObjC(id)(ppMacInternal.window, sel(contentView)), sel(frame));
    ppCallCallback(Resized, frame.size.width, frame.size.height);
}

static void mouseEntered(id self, SEL _sel, id event) {
    ppMacInternal.mouseInWindow = true;
}

static void mouseExited(id self, SEL _sel, id event) {
    ppMacInternal.mouseInWindow = false;
}

static void drawRect(id self, SEL _self, CGRect rect) {
    if (!ppInternal.pbo)
        return;

    CGContextRef ctx = (CGContextRef)ObjC(id)(ObjC(id)(class(NSGraphicsContext), sel(currentContext)), sel(CGContext));
    CGColorSpaceRef s = CGColorSpaceCreateDeviceRGB();
    CGDataProviderRef p = CGDataProviderCreateWithData(NULL, ppInternal.pbo->buf, ppInternal.pbo->w * ppInternal.pbo->h * 4, NULL);
    CGImageRef img = CGImageCreate(ppInternal.pbo->w, ppInternal.pbo->h, 8, 32, ppInternal.pbo->w * 4, s, kCGImageAlphaNoneSkipFirst | kCGBitmapByteOrder32Little, p, NULL, 0, kCGRenderingIntentDefault);
    CGRect wh = ObjC_Struct(CGRect)(self, sel(frame));
    CGContextDrawImage(ctx, CGRectMake(0, 0, wh.size.width, wh.size.height), img);
    CGColorSpaceRelease(s);
    CGDataProviderRelease(p);
    CGImageRelease(img);
}

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags) {
    mach_timebase_info(&ppMacInternal.info);
    ppMacInternal.timestamp = mach_absolute_time();

    AutoreleasePool({
        ObjC(id)(class(NSApplication), sel(sharedApplication));
        ObjC(void, NSInteger)(NSApp, sel(setActivationPolicy:), NSApplicationActivationPolicyRegular);

        Class AppDelegate = ObjC_Class(AppDelegate, NSObject);
        ObjC_AddMethod(AppDelegate, applicationShouldTerminate, applicationShouldTerminate, NSUIntegerEncoding "@:@:");
        id appDelegate = ObjC(id)(ObjC(id)((id)AppDelegate, sel(alloc)), sel(init));

        ObjC(void, id)(NSApp, sel(setDelegate:), appDelegate);
        ObjC(void)(NSApp, sel(finishLaunching));

        id menuBar = ObjC_Initalize(NSMenu);
        id menuItem = ObjC_Initalize(NSMenuItem);
        ObjC(void, id)(menuBar, sel(addItem:), menuItem);
        ObjC(id, id)(NSApp, sel(setMainMenu:), menuBar);
        id procInfo = ObjC(id)(class(NSProcessInfo), sel(processInfo));
        id appName = ObjC(id)(procInfo, sel(processName));

        id appMenu = ObjC(id, id)(ObjC_Alloc(NSMenu), sel(initWithTitle:), appName);
        id quitTitleStr = ObjC(id, const char*)(class(NSString), sel(stringWithUTF8String:), "Quit ");
        id quitTitle = ObjC(id, id)(quitTitleStr, sel(stringByAppendingString:), appName);
        id quitItemKey = ObjC(id, const char*)(class(NSString), sel(stringWithUTF8String:), "q");
        id quitItem = ObjC(id, id, SEL, id)(ObjC_Alloc(NSMenuItem), sel(initWithTitle:action:keyEquivalent:), quitTitle, sel(terminate:), quitItemKey);

        ObjC(void, id)(appMenu, sel(addItem:), quitItem);
        ObjC(void, id)(menuItem, sel(setSubmenu:), appMenu);

        NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
        flags |= (flags & ppFullscreen ? (ppBorderless | ppResizable | ppFullscreenDesktop) : 0);
        styleMask |= (flags & ppResizable ? NSWindowStyleMaskResizable : 0);
        styleMask |= (flags & ppBorderless ? NSWindowStyleMaskFullSizeContentView : 0);
        if (flags & ppFullscreenDesktop) {
            NSRect f = ObjC_Struct(NSRect)(ObjC(id)(class(NSScreen), sel(mainScreen)), sel(frame));
            w = f.size.width;
            h = f.size.height;
            styleMask |= NSWindowStyleMaskFullSizeContentView;
        }
        NSRect windowFrame = {{0, 0}, {w, h}};

        ppMacInternal.window = ObjC(id, NSRect, NSUInteger, NSUInteger, BOOL)(ObjC_Alloc(NSWindow), sel(initWithContentRect:styleMask:backing:defer:), windowFrame, styleMask, NSBackingStoreBuffered, NO);
        ObjC(void, BOOL)(ppMacInternal.window, sel(setReleasedWhenClosed:), NO);

        if (flags & ppAlwaysOnTop)
            ObjC(void, NSInteger)(ppMacInternal.window, sel(setLevel:), NSFloatingWindowLevel);

#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
        if (flags & ppFullscreen) {
            ObjC(void, NSUInteger)(ppMacInternal.window, sel(setCollectionBehavior:), NSWindowCollectionBehaviorFullScreenPrimary);
            ObjC(void, SEL, id, BOOL)(ppMacInternal.window, sel(performSelectorOnMainThread:withObject:waitUntilDone:), sel(toggleFullScreen:), ppMacInternal.window, NO);
        }
#else
#pragma message("Fullscreen is unsupported on OSX versions < 10.7")
#endif

        ObjC(void, BOOL)(ppMacInternal.window, sel(setAcceptsMouseMovedEvents:), YES);
        ObjC(void, BOOL)(ppMacInternal.window, sel(setRestorable:), NO);
        ObjC(void, BOOL)(ppMacInternal.window, sel(setReleasedWhenClosed:), NO);

        id windowTitle = nil;
        if (flags & ppBorderless && flags & ~ppFullscreen) {
            windowTitle = ObjC(id)(class(NSString), sel(string));
            ObjC(void, BOOL)(ppMacInternal.window, sel(setTitlebarAppearsTransparent:), YES);
            ObjC(void, BOOL)(ObjC(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowZoomButton), sel(setHidden:), YES);
            ObjC(void, BOOL)(ObjC(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowCloseButton), sel(setHidden:), YES);
            ObjC(void, BOOL)(ObjC(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowMiniaturizeButton), sel(setHidden:), YES);
        } else {
            windowTitle = ObjC(id, const char*)(class(NSString), sel(stringWithUTF8String:), title);
        }
        ObjC(void, id)(ppMacInternal.window, sel(setTitle:), windowTitle);
        ObjC(void)(ppMacInternal.window, sel(center));

        Class WindowDelegate = ObjC_Class(WindowDelegate, NSObject);
        ObjC_AddMethod(WindowDelegate, windowShouldClose:, windowShouldClose, "c@:@");
        ObjC_AddMethod(WindowDelegate, windowWillClose:, windowWillClose, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidBecomeKey:, windowDidBecomeKey, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidResignKey:, windowDidResignKey, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidResize:, windowDidResize, "v@:@");
        ObjC_AddMethod(WindowDelegate, mouseEntered:, mouseEntered, "v@:@");
        ObjC_AddMethod(WindowDelegate, mouseExited:, mouseExited, "v@:@");
        ObjC_SubClass(WindowDelegate);
        id windowDelegate = ObjC(id)(ObjC(id)((id)WindowDelegate, sel(alloc)), sel(init));
        ObjC(void, id)(ppMacInternal.window, sel(setDelegate:), windowDelegate);

        Class View = ObjC_Class(View, NSView);
        ObjC_AddMethod(View, drawRect:, drawRect, "v@:");
        ObjC_SubClass(View);
        int trackingFlags = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
        id trackingArea = ObjC(id, NSRect, int, id, id)(ObjC_Alloc(NSTrackingArea), sel(initWithRect:options:owner:userInfo:), windowFrame, trackingFlags, windowDelegate, nil);
        id view = ObjC(id, NSRect)(ObjC_Alloc(View), sel(initWithFrame:), windowFrame);
        ObjC(void, id)(view, sel(addTrackingArea:), trackingArea);
        ObjC(void, id)(ppMacInternal.window, sel(setContentView:), view);

        ObjC(void, SEL, id, BOOL)(ppMacInternal.window, sel(performSelectorOnMainThread:withObject:waitUntilDone:), sel(makeKeyAndOrderFront:), nil, YES);

        ObjC(void, BOOL)(NSApp, sel(activateIgnoringOtherApps:), YES);
    });

    ppInternal.running = true;
    return true;
}

// from Carbon HIToolbox/Events.h
enum {
    kVK_ANSI_A = 0x00,
    kVK_ANSI_S = 0x01,
    kVK_ANSI_D = 0x02,
    kVK_ANSI_F = 0x03,
    kVK_ANSI_H = 0x04,
    kVK_ANSI_G = 0x05,
    kVK_ANSI_Z = 0x06,
    kVK_ANSI_X = 0x07,
    kVK_ANSI_C = 0x08,
    kVK_ANSI_V = 0x09,
    kVK_ANSI_B = 0x0B,
    kVK_ANSI_Q = 0x0C,
    kVK_ANSI_W = 0x0D,
    kVK_ANSI_E = 0x0E,
    kVK_ANSI_R = 0x0F,
    kVK_ANSI_Y = 0x10,
    kVK_ANSI_T = 0x11,
    kVK_ANSI_1 = 0x12,
    kVK_ANSI_2 = 0x13,
    kVK_ANSI_3 = 0x14,
    kVK_ANSI_4 = 0x15,
    kVK_ANSI_6 = 0x16,
    kVK_ANSI_5 = 0x17,
    kVK_ANSI_Equal = 0x18,
    kVK_ANSI_9 = 0x19,
    kVK_ANSI_7 = 0x1A,
    kVK_ANSI_Minus = 0x1B,
    kVK_ANSI_8 = 0x1C,
    kVK_ANSI_0 = 0x1D,
    kVK_ANSI_RightBracket = 0x1E,
    kVK_ANSI_O = 0x1F,
    kVK_ANSI_U = 0x20,
    kVK_ANSI_LeftBracket = 0x21,
    kVK_ANSI_I = 0x22,
    kVK_ANSI_P = 0x23,
    kVK_ANSI_L = 0x25,
    kVK_ANSI_J = 0x26,
    kVK_ANSI_Quote = 0x27,
    kVK_ANSI_K = 0x28,
    kVK_ANSI_Semicolon = 0x29,
    kVK_ANSI_Backslash = 0x2A,
    kVK_ANSI_Comma = 0x2B,
    kVK_ANSI_Slash = 0x2C,
    kVK_ANSI_N = 0x2D,
    kVK_ANSI_M = 0x2E,
    kVK_ANSI_Period = 0x2F,
    kVK_ANSI_Grave = 0x32,
    kVK_ANSI_KeypadDecimal = 0x41,
    kVK_ANSI_KeypadMultiply = 0x43,
    kVK_ANSI_KeypadPlus = 0x45,
    kVK_ANSI_KeypadClear = 0x47,
    kVK_ANSI_KeypadDivide = 0x4B,
    kVK_ANSI_KeypadEnter = 0x4C,
    kVK_ANSI_KeypadMinus = 0x4E,
    kVK_ANSI_KeypadEquals = 0x51,
    kVK_ANSI_Keypad0 = 0x52,
    kVK_ANSI_Keypad1 = 0x53,
    kVK_ANSI_Keypad2 = 0x54,
    kVK_ANSI_Keypad3 = 0x55,
    kVK_ANSI_Keypad4 = 0x56,
    kVK_ANSI_Keypad5 = 0x57,
    kVK_ANSI_Keypad6 = 0x58,
    kVK_ANSI_Keypad7 = 0x59,
    kVK_ANSI_Keypad8 = 0x5B,
    kVK_ANSI_Keypad9 = 0x5C,
    kVK_Return = 0x24,
    kVK_Tab = 0x30,
    kVK_Space = 0x31,
    kVK_Delete = 0x33,
    kVK_Escape = 0x35,
    kVK_Command = 0x37,
    kVK_Shift = 0x38,
    kVK_CapsLock = 0x39,
    kVK_Option = 0x3A,
    kVK_Control = 0x3B,
    kVK_RightShift = 0x3C,
    kVK_RightOption = 0x3D,
    kVK_RightControl = 0x3E,
    kVK_Function = 0x3F,
    kVK_F17 = 0x40,
    kVK_VolumeUp = 0x48,
    kVK_VolumeDown = 0x49,
    kVK_Mute = 0x4A,
    kVK_F18 = 0x4F,
    kVK_F19 = 0x50,
    kVK_F20 = 0x5A,
    kVK_F5 = 0x60,
    kVK_F6 = 0x61,
    kVK_F7 = 0x62,
    kVK_F3 = 0x63,
    kVK_F8 = 0x64,
    kVK_F9 = 0x65,
    kVK_F11 = 0x67,
    kVK_F13 = 0x69,
    kVK_F16 = 0x6A,
    kVK_F14 = 0x6B,
    kVK_F10 = 0x6D,
    kVK_F12 = 0x6F,
    kVK_F15 = 0x71,
    kVK_Help = 0x72,
    kVK_Home = 0x73,
    kVK_PageUp = 0x74,
    kVK_ForwardDelete = 0x75,
    kVK_F4 = 0x76,
    kVK_End = 0x77,
    kVK_F2 = 0x78,
    kVK_PageDown = 0x79,
    kVK_F1 = 0x7A,
    kVK_LeftArrow = 0x7B,
    kVK_RightArrow = 0x7C,
    kVK_DownArrow = 0x7D,
    kVK_UpArrow = 0x7E
};

// Taken from tigr: https://github.com/erkkah/tigr/blob/master/tigr.c#L3142
static uint8_t ConvertMacKey(uint16_t key) {
    switch (key) {
        case kVK_ANSI_Q:
            return 'Q';
        case kVK_ANSI_W:
            return 'W';
        case kVK_ANSI_E:
            return 'E';
        case kVK_ANSI_R:
            return 'R';
        case kVK_ANSI_T:
            return 'T';
        case kVK_ANSI_Y:
            return 'Y';
        case kVK_ANSI_U:
            return 'U';
        case kVK_ANSI_I:
            return 'I';
        case kVK_ANSI_O:
            return 'O';
        case kVK_ANSI_P:
            return 'P';
        case kVK_ANSI_A:
            return 'A';
        case kVK_ANSI_S:
            return 'S';
        case kVK_ANSI_D:
            return 'D';
        case kVK_ANSI_F:
            return 'F';
        case kVK_ANSI_G:
            return 'G';
        case kVK_ANSI_H:
            return 'H';
        case kVK_ANSI_J:
            return 'J';
        case kVK_ANSI_K:
            return 'K';
        case kVK_ANSI_L:
            return 'L';
        case kVK_ANSI_Z:
            return 'Z';
        case kVK_ANSI_X:
            return 'X';
        case kVK_ANSI_C:
            return 'C';
        case kVK_ANSI_V:
            return 'V';
        case kVK_ANSI_B:
            return 'B';
        case kVK_ANSI_N:
            return 'N';
        case kVK_ANSI_M:
            return 'M';
        case kVK_ANSI_0:
            return '0';
        case kVK_ANSI_1:
            return '1';
        case kVK_ANSI_2:
            return '2';
        case kVK_ANSI_3:
            return '3';
        case kVK_ANSI_4:
            return '4';
        case kVK_ANSI_5:
            return '5';
        case kVK_ANSI_6:
            return '6';
        case kVK_ANSI_7:
            return '7';
        case kVK_ANSI_8:
            return '8';
        case kVK_ANSI_9:
            return '9';
        case kVK_ANSI_Keypad0:
            return KEY_PAD0;
        case kVK_ANSI_Keypad1:
            return KEY_PAD1;
        case kVK_ANSI_Keypad2:
            return KEY_PAD2;
        case kVK_ANSI_Keypad3:
            return KEY_PAD3;
        case kVK_ANSI_Keypad4:
            return KEY_PAD4;
        case kVK_ANSI_Keypad5:
            return KEY_PAD5;
        case kVK_ANSI_Keypad6:
            return KEY_PAD6;
        case kVK_ANSI_Keypad7:
            return KEY_PAD7;
        case kVK_ANSI_Keypad8:
            return KEY_PAD8;
        case kVK_ANSI_Keypad9:
            return KEY_PAD9;
        case kVK_ANSI_KeypadMultiply:
            return KEY_PADMUL;
        case kVK_ANSI_KeypadPlus:
            return KEY_PADADD;
        case kVK_ANSI_KeypadEnter:
            return KEY_PADENTER;
        case kVK_ANSI_KeypadMinus:
            return KEY_PADSUB;
        case kVK_ANSI_KeypadDecimal:
            return KEY_PADDOT;
        case kVK_ANSI_KeypadDivide:
            return KEY_PADDIV;
        case kVK_F1:
            return KEY_F1;
        case kVK_F2:
            return KEY_F2;
        case kVK_F3:
            return KEY_F3;
        case kVK_F4:
            return KEY_F4;
        case kVK_F5:
            return KEY_F5;
        case kVK_F6:
            return KEY_F6;
        case kVK_F7:
            return KEY_F7;
        case kVK_F8:
            return KEY_F8;
        case kVK_F9:
            return KEY_F9;
        case kVK_F10:
            return KEY_F10;
        case kVK_F11:
            return KEY_F11;
        case kVK_F12:
            return KEY_F12;
        case kVK_Shift:
            return KEY_LSHIFT;
        case kVK_Control:
            return KEY_LCONTROL;
        case kVK_Option:
            return KEY_LALT;
        case kVK_CapsLock:
            return KEY_CAPSLOCK;
        case kVK_Command:
            return KEY_LWIN;
        case kVK_Command - 1:
            return KEY_RWIN;
        case kVK_RightShift:
            return KEY_RSHIFT;
        case kVK_RightControl:
            return KEY_RCONTROL;
        case kVK_RightOption:
            return KEY_RALT;
        case kVK_Delete:
            return KEY_BACKSPACE;
        case kVK_Tab:
            return KEY_TAB;
        case kVK_Return:
            return KEY_RETURN;
        case kVK_Escape:
            return KEY_ESCAPE;
        case kVK_Space:
            return KEY_SPACE;
        case kVK_PageUp:
            return KEY_PAGEUP;
        case kVK_PageDown:
            return KEY_PAGEDN;
        case kVK_End:
            return KEY_END;
        case kVK_Home:
            return KEY_HOME;
        case kVK_LeftArrow:
            return KEY_LEFT;
        case kVK_UpArrow:
            return KEY_UP;
        case kVK_RightArrow:
            return KEY_RIGHT;
        case kVK_DownArrow:
            return KEY_DOWN;
        case kVK_Help:
            return KEY_INSERT;
        case kVK_ForwardDelete:
            return KEY_DELETE;
        case kVK_F14:
            return KEY_SCROLL;
        case kVK_F15:
            return KEY_PAUSE;
        case kVK_ANSI_KeypadClear:
            return KEY_NUMLOCK;
        case kVK_ANSI_Semicolon:
            return KEY_SEMICOLON;
        case kVK_ANSI_Equal:
            return KEY_EQUALS;
        case kVK_ANSI_Comma:
            return KEY_COMMA;
        case kVK_ANSI_Minus:
            return KEY_MINUS;
        case kVK_ANSI_Slash:
            return KEY_SLASH;
        case kVK_ANSI_Backslash:
            return KEY_BACKSLASH;
        case kVK_ANSI_Grave:
            return KEY_BACKTICK;
        case kVK_ANSI_Quote:
            return KEY_TICK;
        case kVK_ANSI_LeftBracket:
            return KEY_LSQUARE;
        case kVK_ANSI_RightBracket:
            return KEY_RSQUARE;
        case kVK_ANSI_Period:
            return KEY_DOT;
        default:
            return 0;
    }
}

static uint32_t ConvertMacMod(NSUInteger modifierFlags) {
    int mods = 0;
    if (modifierFlags & NSEventModifierFlagShift)
        mods |= KEY_MOD_SHIFT;
    if (modifierFlags & NSEventModifierFlagControl)
        mods |= KEY_MOD_CONTROL;
    if (modifierFlags & NSEventModifierFlagOption)
        mods |= KEY_MOD_ALT;
    if (modifierFlags & NSEventModifierFlagCommand)
        mods |= KEY_MOD_SUPER;
    if (modifierFlags & NSEventModifierFlagCapsLock)
        mods |= KEY_MOD_CAPS_LOCK;
    return mods;
}

bool ppPoll(void) {
    if (!ppInternal.running)
        return false;

    AutoreleasePool({
        id distantPast = ObjC(id)(class(NSDate), sel(distantPast));
        id e = nil;
        while (ppInternal.running && (e = ObjC(id, unsigned long long, id, id, BOOL)(NSApp, sel(nextEventMatchingMask:untilDate:inMode:dequeue:), NSUIntegerMax, distantPast, NSDefaultRunLoopMode, YES))) {
            NSUInteger type = ObjC(NSUInteger)(e, sel(type));
            switch (type) {
                case NSEventTypeLeftMouseDown:
                case NSEventTypeLeftMouseUp:
                    ppCallCallback(MouseButton, 1, ConvertMacMod(ObjC(NSUInteger)(e, sel(modifierFlags))), type == NSEventTypeLeftMouseDown);
                    break;
                case NSEventTypeRightMouseDown:
                case NSEventTypeRightMouseUp:
                    ppCallCallback(MouseButton, 2, ConvertMacMod(ObjC(NSUInteger)(e, sel(modifierFlags))), type == NSEventTypeRightMouseDown);
                    break;
                case NSEventTypeOtherMouseDown:
                case NSEventTypeOtherMouseUp:
                    ppCallCallback(MouseButton, (int)ObjC(NSUInteger)(e, sel(buttonNumber)), ConvertMacMod(ObjC(NSUInteger)(e, sel(modifierFlags))), type == NSEventTypeOtherMouseDown);
                    break;
                case NSEventTypeScrollWheel:
                    ppCallCallback(MouseScroll, ObjC(CGFloat)(e, sel(deltaX)), ObjC(CGFloat)(e, sel(deltaY)), ConvertMacMod(ObjC(NSUInteger)(e, sel(modifierFlags))));
                    break;
                case NSEventTypeKeyDown:
                case NSEventTypeKeyUp:
                    ppCallCallback(Keyboard, ConvertMacKey(ObjC(unsigned short)(e, sel(keyCode))), ConvertMacMod(ObjC(NSUInteger)(e, sel(modifierFlags))), type == NSEventTypeKeyDown);
                    break;
                case NSEventTypeMouseMoved:
                    if (ppMacInternal.mouseInWindow) {
                        CGPoint locationInWindow = ObjC(CGPoint)(e, sel(locationInWindow));
                        ppCallCallback(MouseMove, (int)locationInWindow.x, (int)(ObjC_Struct(CGRect)(ObjC(id)(ppMacInternal.window, sel(contentView)), sel(frame)).size.height - roundf(locationInWindow.y)), ObjC(CGFloat)(e, sel(deltaX)), ObjC(CGFloat)(e, sel(deltaY)));
                    }
                    break;
                default:
                    break;
            }
            ObjC(void, id)(NSApp, sel(sendEvent:), e);
        }
    });

    return ppInternal.running;
}

void ppFlush(Bitmap *bitmap) {
    if (!bitmap || !bitmap->buf || !bitmap->w || !bitmap->h) {
        ppInternal.pbo = NULL;
        return;
    }
    ppInternal.pbo = bitmap;
    ObjC(void, BOOL)(ObjC(id)(ppMacInternal.window, sel(contentView)), sel(setNeedsDisplay:), YES);
}

void ppEnd(void) {
    if (!ppInternal.running)
        return;
    ObjC(void)(ppMacInternal.window, sel(close));
    ObjC_Release(ppMacInternal.window);
    ObjC(void, id)(NSApp, sel(terminate:), nil);
}

double ppTime(void) {
    uint64_t now = mach_absolute_time();
    double elapsed = (double)(now - ppMacInternal.timestamp) * ppMacInternal.info.numer / (ppMacInternal.info.denom * 1000000000.0);
    ppMacInternal.timestamp = now;
    return elapsed;
}
#elif defined(PP_WINDOWS)
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#if defined(_MSC_VER)
#pragma comment(lib, "user32")
#pragma comment(lib, "gdi32")

#if !defined(_DLL)
#include <shellapi.h>
#pragma comment(lib, "shell32")
#include <stdlib.h>

extern int main(int argc, const char *argv[]);

#ifdef UNICODE
int CALLBACK wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow)
#else
int CALLBACK WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
#endif
{
    int n, argc;
    LPWSTR *wargv = CommandLineToArgvW(GetCommandLineW(), &argc);
    char **argv = calloc(argc + 1, sizeof(int));

    (void)hInstance;
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    for (n = 0; n < argc; n++) {
        int len = WideCharToMultiByte(CP_UTF8, 0, wargv[n], -1, 0, 0, NULL, NULL);
        argv[n] = malloc(len);
        WideCharToMultiByte(CP_UTF8, 0, wargv[n], -1, argv[n], len, NULL, NULL);
    }
    return main(argc, argv);
}
#endif
#endif

static struct {
    WNDCLASS wnd;
    HWND hwnd;
    HDC hdc;
    BITMAPINFO *bmp;
    TRACKMOUSEEVENT tme;
    bool tmeRefresh;
    int width, height;
    int cursorLastX, cursorLastY;
    LARGE_INTEGER timestamp;
} ppWinInternal = {0};

static int WindowsModState(void) {
    int mods = 0;

    if (GetKeyState(VK_SHIFT) & 0x8000)
        mods |= KEY_MOD_SHIFT;
    if (GetKeyState(VK_CONTROL) & 0x8000)
        mods |= KEY_MOD_CONTROL;
    if (GetKeyState(VK_MENU) & 0x8000)
        mods |= KEY_MOD_ALT;
    if ((GetKeyState(VK_LWIN) | GetKeyState(VK_RWIN)) & 0x8000)
        mods |= KEY_MOD_SUPER;
    if (GetKeyState(VK_CAPITAL) & 1)
        mods |= KEY_MOD_CAPS_LOCK;
    if (GetKeyState(VK_NUMLOCK) & 1)
        mods |= KEY_MOD_NUM_LOCK;

    return mods;
}

static int ConvertWindowsKey(int key) {
    if (key >= 'A' && key <= 'Z')
        return key;
    if (key >= '0' && key <= '9')
        return key;
    switch (key) {
        case VK_BACK:
            return KEY_BACKSPACE;
        case VK_TAB:
            return KEY_TAB;
        case VK_RETURN:
            return KEY_RETURN;
        case VK_SHIFT:
            return KEY_SHIFT;
        case VK_CONTROL:
            return KEY_CONTROL;
        case VK_MENU:
            return KEY_ALT;
        case VK_PAUSE:
            return KEY_PAUSE;
        case VK_CAPITAL:
            return KEY_CAPSLOCK;
        case VK_ESCAPE:
            return KEY_ESCAPE;
        case VK_SPACE:
            return KEY_SPACE;
        case VK_PRIOR:
            return KEY_PAGEUP;
        case VK_NEXT:
            return KEY_PAGEDN;
        case VK_END:
            return KEY_END;
        case VK_HOME:
            return KEY_HOME;
        case VK_LEFT:
            return KEY_LEFT;
        case VK_UP:
            return KEY_UP;
        case VK_RIGHT:
            return KEY_RIGHT;
        case VK_DOWN:
            return KEY_DOWN;
        case VK_INSERT:
            return KEY_INSERT;
        case VK_DELETE:
            return KEY_DELETE;
        case VK_LWIN:
            return KEY_LWIN;
        case VK_RWIN:
            return KEY_RWIN;
        case VK_NUMPAD0:
            return KEY_PAD0;
        case VK_NUMPAD1:
            return KEY_PAD1;
        case VK_NUMPAD2:
            return KEY_PAD2;
        case VK_NUMPAD3:
            return KEY_PAD3;
        case VK_NUMPAD4:
            return KEY_PAD4;
        case VK_NUMPAD5:
            return KEY_PAD5;
        case VK_NUMPAD6:
            return KEY_PAD6;
        case VK_NUMPAD7:
            return KEY_PAD7;
        case VK_NUMPAD8:
            return KEY_PAD8;
        case VK_NUMPAD9:
            return KEY_PAD9;
        case VK_MULTIPLY:
            return KEY_PADMUL;
        case VK_ADD:
            return KEY_PADADD;
        case VK_SEPARATOR:
            return KEY_PADENTER;
        case VK_SUBTRACT:
            return KEY_PADSUB;
        case VK_DECIMAL:
            return KEY_PADDOT;
        case VK_DIVIDE:
            return KEY_PADDIV;
        case VK_F1:
            return KEY_F1;
        case VK_F2:
            return KEY_F2;
        case VK_F3:
            return KEY_F3;
        case VK_F4:
            return KEY_F4;
        case VK_F5:
            return KEY_F5;
        case VK_F6:
            return KEY_F6;
        case VK_F7:
            return KEY_F7;
        case VK_F8:
            return KEY_F8;
        case VK_F9:
            return KEY_F9;
        case VK_F10:
            return KEY_F10;
        case VK_F11:
            return KEY_F11;
        case VK_F12:
            return KEY_F12;
        case VK_NUMLOCK:
            return KEY_NUMLOCK;
        case VK_SCROLL:
            return KEY_SCROLL;
        case VK_LSHIFT:
            return KEY_LSHIFT;
        case VK_RSHIFT:
            return KEY_RSHIFT;
        case VK_LCONTROL:
            return KEY_LCONTROL;
        case VK_RCONTROL:
            return KEY_RCONTROL;
        case VK_LMENU:
            return KEY_LALT;
        case VK_RMENU:
            return KEY_RALT;
        case VK_OEM_1:
            return KEY_SEMICOLON;
        case VK_OEM_PLUS:
            return KEY_EQUALS;
        case VK_OEM_COMMA:
            return KEY_COMMA;
        case VK_OEM_MINUS:
            return KEY_MINUS;
        case VK_OEM_PERIOD:
            return KEY_DOT;
        case VK_OEM_2:
            return KEY_SLASH;
        case VK_OEM_3:
            return KEY_BACKTICK;
        case VK_OEM_4:
            return KEY_LSQUARE;
        case VK_OEM_5:
            return KEY_BACKSLASH;
        case VK_OEM_6:
            return KEY_RSQUARE;
        case VK_OEM_7:
            return KEY_TICK;
    }
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    if (!ppInternal.running)
        goto DEFAULT_PROC;

    switch (message) {
        case WM_PAINT:
            if (!ppInternal.pbo)
                goto DEFAULT_PROC;
            ppWinInternal.bmp->bmiHeader.biWidth = ppInternal.pbo->w;
            ppWinInternal.bmp->bmiHeader.biHeight = -ppInternal.pbo->h;
            StretchDIBits(ppWinInternal.hdc, 0, 0, ppWinInternal.width, ppWinInternal.height, 0, 0, ppInternal.pbo->w, ppInternal.pbo->h, ppInternal.pbo->buf, ppWinInternal.bmp, DIB_RGB_COLORS, SRCCOPY);
            ValidateRect(hWnd, NULL);
            break;
        case WM_DESTROY:
        case WM_CLOSE:
            if (ppInternal.ClosedCallback)
                ppInternal.ClosedCallback(ppInternal.userdata);
            ppInternal.running = false;
            break;
        case WM_SIZE:
            ppWinInternal.width = LOWORD(lParam);
            ppWinInternal.height = HIWORD(lParam);
            ppCallCallback(Resized, ppWinInternal.width, ppWinInternal.height);
            break;
        case WM_MENUCHAR:
            // Disable beep on Alt+Enter
            if (LOWORD(wParam) == VK_RETURN)
                return MNC_CLOSE << 16;
            return DefWindowProcW(hWnd, message, wParam, lParam);
        case WM_KEYDOWN:
        case WM_SYSKEYDOWN:
        case WM_KEYUP:
        case WM_SYSKEYUP:
            ppCallCallback(Keyboard, ConvertWindowsKey(wParam), WindowsModState(), !((lParam >> 31) & 1));
            break;
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
        case WM_MBUTTONUP:
        case WM_XBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK:
        case WM_RBUTTONDOWN:
        case WM_RBUTTONDBLCLK:
        case WM_MBUTTONDOWN:
        case WM_MBUTTONDBLCLK:
        case WM_XBUTTONDOWN:
        case WM_XBUTTONDBLCLK: {
            int button = 0;
            bool action = false;
            switch (message) {
                case WM_LBUTTONDOWN:
                    action = true;
                case WM_LBUTTONUP:
                    button = 1;
                    break;
                case WM_RBUTTONDOWN:
                    action = true;
                case WM_RBUTTONUP:
                    button = 2;
                    break;
                case WM_MBUTTONDOWN:
                    action = true;
                case WM_MBUTTONUP:
                    button = 3;
                    break;
                default:
                    button = (GET_XBUTTON_WPARAM(wParam) == XBUTTON1 ? 5 : 6);
                    if (message == WM_XBUTTONDOWN)
                        action = 1;
            }
            ppCallCallback(MouseButton, button, WindowsModState(), action);
            break;
        }
        case WM_MOUSEWHEEL:
            ppCallCallback(MouseScroll, 0.f, (SHORT)HIWORD(wParam) / (float)WHEEL_DELTA, WindowsModState());
            break;
        case WM_MOUSEHWHEEL:
            ppCallCallback(MouseScroll, -((SHORT)HIWORD(wParam) / (float)WHEEL_DELTA), 0., WindowsModState());
            break;
        case WM_MOUSEMOVE: {
            if (ppWinInternal.tmeRefresh) {
                ppWinInternal.tme.cbSize = sizeof(ppWinInternal.tme);
                ppWinInternal.tme.hwndTrack = ppWinInternal.hwnd;
                ppWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
                ppWinInternal.tme.dwHoverTime = 1;
                TrackMouseEvent(&ppWinInternal.tme);
            }
            int cx = ((int)(short)LOWORD(lParam));
            int cy = ((int)(short)HIWORD(lParam));
            ppCallCallback(MouseMove, cx, cy, cx - ppWinInternal.cursorLastX, cy - ppWinInternal.cursorLastY);
            ppWinInternal.cursorLastX = cx;
            ppWinInternal.cursorLastY = cy;
            break;
        }
        case WM_MOUSEHOVER:
            ppWinInternal.tmeRefresh = true;
            break;
        case WM_MOUSELEAVE:
            ppWinInternal.tmeRefresh = false;
            break;
        case WM_SETFOCUS:
            ppCallCallback(Focus, true);
            break;
        case WM_KILLFOCUS:
            ppCallCallback(Focus, false);
            break;
        default:
            goto DEFAULT_PROC;
    }

    return FALSE;
DEFAULT_PROC:
    return DefWindowProc(hWnd, message, wParam, lParam);
}

static bool ppBeginNative(int w, int h, const char *title, ppFlags flags) {
    QueryPerformanceCounter(&ppWinInternal.timestamp);

    RECT rect = {0};
    long windowFlags = WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX;
    if (flags & ppFullscreen) {
        flags = ppFullscreen;
        rect.right = GetSystemMetrics(SM_CXSCREEN);
        rect.bottom = GetSystemMetrics(SM_CYSCREEN);
        windowFlags = WS_POPUP & ~(WS_CAPTION | WS_THICKFRAME | WS_MINIMIZE | WS_MAXIMIZE | WS_SYSMENU);

        DEVMODE settings = {0};
        EnumDisplaySettings(0, 0, &settings);
        settings.dmPelsWidth = GetSystemMetrics(SM_CXSCREEN);
        settings.dmPelsHeight = GetSystemMetrics(SM_CYSCREEN);
        settings.dmBitsPerPel = 32;
        settings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

        if (ChangeDisplaySettings(&settings, CDS_FULLSCREEN) != DISP_CHANGE_SUCCESSFUL)
            flags = ppFullscreenDesktop;
    }

    if (flags & ppBorderless)
        windowFlags = WS_POPUP;
    if (flags & ppResizable)
        windowFlags |= WS_MAXIMIZEBOX | WS_SIZEBOX;
    if (flags & ppFullscreenDesktop) {
        windowFlags = WS_OVERLAPPEDWINDOW;

        int width = GetSystemMetrics(SM_CXFULLSCREEN);
        int height = GetSystemMetrics(SM_CYFULLSCREEN);

        rect.right = width;
        rect.bottom = height;
        AdjustWindowRect(&rect, windowFlags, 0);
        if (rect.left < 0) {
            width += rect.left * 2;
            rect.right += rect.left;
            rect.left = 0;
        }
        if (rect.bottom > (LONG)height) {
            height -= (rect.bottom - height);
            rect.bottom += (rect.bottom - height);
            rect.top = 0;
        }
    } else {
        rect.right = w;
        rect.bottom = h;

        AdjustWindowRect(&rect, windowFlags, 0);

        rect.right -= rect.left;
        rect.bottom -= rect.top;

        rect.left = (GetSystemMetrics(SM_CXSCREEN) - rect.right) / 2;
        rect.top = (GetSystemMetrics(SM_CYSCREEN) - rect.bottom + rect.top) / 2;
    }

    memset(&ppWinInternal.wnd, 0, sizeof(ppWinInternal.wnd));
    ppWinInternal.wnd.style = CS_OWNDC | CS_VREDRAW | CS_HREDRAW;
    ppWinInternal.wnd.lpfnWndProc = WndProc;
    ppWinInternal.wnd.hCursor = LoadCursor(0, IDC_ARROW);
    ppWinInternal.wnd.lpszClassName = title;
    if (!RegisterClass(&ppWinInternal.wnd))
        return false;

    ppWinInternal.width = rect.right;
    ppWinInternal.height = rect.bottom;
    if (!(ppWinInternal.hwnd = CreateWindowEx(0, title, title, windowFlags, rect.left, rect.top, rect.right, rect.bottom, 0, 0, 0, 0)))
        return false;
    if (!(ppWinInternal.hdc = GetDC(ppWinInternal.hwnd)))
        return false;

    if (flags & ppAlwaysOnTop)
        SetWindowPos(ppWinInternal.hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

    ShowWindow(ppWinInternal.hwnd, SW_NORMAL);
    SetFocus(ppWinInternal.hwnd);

    size_t bmpSz = sizeof(BITMAPINFOHEADER) + sizeof(RGBQUAD) * 3;
    if (!(ppWinInternal.bmp = malloc(bmpSz)))
        return false;
    ZeroMemory(ppWinInternal.bmp, bmpSz);
    ppWinInternal.bmp->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    ppWinInternal.bmp->bmiHeader.biPlanes = 1;
    ppWinInternal.bmp->bmiHeader.biBitCount = 32;
    ppWinInternal.bmp->bmiHeader.biCompression = BI_BITFIELDS;
    ppWinInternal.bmp->bmiHeader.biWidth = w;
    ppWinInternal.bmp->bmiHeader.biHeight = -(LONG)h;
    ppWinInternal.bmp->bmiColors[0].rgbRed = 0xFF;
    ppWinInternal.bmp->bmiColors[1].rgbGreen = 0xFF;
    ppWinInternal.bmp->bmiColors[2].rgbBlue = 0xff;

    ppWinInternal.tme.cbSize = sizeof(ppWinInternal.tme);
    ppWinInternal.tme.hwndTrack = ppWinInternal.hwnd;
    ppWinInternal.tme.dwFlags = TME_HOVER | TME_LEAVE;
    ppWinInternal.tme.dwHoverTime = HOVER_DEFAULT;
    TrackMouseEvent(&ppWinInternal.tme);

    ppInternal.running = true;
    return true;
}

bool ppPoll(void) {
    static MSG msg;
    if (PeekMessage(&msg, ppWinInternal.hwnd, 0, 0, PM_REMOVE)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return ppInternal.running;
}

void ppFlush(Bitmap *bitmap) {
    if (!bitmap || !bitmap->buf || !bitmap->w || !bitmap->h) {
        ppInternal.pbo = NULL;
        return;
    }
    ppInternal.pbo = bitmap;
    InvalidateRect(ppWinInternal.hwnd, NULL, TRUE);
    SendMessage(ppWinInternal.hwnd, WM_PAINT, 0, 0);
}

void ppEnd(void) {
    if (!ppInternal.running)
        return;
    free(ppWinInternal.bmp);
    ReleaseDC(ppWinInternal.hwnd, ppWinInternal.hdc);
    DestroyWindow(ppWinInternal.hwnd);
}

double ppTime(void) {
    LARGE_INTEGER cnt, freq;
    QueryPerformanceCounter(&cnt);
    QueryPerformanceFrequency(&freq);
    ULONGLONG diff = cnt.QuadPart - ppWinInternal.timestamp.QuadPart;
    ppWinInternal.timestamp = cnt;
    return (double)(diff / (double)freq.QuadPart);
}
