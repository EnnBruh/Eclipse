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

static char public_address[128];
static char local_address[128];
static vector(char) input_string;
static char error[256];
static Sprite background;


void network_layer_init(void) {
        DEBUG_TRACE();
        net_state.status = ENN_NETWORK_DISCONNECTED;
        net_state.time_since_send = 0.0;
        net_state.time_since_recv = 0.0;

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

        sprintf(public_address, "YOUR PUBLIC IP: %s:%" PRIu16, net_state.self_info.PUBLIC_IP, net_state.self_info.PUBLIC_PORT);
        sprintf(local_address, "YOUR LOCAL IP: %s:%" PRIu16, net_state.self_info.LOCAL_IP, net_state.self_info.LOCAL_PORT);

        vector_reserve(input_string, 64);

        background = render_sprite_create(&global_render.sprite_sheet,
                (i32vec2) { 0, 567 },
                (i32vec2) { 383, 783 });

        DEBUG_UNTRACE();
}

void network_layer_term(void) {
}

void network_layer_on_render(void) {
        DEBUG_TRACE();
        if (net_state.status == ENN_NETWORK_CONNECTED) {
                DEBUG_UNTRACE();
                return ;
        }
        render_proj_set((mat4) {
                1, 0, 0, 0,
                0, -1, 0, 0,
                0, 0, 1, 0,
                0, 0, 0, 1
        });

        render_sprite_push(
                (f32vec2) { -1.0, -1.0 },
                (f32vec2) { 1.0, 1.0 },
                &background);

        switch (net_state.status) {
                case ENN_NETWORK_DISCONNECTED:
                {
                        render_text_push(
                                (f32vec2) { -0.3, -0.8 },
                                "DISCONNECTED", 0xa1274eFF, 0.1);
                        break;
                }
                case ENN_NETWORK_CONNECTING:
                {
                        render_text_push(
                                (f32vec2) { -0.25, -0.8 },
                                "CONNECTING", 0xffd37fFF, 0.1);
                        break;
                }
                case ENN_NETWORK_CONNECTED:
                {
                        render_text_push(
                                (f32vec2) { -0.225, -0.8 },
                                "CONNECTED", 0xadef8fFF, 0.1);
                        break;
                }
                default: break;
        }

        render_text_push(
                (f32vec2) { -0.9, -0.5 },
                public_address, 0xFFFFFFFF, 0.1);

        render_text_push(
                (f32vec2) { -0.9, -0.35 },
                local_address, 0xFFFFFFFF, 0.1);

        render_rectangle_push(
                (f32vec2) { -0.8, 0.0 },
                (f32vec2) { 0.8, 0.1 },
                0xAAAAAA80);

        if (vector_size(input_string) == 0) {
                render_text_push(
                    (f32vec2){-0.8, 0.0},
                    "YOUR FRIEND'S PUBLIC IP", 0x505050AA, 0.09);
        } else {
                render_text_push(
                    (f32vec2){-0.8, 0.0},
                    input_string.data + input_string.start, 0xFFFFFFFF, 0.09);
        }

        render_text_push(
                (f32vec2) { -0.8, 0.5 },
                error, 0xa1274eFF, 0.05);
        DEBUG_UNTRACE();
}

