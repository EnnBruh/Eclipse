#ifndef LAYERS_H
#define LAYERS_H

#include "core.h"
#include "Rendering/render.h"
#include "UI/ui.h"

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


extern i32vec4 window_viewport;

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

extern struct NetworkState {
        ENN_NETWORK_LOCALITY    choice;
        ENN_NETWORK_STATUS      status;

        Socket                  self_sock;
        NetworkInformation      self_info;

        struct sockaddr_in      friend_address;

        f64                     time_send;
        f64                     time_recv;
} net_state;

#endif