#include "OpenVTab.h"
#include <X11/Xlib.h>
#include <arpa/inet.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/uinput.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define OVT_PRESSURE_MIN 0.
#define OVT_PRESSURE_MAX 2147483646.

int is_running = 1;
struct libevdev *dev;
struct libevdev_uinput *uidev;
Display *display;
Screen *screen;

void die(const char *why) {
    perror(why);
    exit(1);
}

int warning(const char *what) {
    return fprintf(stderr, "Warning: %s\n", what);
}

void handle_signal(int sig) {
    switch (sig) {
        case SIGUSR1:
        case SIGINT:
            is_running = 0;
            break;
        default:
            break;
    }
}

void print_flags(enum PointerPointPropertiesFlags flags) {
    printf("flags: ");
    if (flags == 0) {
        printf("None\n");
        return;
    }
    if (flags & TouchConfidence)
        printf("TouchConfidence, ");
    if (flags & LeftButtonPressed)
        printf("LeftButtonPressed, ");
    if (flags & RightButtonPressed)
        printf("RightButtonPressed, ");
    if (flags & MiddleButtonPressed)
        printf("MiddleButtonPressed, ");
    if (flags & Inverted)
        printf("Inverted, ");
    if (flags & Eraser)
        printf("Eraser, ");
    if (flags & HorizontalMouseWheel)
        printf("HorizontalMouseWheel, ");
    if (flags & Primary)
        printf("Primary, ");
    if (flags & InRange)
        printf("InRange, ");
    if (flags & Canceled)
        printf("Canceled, ");
    if (flags & BarrelButtonPressed)
        printf("BarrelButtonPressed, ");
    if (flags & XButton1Pressed)
        printf("XButton1Pressed, ");
    if (flags & XButton2Pressed)
        printf("XButton2Pressed");
    printf("\n");
}

void print_pointer_event_type(enum PointerEventType type) {
    printf("type: ");
    switch (type) {
        case PointerCanceled:
            printf("PointerCanceled");
            break;
        case PointerCaptureLost:
            printf("PointerCaptureLost");
            break;
        case PointerEntered:
            printf("PointerEntered");
            break;
        case PointerExited:
            printf("PointerExited");
            break;
        case PointerMoved:
            printf("PointerMoved");
            break;
        case PointerPressed:
            printf("PointerPressed");
            break;
        case PointerReleased:
            printf("PointerReleased");
            break;
        case PointerWheelChanged:
            printf("PointerWheelChanged");
            break;
        default:
            printf("Unknown pointer event type");
    }
    printf("\n");
}

int perror_code(const char *str, int code) {
    return fprintf(stderr, "%s: %s\n", str, strerror(code));
}

void handle_pointer_event(int socket) {
    PointerEvent ev = {};
    ReceivePointerEvent(socket, &ev);
    print_pointer_event_type(ev.type);
    print_flags(ev.ptrPt.Properties.Flags);

    int rc;
    rc = ev.ptrPt.Properties.Flags & LeftButtonPressed ? libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, 1)
                                                       : libevdev_uinput_write_event(uidev, EV_KEY, BTN_LEFT, 0);
    if (rc < 0)
        perror_code("Cannot write event BTN_LEFT", rc);

    rc = ev.ptrPt.Properties.Flags & RightButtonPressed ? libevdev_uinput_write_event(uidev, EV_KEY, BTN_RIGHT, 1)
                                                        : libevdev_uinput_write_event(uidev, EV_KEY, BTN_RIGHT, 0);
    if (rc < 0)
        perror_code("Cannot write event BTN_RIGHT", rc);

    rc = ev.ptrPt.Properties.Flags & MiddleButtonPressed ? libevdev_uinput_write_event(uidev, EV_KEY, BTN_MIDDLE, 1)
                                                         : libevdev_uinput_write_event(uidev, EV_KEY, BTN_MIDDLE, 0);
    if (rc < 0)
        perror_code("Cannot write event BTN_MIDDLE", rc);

    rc = libevdev_uinput_write_event(uidev, EV_ABS, ABS_PRESSURE, (int) (ev.ptrPt.Properties.Pressure * OVT_PRESSURE_MAX));
    if (rc < 0)
        perror_code("Cannot write event ABS_PRESSURE", rc);

    libevdev_uinput_write_event(uidev, EV_ABS, ABS_X, (int) (ev.ptrPt.Position.X * screen->width));
    libevdev_uinput_write_event(uidev, EV_ABS, ABS_Y, (int) (ev.ptrPt.Position.Y * screen->height));
}

void handle_client(int socket) {
    enum EventType eventType = 0;

    ssize_t rc;
    while ((rc = ReceiveEventType(socket, &eventType)) > 0) {
        switch (eventType) {
            case Pointer:
                handle_pointer_event(socket);
                break;
            default:
                fprintf(stderr, "Unknown event type: %i\n", eventType);
                break;
        }
    }
    if (rc < 0)
        die("Error occurred while receiving data");
}

int enable_abs_event_code(struct libevdev *_dev, int code, int min, int max) {
    struct input_absinfo absinfo = {
            .minimum = min,
            .maximum = max,
    };

    return libevdev_enable_event_code(_dev, EV_ABS, code, &absinfo);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        exit(1);
    }

    struct sigaction sa = {
            .sa_handler = handle_signal};

    if (sigaction(SIGUSR1, &sa, NULL) < 0)
        die("Cannot register signal action");

    if (sigaction(SIGINT, &sa, NULL) < 0)
        die("Cannot register signal action");

    uint16_t port = strtol(argv[1], NULL, 10);

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        die("Cannot create socket");

    struct sockaddr_in address = {
            .sin_addr = INADDR_ANY,
            .sin_port = htons(port),
            .sin_family = AF_INET};

    if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0)
        die("Cannot bind socket");

    if (listen(sock, 5) < 0)
        die("Cannot listen to socket");

    if (!(display = XOpenDisplay(NULL)))
        warning("Cannot get display");
    if (!(screen = DefaultScreenOfDisplay(display)))
        warning("Cannot get screen");

    dev = libevdev_new();
    libevdev_set_name(dev, "openvtab");
    if (libevdev_enable_event_type(dev, EV_ABS) < 0 ||
        enable_abs_event_code(dev, ABS_X, 0, screen->width) < 0 ||
        enable_abs_event_code(dev, ABS_Y, 0, screen->height) < 0 ||
        enable_abs_event_code(dev, ABS_PRESSURE, OVT_PRESSURE_MIN, OVT_PRESSURE_MAX))
        warning("Cannot enable EV_ABS");

    if (libevdev_enable_event_type(dev, EV_KEY) < 0 ||
        libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL) < 0 ||
        libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, NULL) < 0 ||
        libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL) < 0)
        warning("Cannot enable EV_KEY");

    if (libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev) < 0)
        die("Cannot create uinput device");

    printf("Server listening on port %i\n", port);

    while (is_running) {
        int client_sock;
        struct sockaddr_in client_address;
        uint client_address_length = sizeof(client_address);

        if ((client_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_length)) < 0)
            die("Cannot accept client");

        printf("New client connected\n");
        handle_client(client_sock);
        close(client_sock);
        printf("Client closed\n");
    }

    libevdev_uinput_destroy(uidev);

    close(sock);

    return 0;
}