void network_layer_on_update(f64 dt) {
        DEBUG_TRACE();
        if (net_state.status == ENN_NETWORK_DISCONNECTED) return;

        net_state.time_since_send += dt;
        net_state.time_since_recv += dt;

        if (net_state.status == ENN_NETWORK_CONNECTED && net_state.time_since_recv > 10.0) {
                net_state.status = ENN_NETWORK_DISCONNECTED;
                sprintf(error, "ERROR: CONNECTION TIMED OUT");
                layer_set_inactive(game_layer_id);
                return;
        }

        if (net_state.status == ENN_NETWORK_CONNECTING && net_state.time_since_recv > 10.0) {
                net_state.status = ENN_NETWORK_DISCONNECTED;
                sprintf(error, "ERROR: CONNECTION TIMED OUT");
                layer_set_inactive(game_layer_id);
                return;
        }

        byte buffer[512];
        struct sockaddr_in from;
        socklen_t from_len = (sizeof from);

        while (true) {
                i32 bytes = recvfrom(net_state.self_sock, (char*)buffer, (int)(sizeof buffer), 0, (struct sockaddr*)&from, &from_len);
                if (bytes <= 0) break;

                if (from.sin_addr.s_addr != net_state.friend_address.sin_addr.s_addr) continue;

                if (bytes >= 5) {
                        u32 magic = ((u32)buffer[0] << 24) | ((u32)buffer[1] << 16) | ((u32)buffer[2] << 8) | buffer[3];
                        
                        if (magic == ENN_MAGIC_NUMBER) {
                                net_state.time_since_recv = 0.0;
                                NetworkPacketType type = (NetworkPacketType)buffer[4];

                                if (type == ENN_PKT_CONNECT_REQ) {
                                        network_packet_send(ENN_PKT_CONNECT_ACK, NULL, 0);
                                        if (net_state.status == ENN_NETWORK_CONNECTING) {
                                                net_state.status = ENN_NETWORK_CONNECTED;
                                                error[0] = '\0';
                                                layer_set_active(select_layer_id);
                                        }
                                } else if (type == ENN_PKT_CONNECT_ACK && net_state.status == ENN_NETWORK_CONNECTING) {
                                        net_state.status = ENN_NETWORK_CONNECTED;
                                        error[0] = '\0';
                                        layer_set_active(select_layer_id);
                                } else if (type == ENN_PKT_GAME_DATA && net_state.status == ENN_NETWORK_CONNECTED) {
                                        if (bytes >= (i32)(5 + (sizeof (PlayerData)))) {
                                                static PlayerData data;
                                                memcpy(&data, &buffer[5], (sizeof (PlayerData)));
                                                game_layer_on_event(&(Event) {
                                                        .type = ENN_NETWORK_GAMEDATA_EVENT,
                                                        .data = (void*)&data
                                                });
                                        }
                                } else if (type == ENN_PKT_SELECT_DATA && net_state.status == ENN_NETWORK_CONNECTED) {
                                        if (bytes >= (i32)(5 + (sizeof (SelectData)))) {
                                                static SelectData data;
                                                memcpy(&data, &buffer[5], (sizeof (SelectData)));
                                                select_layer_on_event(&(Event) {
                                                        .type = ENN_NETWORK_GAMEDATA_EVENT,
                                                        .data = (void*)&data
                                                });
                                        }
                                }
                        }
                }
        }

        if (net_state.status == ENN_NETWORK_CONNECTING && net_state.time_since_send >= 0.1) {
                network_packet_send(ENN_PKT_CONNECT_REQ, NULL, 0);
        } else if (net_state.status == ENN_NETWORK_CONNECTED && net_state.time_since_send >= 1.0) {
                network_packet_send(ENN_PKT_CONNECT_MNT, NULL, 0);
        }
        DEBUG_UNTRACE();
}

void network_layer_on_event(Event* event) {
        DEBUG_TRACE();
        if (net_state.status == ENN_NETWORK_CONNECTED) {
                DEBUG_UNTRACE();
                return ;
        }
        switch (event -> type) {
                case ENN_INPUT_TEXT_EVENT:
                {
                        struct { u32 code; }* data = event -> data;
                        if (data -> code < ENN_FONT_ATLAS_FIRST_CHAR || data -> code > ENN_FONT_ATLAS_LAST_CHAR) break;
                        char ch = (char)data -> code;
                        vector_push_back(input_string, ch);
                        break;
                }
                case ENN_INPUT_KEY_EVENT:
                {
                        struct { i32 key, action; }* data = event -> data;
                        if (data -> key == GLFW_KEY_BACKSPACE && (data -> action == GLFW_PRESS || data -> action == GLFW_REPEAT)) {
                                if (vector_size(input_string) <= 0) break;
                                vector_pop_back(input_string);
                                input_string.data[input_string.end] = '\0';
                        }

                        if (data -> key == GLFW_KEY_ENTER && (data -> action == GLFW_PRESS || data -> action == GLFW_REPEAT)) {
                                net_state.status = ENN_NETWORK_CONNECTING;
                                char address[256] = { 0 };
                                memcpy(address, input_string.data + input_string.start, (sizeof (char)) * (vector_size(input_string)));
                                char* discrim = strchr(address, ':');
                                if (discrim == NULL) {
                                        net_state.status = ENN_NETWORK_DISCONNECTED;
                                        sprintf(error, "ERROR: YOU NEED TO SPECIFY THE IP AND PORT X.X.X.X:XXXX");
                                        break;
                                }
                                *discrim = '\0';
                                net_state.friend_address.sin_family = AF_INET;
                                
                                u16 port = 0;
                                sscanf(discrim + 1, "%" SCNu16, &port);
                                net_state.friend_address.sin_port = htons(port);
                                net_state.friend_address.sin_addr.s_addr = inet_addr(address);
                                LOG("[Network] Connecting to %s:%" PRIu16, address, port);
                                net_state.time_since_send = 0.1;
                                net_state.time_since_recv = 0.0;
                                break;
                        }

                        if (data -> key == GLFW_KEY_ESCAPE && data -> action == GLFW_PRESS) {
                                layer_set_inactive(network_layer_id);
                                layer_set_active(menu_layer_id);
                        }
                }
                default: break;
        }
        DEBUG_UNTRACE();
}