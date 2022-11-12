#include "ppCommon.c"

#include <stdio.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#include <objc/objc.h>
#include <objc/runtime.h>
#include <objc/message.h>
#include <objc/NSObjCRuntime.h>

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
typedef CGPoint NSPoint;
typedef CGSize NSSize;
typedef CGRect NSRect;

extern id NSApp;
extern id const NSDefaultRunLoopMode;

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

#define $Super(RET, ...) ((RET(*)(struct objc_super*, SEL, ##__VA_ARGS__))objc_msgSendSuper)
#define $Set(OBJ, VAR, VAL) object_setInstanceVariable(OBJ, __STRINGIFY(VAR), (void*)VAL)
#define $Get(OBJ, VAR, OUT) object_getInstanceVariable(self, __STRINGIFY(VAR), (void**)&OUT)
#define $SelfSet(VAR, VAL) $Set(self, VAR, VAL)
#define $SelfGet(VAR, OUT) $Get(self, VAR, OUT)
#define $(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))objc_msgSend)
// ObjC functions that return regular structs (e.g. NSRect) must use this
#define $struct(RET, ...) ((RET(*)(id, SEL, ##__VA_ARGS__))abi_objc_msgSend_stret)
#define $id $(id)
#define $void $(void)

#define $Class(NAME, SUPER) \
    objc_allocateClassPair((Class)objc_getClass(__STRINGIFY(SUPER)), __STRINGIFY(NAME), 0)
#define $Method(CLASS, SEL, IMPL, SIGNATURE)                         \
    if (!class_addMethod(CLASS, sel(SEL), (IMP)(IMPL), (SIGNATURE))) \
        assert(false)
#define $IVar(CLASS, NAME, SIZE, SIGNATURE)                             \
    if (!class_addIvar(CLASS, NAME, SIZE, rint(log2(SIZE)), SIGNATURE)) \
        assert(false)
#define $Protocol(CLASS, PROTOCOL)                                  \
    if (!class_addProtocol(CLASS, protocol(__STRINGIFY(PROTOCOL)))) \
        assert(false);

#if defined(__OBJC__) && __has_feature(objc_arc) && !defined(OBJC_NO_ARC)
#define OBJC_ARC_AVAILABLE
#endif

#if defined(OBJC_ARC_AVAILABLE)
#define $Alloc(CLASS) class(CLASS)
#define $Autorelease(CLASS)
#define AutoreleasePool(...) \
    do                       \
    {                        \
        @autoreleasepool     \
        {                    \
            __VA_ARGS__      \
        }                    \
    } while (0)
#else
#define $Alloc(CLASS) $(id)(class(CLASS), sel(alloc))
#define $Autorelease(CLASS) $(void)(class(CLASS), sel(autorelease))
#define AutoreleasePool(...)                       \
    do                                             \
    {                                              \
        id __OBJC_POOL = $Init(NSAutoreleasePool); \
        __VA_ARGS__                                \
        $(void)(__OBJC_POOL, sel(drain));          \
    } while (0)
#endif
#define $Init(CLASS) $(id)($Alloc(CLASS), sel(init))

typedef enum {
    NSWindowStyleMaskBorderless = 0,
    NSWindowStyleMaskTitled = 1 << 0,
    NSWindowStyleMaskClosable = 1 << 1,
    NSWindowStyleMaskMiniaturizable = 1 << 2,
    NSWindowStyleMaskResizable    = 1 << 3,
    NSWindowStyleMaskFullScreen API_AVAILABLE(macos(10.7)) = 1 << 14,
    NSWindowStyleMaskFullSizeContentView API_AVAILABLE(macos(10.10)) = 1 << 15,
} NSWindowStyleMask;

typedef enum {
    NSWindowCloseButton,
    NSWindowMiniaturizeButton,
    NSWindowZoomButton
} NSWindowButton;

