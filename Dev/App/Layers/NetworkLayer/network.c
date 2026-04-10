#include "Layers/layers.h"

LayerID network_layer_id;

struct NetworkState net_state;

ENNDEF_PUBLIC i32 network_get_error(void) {
#       if ENN_PLATFORM == ENN_WINDOWS
                return WSAGetLastError();
#       else
                return errno;
#       endif
}

#define DNS_SERVER_IP "8.8.8.8"
#define DNS_SERVER_PORT 53


ENNDEF_PUBLIC void network_get_local(void) {
        DEBUG_TRACE();
        strcpy(net_state.self_info.LOCAL_IP, "127.0.0.1");
        Socket help = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (ENN_IS_SOCKET_INVALID(help)) {
                DEBUG_LOG("[Network] Could not create a helper socket for Local IP discovery. Error code %" PRIi32, network_get_error());
                DEBUG_UNTRACE();
                return ;
        }

        struct sockaddr_in server; memset(&server, 0x0, (sizeof server));
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = inet_addr(DNS_SERVER_IP); 
        server.sin_port = htons(DNS_SERVER_PORT);

        if (connect(help, (struct sockaddr*)&server, (sizeof server)) == 0) {
                struct sockaddr_in name;
                socklen_t name_len = (sizeof name);
                getsockname(help, (struct sockaddr*)&name, &name_len);

                u32 ip = ntohl(name.sin_addr.s_addr);
                sprintf(net_state.self_info.LOCAL_IP, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
        } else DEBUG_LOG("[Network] Could not get Local IP");
        network_socket_close(help);
        DEBUG_UNTRACE();
}

#define STUN_SERVER_IP "stun.l.google.com"
#define STUN_SERVER_PORT 19302
#define XOR_MAPPED_ADDRESS 0x0020

ENNDEF_PUBLIC void network_get_public(void) {
        DEBUG_TRACE();
        strcpy(net_state.self_info.PUBLIC_IP, "Unknown");

        struct addrinfo hints; memset(&hints, 0x0, (sizeof hints));
        struct addrinfo* result = NULL;

        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_DGRAM;

        if (getaddrinfo(STUN_SERVER_IP, TO_STR(STUN_SERVER_PORT), &hints, &result) != 0) {
                DEBUG_LOG("[Network] Could not resolve STUN Server %s:%s", STUN_SERVER_IP, TO_STR(STUN_SERVER_PORT));
                DEBUG_UNTRACE();
                return ;
        }

        /*
        * Binding Request to a STUN Server to get the Public Ip address and port
        * Message is built according to the RFC5389 Standard
        * See https://datatracker.ietf.org/doc/html/rfc5389#section-6
        */
        byte request[20]; memset(request, 0x0, (sizeof request));
        request[0] = 0x00; request[1] = 0x01; // Request type 0x0001 for Bind Request
        // request[2] = 0x0; request[3] = 0x0 // Message Length 0
        request[4] = 0x21; request[5] = 0x12; request[6] = 0xA4; request[7] = 0x42; // Magic Cookie MUST be 0x2112A442

        if (sendto(net_state.self_sock, (const char*)request, (int)(sizeof request), 0, result -> ai_addr, result -> ai_addrlen) == ENN_SOCKET_ERR) {
                DEBUG_LOG("[Network] Cannot send bind request to STUN Server %s:%s", STUN_SERVER_IP, TO_STR(STUN_SERVER_PORT));
                freeaddrinfo(result);
                DEBUG_UNTRACE();
                return ;
        }

        freeaddrinfo(result);

        fd_set socket_handle; FD_ZERO(&socket_handle);
        FD_SET(net_state.self_sock, &socket_handle);
        struct timeval wait = {
                .tv_sec = 3,
                .tv_usec = 0
        };

        i32 select_status = select((int)net_state.self_sock + 1, &socket_handle, NULL, NULL, &wait);
        // Wait 3 seconds for response from STUN server

        if (select_status < 0) {
                DEBUG_LOG("[Network] select() failed. No STUN Server response. Error code %" PRIi32, network_get_error());
        } else if (select_status == 0) {
                DEBUG_LOG("[Network] STUN request timed out");
        } else {
                byte response[256]; memset(response, 0x0, (sizeof response));
                i32 response_len = recvfrom(net_state.self_sock, (char*)response, (int)(sizeof response), 0, NULL, NULL);
                if (response_len >= 20) {
                        i32 offset = 20;
                        u16 message_len = (response[2] << 8) | response[3];
                        while (offset + 4 <= 20 + message_len) {
                                u16 type = (response[offset] << 8) | response[offset + 1];
                                u16 len  = (response[offset + 2] << 8) | response[offset + 3];

                                if (type == XOR_MAPPED_ADDRESS && len >= 8) {
                                        net_state.self_info.PUBLIC_PORT  = (response[offset + 6] << 8) | response[offset + 7];
                                        net_state.self_info.PUBLIC_PORT ^= 0x2112;

                                        u32 ip  = (response[offset + 8] << 24) | (response[offset + 9] << 16) | (response[offset + 10] << 8) | response[offset + 11];
                                            ip ^= 0x2112A442;
                                        
                                        sprintf(net_state.self_info.PUBLIC_IP, "%u.%u.%u.%u", (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
                                        break;
                                }

                                offset += 4 + len + ((4 - (len % 4)) % 4);
                        }
                }
        }

        DEBUG_UNTRACE();
}

void network_layer_init(void) {
        DEBUG_TRACE();
        net_state.status = ENN_NETWORK_DISCONNECTED;

        net_state.self_sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
        if (ENN_IS_SOCKET_INVALID(net_state.self_sock)) {
                DEBUG_LOG("[Network] Could not create a UDP socket. Error code: %" PRIi32, network_get_error());
                strcpy(net_state.self_info.LOCAL_IP, "Unknown");
                strcpy(net_state.self_info.PUBLIC_IP, "Unknown");
                DEBUG_UNTRACE();
                return ;
        }

        struct sockaddr_in local; memset(&local, 0x0, (sizeof local));
        local.sin_family = AF_INET;
        local.sin_port = 0;
        local.sin_addr.s_addr = INADDR_ANY;

        if (bind(net_state.self_sock, (struct sockaddr*)&local, (sizeof local)) == ENN_SOCKET_ERR) {
                DEBUG_LOG("[Network] Could not bind UDP socket. Error code: %" PRIi32, network_get_error());
                strcpy(net_state.self_info.LOCAL_IP, "Unknown");
                strcpy(net_state.self_info.PUBLIC_IP, "Unknown");
                DEBUG_UNTRACE();
                return ;
        }

        struct sockaddr_in bound;
        socklen_t bound_len = (sizeof bound);
        getsockname(net_state.self_sock, (struct sockaddr*)&bound, &bound_len);
        net_state.self_info.LOCAL_PORT = ntohs(bound.sin_port);

        network_get_local();
        network_get_public();

#       if ENN_PLATFORM == ENN_WINDOWS
                u_long mode = 1;
                ioctlsocket(net_state.self_sock, FIONBIO, &mode);
#       else
                fcntl(net_state.self_sock, F_SETFL, O_NONBLOCK);
#       endif

        LOG("[Network] Public IP: %s:%" PRIu16, net_state.self_info.PUBLIC_IP, net_state.self_info.PUBLIC_PORT);
        LOG("[Network] Local IP: %s:%" PRIu16, net_state.self_info.LOCAL_IP, net_state.self_info.LOCAL_PORT);

        DEBUG_UNTRACE();
}

void network_layer_term(void) {
}

void network_layer_on_render(void) {
}

void network_layer_on_update(f64 dt) {
}

void network_layer_on_event(Event* event) {
}