#ifndef LAYERS_H
#define LAYERS_H

#include "core.h"

#define LAYER_DEFINE(name)                                                      \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_init))(void);                    \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_term))(void);                    \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_render))(void);               \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_event))(Event* event);        \
        ENNDEF_PRIVATE void EXPAND(JOIN(name, _layer_on_update))(f64 dt)             

#define LAYER_ASSIGN(name)                                                      \
        .init = EXPAND(JOIN(name, _layer_init)),                                      \
        .term = EXPAND(JOIN(name, _layer_term)),                                      \
        .on_render = EXPAND(JOIN(name, _layer_on_render)),                            \
        .on_update = EXPAND(JOIN(name, _layer_on_update)),                            \
        .on_event  = EXPAND(JOIN(name, _layer_on_event))

LAYER_DEFINE(void);
LAYER_DEFINE(debug);
LAYER_DEFINE(network);

#endif