typedef enum {
    NSEventTypeLeftMouseDown             = 1,
    NSEventTypeLeftMouseUp               = 2,
    NSEventTypeRightMouseDown            = 3,
    NSEventTypeRightMouseUp              = 4,
    NSEventTypeMouseMoved                = 5,
    NSEventTypeLeftMouseDragged          = 6,
    NSEventTypeRightMouseDragged         = 7,
    NSEventTypeMouseEntered              = 8,
    NSEventTypeMouseExited               = 9,
    NSEventTypeKeyDown                   = 10,
    NSEventTypeKeyUp                     = 11,
    NSEventTypeFlagsChanged              = 12,
    NSEventTypeCursorUpdate              = 17,
    NSEventTypeScrollWheel               = 22,
    NSEventTypeOtherMouseDown            = 25,
    NSEventTypeOtherMouseUp              = 26,
    NSEventTypeOtherMouseDragged         = 27
} NSEventType;

#define NSBackingStoreBuffered 2
#define NSFloatingWindowLevel 5
#define NSWindowCollectionBehaviorFullScreenPrimary 1 << 7

static struct {
    id window, view;
} ppMacInternal;

NSUInteger applicationShouldTerminate(id self, SEL _sel, id sender) {
    ppInternal.running = false;
    return 0;
}

void windowWillClose(id self, SEL _sel, id notification) {
    ppInternal.running = false;
}

