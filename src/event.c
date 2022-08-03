#ifndef EVENT_HEADER
#define EVENT_HEADER
#include "types.h"

// Variaveis
typedef f64 Time;
typedef u32 Color;

// Estruturas Usadas
typedef enum _EventType {
    EVENT_arrival, EVENT_leave,
} EventType;

typedef struct _Person {
    Time arrived_time;
    Color color;
} Person;

typedef struct _Event {
    EventType type;
    Time time;
    Person person;
} Event;

Event create_event_arrival(Time t, Color color);
Event create_event_leave(Time t, Person p);

#endif // EVENT_HEADER

#ifdef EVENT_IMPL
#undef EVENT_IMPL
#include <assert.h>

// Criamos o evento de chegada com tempo t e uma cor
Event create_event_arrival(Time t, Color color) {
    const Event ret = {
        .type = EVENT_arrival,
        .time = t,
        .person = {
            .arrived_time = t,
            .color = color,
        },
    };
    return ret;
}

// Criamos o evento de saida com tempo t e uma cor
Event create_event_leave(Time t, Person p) {
    const Event ret = {
        .type = EVENT_leave,
        .time = t,
        .person = p,
    };
    return ret;
}

#endif // EVENT_IMPL
