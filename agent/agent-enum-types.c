//
// Created by 沈昊 on 2022/12/1.
//
#include "agent-enum-types.h"
#include "agent.h"
#include "gobject/genums.h"
unsigned long nice_nomination_mode_get_type() {
    static unsigned long type = 0;
    if (!type) {
        static const GEnumValue values[] = {
                {NICE_NOMINATION_MODE_REGULAR, "NICE_NOMINATION_MODE_REGULAR", "regular"},
                {NICE_NOMINATION_MODE_AGGRESSIVE, "NICE_NOMINATION_MODE_AGGRESSIVE", "aggressive"},
                {0, NULL, NULL}};
        type = g_enum_register_static("NiceNominationMode", values);
    }
    return type;
}