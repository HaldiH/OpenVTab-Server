//
// Created by hugo1 on 19.02.2021.
//

#include "OpenVTab.h"
#include <sys/socket.h>

ssize_t ReceivePointerPointProperties(int fd, PointerPointProperties *properties) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = recv(fd, &(properties->Pressure), sizeof(properties->Pressure), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->Orientation), sizeof(properties->Orientation), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->XTilt), sizeof(properties->XTilt), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->YTilt), sizeof(properties->YTilt), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->Twist), sizeof(properties->Twist), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->MouseWheelDelta), sizeof(properties->MouseWheelDelta), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(properties->Flags), sizeof(u_int16_t), 0)) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceivePoint(int fd, Point *pt) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = recv(fd, &(pt->X), sizeof(pt->X), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(pt->Y), sizeof(pt->Y), 0)) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceivePointerPoint(int fd, PointerPoint *ppt) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = ReceivePoint(fd, &(ppt->Position))) < 0)
        return -1;
    total += rc;
    if ((rc = ReceivePointerPointProperties(fd, &(ppt->Properties))) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceivePointerEventType(int fd, enum PointerEventType *eventType) {
    return recv(fd, eventType, 1, 0);
}

ssize_t ReceivePointerEvent(int fd, PointerEvent *ev) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = ReceivePointerEventType(fd, &(ev->type))) < 0)
        return -1;
    total += rc;
    if ((rc = ReceivePointerPoint(fd, &(ev->ptrPt))) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceiveConfig(int fd, ConfigData *data) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = recv(fd, &(data->Width), sizeof(data->Width), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(data->Height), sizeof(data->Height), 0)) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceiveWindowEventType(int fd, enum WindowEventType *eventType) {
    return recv(fd, eventType, 1, 0);
}

ssize_t ReceiveSize(int fd, Size *size) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = recv(fd, &(size->Width), sizeof(size->Width), 0)) < 0)
        return -1;
    total += rc;
    if ((rc = recv(fd, &(size->Height), sizeof(size->Height), 0)) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceiveWindow(int fd, Window_t *window) {
    return ReceiveSize(fd, &(window->size));
}

ssize_t ReceiveWindowEvent(int fd, WindowEvent *ev) {
    ssize_t rc;
    ssize_t total = 0;
    if ((rc = ReceiveWindowEventType(fd, &(ev->type))) < 0)
        return -1;
    total += rc;
    if ((rc = ReceiveWindow(fd, &(ev->window))) < 0)
        return -1;
    total += rc;
    return total;
}

ssize_t ReceiveEventType(int fd, enum EventType *eventType) {
    return recv(fd, eventType, 1, 0);
}
