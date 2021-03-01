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

ssize_t ReceiveEventType(int fd, enum EventType *eventType) {
    return recv(fd, eventType, 1, 0);
}
