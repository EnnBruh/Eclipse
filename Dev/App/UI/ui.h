#ifndef UI_H
#define UI_H

#include "core.h"
#include "Rendering/render.h"

typedef struct Button {
        f32vec2 pos;
        f32vec2 dim;

        char*   name;
        u32     color;
} Button;

#define is_button_under(_button, _coord)                                                                   \
        (((_button).pos.x <= (_coord).x && (_button).pos.x + (_button).dim.x >= (_coord).x) &&          \
         ((_button).pos.y <= (_coord).y && (_button).pos.y + (_button).dim.y >= (_coord).y))

ENNDEF_PUBLIC void ui_button_render(Button* butt) {
        DEBUG_TRACE();
        DEBUG_ASSERT(butt != NULL);

        render_text_push(butt -> pos, butt -> name, butt -> color, butt -> dim.y);

        DEBUG_UNTRACE();
}

#endif