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

extern id NSApp;
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
        ObjC_Autorelease(AppDelegate);
        
        ObjC(void, id)(NSApp, sel(setDelegate:), appDelegate);
        ObjC(void)(NSApp, sel(finishLaunching));
        
        id menuBar = ObjC_Initalize(NSMenu);
        ObjC_Autorelease(menuBar);
        id menuItem = ObjC_Initalize(NSMenuItem);
        ObjC_Autorelease(menuItem);
        ObjC(void, id)(menuBar, sel(addItem:), menuItem);
        ObjC(id, id)(NSApp, sel(setMainMenu:), menuBar);
        id procInfo = ObjC(id)(class(NSProcessInfo), sel(processInfo));
        id appName = ObjC(id)(procInfo, sel(processName));
        
        id appMenu = ObjC(id, id)(ObjC_Alloc(NSMenu), sel(initWithTitle:), appName);
        ObjC_Autorelease(appMenu);
        id quitTitleStr = ObjC(id, const char*)(class(NSString), sel(stringWithUTF8String:), "Quit ");
        id quitTitle = ObjC(id, id)(quitTitleStr, sel(stringByAppendingString:), appName);
        id quitItemKey = ObjC(id, const char*)(class(NSString), sel(stringWithUTF8String:), "q");
        id quitItem = ObjC(id, id, SEL, id)(ObjC_Alloc(NSMenuItem), sel(initWithTitle:action:keyEquivalent:), quitTitle, sel(terminate:), quitItemKey);
        ObjC_Autorelease(quitItem);
        
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
        ObjC_AddProtocol(WindowDelegate, NSWindowDelegate);
        ObjC_AddMethod(WindowDelegate, windowWillClose:, windowWillClose, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidBecomeKey:, windowDidBecomeKey, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidResignKey:, windowDidResignKey, "v@:@");
        ObjC_AddMethod(WindowDelegate, windowDidResize:, windowDidResize, "v@:@");
        ObjC_AddMethod(WindowDelegate, mouseEntered:, mouseEntered, "v@:@");
        ObjC_AddMethod(WindowDelegate, mouseExited:, mouseExited, "v@:@");
        id windowDelegate = ObjC(id)(ObjC(id)((id)WindowDelegate, sel(alloc)), sel(init));
        ObjC_Autorelease(windowDelegate);
        ObjC(void, id)(ppMacInternal.window, sel(setDelegate:), windowDelegate);
        
        Class View = ObjC_Class(View, NSView);
        ObjC_AddMethod(View, drawRect:, drawRect, "v@:");
        ObjC_SubClass(View);
        int trackingFlags = NSTrackingMouseEnteredAndExited | NSTrackingActiveInKeyWindow | NSTrackingInVisibleRect;
        id trackingArea = ObjC(id, NSRect, int, id, id)(ObjC_Alloc(NSTrackingArea), sel(initWithRect:options:owner:userInfo:), windowFrame, trackingFlags, windowDelegate, nil);
        id view = ObjC(id, NSRect)(ObjC_Alloc(View), sel(initWithFrame:), windowFrame);
        ObjC(void, id)(view, sel(addTrackingArea:), trackingArea);
        ObjC(void, id)(ppMacInternal.window, sel(setContentView:), view);
        ObjC_Autorelease(view);
        ObjC_Autorelease(trackingArea);
        
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
                        ppCallCallback(MouseMove, (int)locationInWindow.x, (int)(ObjC(CGRect)(ObjC(id)(ppMacInternal.window, sel(contentView)), sel(frame)).size.height - roundf(locationInWindow.y)), ObjC(CGFloat)(e, sel(deltaX)), ObjC(CGFloat)(e, sel(deltaY)));
                    }
                    break;
                default:
                    break;
            }
            ObjC(void, id)(NSApp, sel(sendEvent:), e);
            ObjC(void)(NSApp, sel(updateWindows));
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
