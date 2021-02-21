#include "include/OpenVTab.h"
#include <arpa/inet.h>
#include <libevdev/libevdev-uinput.h>
#include <libevdev/libevdev.h>
#include <linux/uinput.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int is_running = 1;
struct libevdev *dev;
struct libevdev_uinput *uidev;

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

void handle_window_event(int socket) {
    WindowEvent ev = {};
    ReceiveWindowEvent(socket, &ev);
    printf("WindowEvent type: %i\n", ev.type);
    printf("Width: %f\n", ev.window.size.Width);
    printf("Height: %f\n\n", ev.window.size.Height);
}

void handle_pointer_event(int socket) {
    PointerEvent ev = {};
    ReceivePointerEvent(socket, &ev);
    printf("PointerEvent type: %i\n", ev.type);
    printf("X: %f\n", ev.ptrPt.Position.X);
    printf("Y: %f\n", ev.ptrPt.Position.Y);
    printf("Pressure: %f\n", ev.ptrPt.Properties.Pressure);
    printf("Flags: %04X\n\n", ev.ptrPt.Properties.Flags);
}

void handle_client(int socket) {
    ConfigData data = {};
    enum EventType eventType = 0;

    //    ReceiveConfig(socket, &data);
    //    printf("Board width: %f, height: %f\n", data.Width, data.Height);
    ssize_t rc;
    while ((rc = ReceiveEventType(socket, &eventType)) > 0) {
        switch (eventType) {
            case Pointer:
                handle_pointer_event(socket);
                break;
            case Window:
                handle_window_event(socket);
                break;
            default:
                fprintf(stderr, "Unknown event type: %i\n", eventType);
                break;
        }
    }
    if (rc < 0) {
        perror("Error occurred while receiving data");
        exit(1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>", argv[0]);
        exit(1);
    }

    struct sigaction sa = {
            .sa_handler = handle_signal};

    if (sigaction(SIGUSR1, &sa, NULL) < 0) {
        perror("Cannot register signal action");
        exit(1);
    }

    if (sigaction(SIGINT, &sa, NULL) < 0) {
        perror("Cannot register signal action");
        exit(1);
    }

    uint16_t port = strtol(argv[1], NULL, 10);

    int sock;
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Cannot create socket");
        exit(1);
    }

    struct sockaddr_in address = {
            .sin_addr = INADDR_ANY,
            .sin_port = htons(port),
            .sin_family = AF_INET};

    if (bind(sock, (struct sockaddr *) &address, sizeof(address)) < 0) {
        perror("Cannot bind socket");
        exit(1);
    }

    if (listen(sock, 5) < 0) {
        perror("Cannot listen to socket");
        exit(1);
    }

    dev = libevdev_new();
    libevdev_set_name(dev, "test device");
    libevdev_enable_event_type(dev, EV_REL);
    libevdev_enable_event_code(dev, EV_REL, REL_X, NULL);
    libevdev_enable_event_code(dev, EV_REL, REL_Y, NULL);
    libevdev_enable_event_type(dev, EV_KEY);
    libevdev_enable_event_code(dev, EV_KEY, BTN_LEFT, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_MIDDLE, NULL);
    libevdev_enable_event_code(dev, EV_KEY, BTN_RIGHT, NULL);
    int rc;
    if ((rc = libevdev_uinput_create_from_device(dev, LIBEVDEV_UINPUT_OPEN_MANAGED, &uidev)) < 0) {
        perror("Create uinput device");
        return rc;
    }

    printf("Server listening on port %i\n", port);

    while (is_running) {
        int client_sock;
        struct sockaddr_in client_address;
        uint client_address_length = sizeof(client_address);

        if ((client_sock = accept(sock, (struct sockaddr *) &client_address, &client_address_length)) < 0) {
            perror("Cannot accept client");
            exit(1);
        }

        printf("New client connected\n");
        handle_client(client_sock);
        close(client_sock);
        printf("Client closed\n");
    }

    libevdev_uinput_destroy(uidev);

    close(sock);

    return 0;
}
