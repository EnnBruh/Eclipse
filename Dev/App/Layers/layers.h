#ifndef LAYERS_H
#define LAYERS_H

#include "core.h"
#include "Rendering/render.h"
#include "UI/ui.h"
#include "GameLayer/player.h"

#define LAYER_DEFINE(name)                                                            \
        extern LayerID EXPAND(JOIN(name, _layer_id));                                 \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_init))(void);                    \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_term))(void);                    \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_render))(void);               \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_event))(Event* event);        \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_update))(f64 dt)             

#define LAYER_ASSIGN(name)                                                            \
        .init = EXPAND(JOIN(name, _layer_init)),                                      \
        .term = EXPAND(JOIN(name, _layer_term)),                                      \
        .on_render = EXPAND(JOIN(name, _layer_on_render)),                            \
        .on_update = EXPAND(JOIN(name, _layer_on_update)),                            \
        .on_event  = EXPAND(JOIN(name, _layer_on_event))

LAYER_DEFINE(void);
LAYER_DEFINE(debug);
LAYER_DEFINE(network);
LAYER_DEFINE(menu);
LAYER_DEFINE(game);
LAYER_DEFINE(select);


extern i32vec4 window_viewport;

ENNDEF_PUBLIC f32vec2 screen_to_ndc(f32vec2 screen) {
        return (f32vec2) {
                .x = (screen.x - window_viewport.x) / ((f32)(window_viewport.z) * 0.5) - 1.0,
                .y = (screen.y - window_viewport.y) / ((f32)(window_viewport.w) * 0.5) - 1.0
        };
}
typedef struct __attribute__((packed)) PlayerData {
        f32vec2                 pos;
        ENN_PLAYER_STATE        state;
        byte                    flags;
} PlayerData;

#define ENN_CHOICE_SHADOW 0x01
#define ENN_CHOICE_LIGHT 0x02
typedef struct __attribute__((packed)) SelectData {
        byte choice;
} SelectData;
typedef enum ENN_NETWORK_LOCALITY {
        ENN_NETWORK_LOCALHOST,
        ENN_NETWORK_LAN,
        ENN_NETWORK_WAN,
        ENN_NETWORK_UNKNOWN
} ENN_NETWORK_LOCALITY;

typedef enum ENN_NETWORK_STATUS {
        ENN_NETWORK_CONNECTED,
        ENN_NETWORK_DISCONNECTED,
        ENN_NETWORK_CONNECTING
} ENN_NETWORK_STATUS;

typedef struct NetworkInformation {
        u16     LOCAL_PORT;
        char    LOCAL_IP[64];

        u16     PUBLIC_PORT;
        char    PUBLIC_IP[64];
} NetworkInformation;

typedef enum NetworkPacketType {
        ENN_PKT_CONNECT_REQ = 0x01,
        ENN_PKT_CONNECT_ACK = 0x02,
        ENN_PKT_CONNECT_MNT = 0x03,
        ENN_PKT_GAME_DATA   = 0x04,
        ENN_PKT_SELECT_DATA = 0x05
} NetworkPacketType;
extern struct NetworkState {
        ENN_NETWORK_LOCALITY    choice;
        ENN_NETWORK_STATUS      status;

        Socket                  self_sock;
        NetworkInformation      self_info;

        struct sockaddr_in      friend_address;

        f64                     time_since_send;
        f64                     time_since_recv;
} net_state;

#define ENN_MAGIC_NUMBER 0x45434C50

ENNDEF_PUBLIC void network_packet_send(NetworkPacketType type, const byte* payload, u32 size) {
        DEBUG_TRACE();
        if (net_state.status == ENN_NETWORK_DISCONNECTED) return;
        if (size > 507) return;

        byte buffer[512];
        buffer[0] = (ENN_MAGIC_NUMBER >> 24) & 0xFF;
        buffer[1] = (ENN_MAGIC_NUMBER >> 16) & 0xFF;
        buffer[2] = (ENN_MAGIC_NUMBER >> 8) & 0xFF;
        buffer[3] = ENN_MAGIC_NUMBER & 0xFF;
        buffer[4] = (byte)type;

        if (payload != NULL && size > 0) {
                memcpy(&buffer[5], payload, size);
        }

        sendto(net_state.self_sock, (const char*)buffer, size + 5, 0, (struct sockaddr*)&net_state.friend_address, (sizeof net_state.friend_address));
        net_state.time_since_send = 0.0;
        DEBUG_TRACE();
}
#endif