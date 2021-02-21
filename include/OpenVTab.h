//
// Created by hugo1 on 14.02.2021.
//

#pragma once

#ifdef __cplusplus
extern C {
#endif

#include <stdbool.h>
#include <sys/types.h>

enum PointerPointPropertiesFlags {
    None = 0,
    TouchConfidence = 1 << 0,
    LeftButtonPressed = 1 << 1,
    RightButtonPressed = 1 << 2,
    MiddleButtonPressed = 1 << 3,
    Inverted = 1 << 4,
    Eraser = 1 << 5,
    HorizontalMouseWheel = 1 << 6,
    Primary = 1 << 7,
    InRange = 1 << 8,
    Canceled = 1 << 9,
    BarrelButtonPressed = 1 << 10,
    XButton1Pressed = 1 << 11,
    XButton2Pressed = 1 << 12
};

enum PointerEventType {
    PointerCanceled = 0,
    PointerCaptureLost = 1,
    PointerEntered = 2,
    PointerExited = 3,
    PointerMoved = 4,
    PointerPressed = 5,
    PointerReleased = 6,
    PointerWheelChanged = 7,
};

enum WindowEventType {
    WindowSizeChanged = 0
};

enum EventType {
    Pointer = 0,
    Window = 1
};

typedef struct {
    float Pressure;
    float Orientation;
    float XTilt;
    float YTilt;
    float Twist;
    int MouseWheelDelta;
    enum PointerPointPropertiesFlags Flags;
} PointerPointProperties;

typedef struct {
    double X;
    double Y;
} Point;

typedef struct {
    Point Position;
    PointerPointProperties Properties;
} PointerPoint;

typedef struct {
    enum PointerEventType type;
    PointerPoint ptrPt;
} PointerEvent;

typedef struct {
    double Width;
    double Height;
} Size;

typedef struct {
    Size size;
} Window_t;

typedef struct {
    enum WindowEventType type;
    Window_t window;
} WindowEvent;

typedef struct {
    double Width;
    double Height;
} ConfigData;

ssize_t ReceiveConfig(int fd, ConfigData *data);
ssize_t ReceivePointerEvent(int fd, PointerEvent *ev);
ssize_t ReceiveWindowEvent(int fd, WindowEvent *ev);
ssize_t ReceiveEventType(int fd, enum EventType *eventType);

#ifdef __cplusplus
}
#endif