bool ppWindow(int w, int h, const char *title, ppWindowFlags flags) {
    if (ppInternal.initialized)
        return false;
    
    AutoreleasePool({
        $(id)(class(NSApplication), sel(sharedApplication));
        $(void, NSInteger)(NSApp, sel(setActivationPolicy:), NSApplicationActivationPolicyRegular);
        
        Class AppDelegate = $Class(AppDelegate, NSObject);
        $Method(AppDelegate, applicationShouldTerminate, applicationShouldTerminate, NSUIntegerEncoding "@:@:");
        id appDelegate = $(id)($(id)((id)AppDelegate, sel(alloc)), sel(init));
        $Autorelease(AppDelegate);
        
        $(void, id)(NSApp, sel(setDelegate:), appDelegate);
        $(void)(NSApp, sel(finishLaunching));
        
        id menuBar = $Init(NSMenu);
        id menuItem = $Init(NSMenuItem);
        $(void, id)(menuBar, sel(addItem:), menuItem);
        $(id, id)(NSApp, sel(setMainMenu:), menuBar);
        id procInfo = $(id)(class(NSProcessInfo), sel(processInfo));
        id appName = $(id)(procInfo, sel(processName));
        
        id appMenu = $(id, id)($Alloc(NSMenu), sel(initWithTitle:), appName);
        id quitTitleStr = $(id, const char*)(class(NSString), sel(stringWithUTF8String:), "Quit ");
        id quitTitle = $(id, id)(quitTitleStr, sel(stringByAppendingString:), appName);
        id quitItemKey = $(id, const char*)(class(NSString), sel(stringWithUTF8String:), "q");
        id quitItem = $(id, id, SEL, id)($Alloc(NSMenuItem), sel(initWithTitle:action:keyEquivalent:), quitTitle, sel(terminate:), quitItemKey);
        
        $(void, id)(appMenu, sel(addItem:), quitItem);
        $(void, id)(menuItem, sel(setSubmenu:), appMenu);
    });
    
    NSWindowStyleMask styleMask = NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskMiniaturizable;
    flags |= (flags & ppFullscreen ? (ppBorderless | ppResizable | ppFullscreenDesktop) : 0);
    styleMask |= (flags & ppResizable ? NSWindowStyleMaskResizable : 0);
    styleMask |= (flags & ppBorderless ? NSWindowStyleMaskFullSizeContentView : 0);
    if (flags & ppFullscreenDesktop) {
        NSRect f = $struct(NSRect)($(id)(class(NSScreen), sel(mainScreen)), sel(frame));
        w = f.size.width;
        h = f.size.height;
        styleMask |= NSWindowStyleMaskFullSizeContentView;
    }
    NSRect windowFrame = {{0, 0}, {w, h}};
    
    ppMacInternal.window = $(id, NSRect, NSUInteger, NSUInteger, BOOL)($Alloc(NSWindow), sel(initWithContentRect:styleMask:backing:defer:), windowFrame, styleMask, NSBackingStoreBuffered, NO);
    $(void, BOOL)(ppMacInternal.window, sel(setReleasedWhenClosed:), NO);
    
    if (flags & ppAlwaysOnTop)
        $(void, NSInteger)(ppMacInternal.window, sel(setLevel:), NSFloatingWindowLevel);
    
#if MAC_OS_X_VERSION_MAX_ALLOWED >= MAC_OS_X_VERSION_10_7
    if (flags & ppFullscreen) {
        $(void, NSUInteger)(ppMacInternal.window, sel(setCollectionBehavior:), NSWindowCollectionBehaviorFullScreenPrimary);
        $(void, SEL, id, BOOL)(ppMacInternal.window, sel(performSelectorOnMainThread:withObject:waitUntilDone:), sel(toggleFullScreen:), ppMacInternal.window, NO);
    }
#else
#pragma message WARN("Fullscreen is unsupported on OSX versions < 10.7")
#endif
    
    $(void, BOOL)(ppMacInternal.window, sel(setAcceptsMouseMovedEvents:), YES);
    $(void, BOOL)(ppMacInternal.window, sel(setRestorable:), NO);
    $(void, BOOL)(ppMacInternal.window, sel(setReleasedWhenClosed:), NO);
    
    id windowTitle = nil;
    if (flags & ppBorderless && flags & ~ppFullscreen) {
        windowTitle = $(id)(class(NSString), sel(string));
        $(void, BOOL)(ppMacInternal.window, sel(setTitlebarAppearsTransparent:), YES);
        $(void, BOOL)($(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowZoomButton), sel(setHidden:), YES);
        $(void, BOOL)($(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowCloseButton), sel(setHidden:), YES);
        $(void, BOOL)($(id, NSUInteger)(ppMacInternal.window, sel(standardWindowButton:), NSWindowMiniaturizeButton), sel(setHidden:), YES);
    } else {
        windowTitle = $(id, const char*)(class(NSString), sel(stringWithUTF8String:), title);
    }
    $(void, id)(ppMacInternal.window, sel(setTitle:), windowTitle);
    $(void)(ppMacInternal.window, sel(center));
    
    Class WindowDelegate = $Class(WindowDelegate, NSObject);
    $Protocol(WindowDelegate, NSWindowDelegate);
    $Method(WindowDelegate, windowWillClose:, windowWillClose, "v@:@");
    id windowDelegate = $(id)($(id)((id)WindowDelegate, sel(alloc)), sel(init));
    $Autorelease(windowDelegate);
    $(void, id)(ppMacInternal.window, sel(setDelegate:), windowDelegate);
    
    $(void, SEL, id, BOOL)(ppMacInternal.window, sel(performSelectorOnMainThread:withObject:waitUntilDone:), sel(makeKeyAndOrderFront:), nil, YES);
    
    $(void, BOOL)(NSApp, sel(activateIgnoringOtherApps:), YES);
    
    ppInternal.initialized = ppInternal.running = true;
    return true;
}

bool ppPollEvents(void) {
    if (!ppInternal.running || !ppInternal.initialized)
        return false;
    
    AutoreleasePool({
        id distantPast = $(id)(class(NSDate), sel(distantPast));
        id e = nil;
        while (ppInternal.running && (e = $(id, unsigned long long, id, id, BOOL)(NSApp, sel(nextEventMatchingMask:untilDate:inMode:dequeue:), NSUIntegerMax, distantPast, NSDefaultRunLoopMode, YES))) {
            switch ($(NSUInteger)(e, sel(type))) {
                default:
                    break;
            }
            $(void, id)(NSApp, sel(sendEvent:), e);
            $(void)(NSApp, sel(updateWindows));
        }
    });
    
    return ppInternal.running;
}

void ppFlush(Bitmap *bitmap) {
    
}

void ppRelease(void) {
    
}
