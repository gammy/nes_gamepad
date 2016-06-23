/* This file is part of uinput-nes.
 *
 * uinput-nes is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * uinput-nes is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with uinput-nes.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "kbd.h"

const struct kbd_keyval const kbd_list[] = {
    {"ESC",         KEY_ESC},
    {"1",           KEY_1},
    {"2",           KEY_2},
    {"3",           KEY_3},
    {"4",           KEY_4},
    {"5",           KEY_5},
    {"6",           KEY_6},
    {"7",           KEY_7},
    {"8",           KEY_8},
    {"9",           KEY_9},
    {"0",           KEY_0},
    {"MINUS",       KEY_MINUS},
    {"EQUAL",       KEY_EQUAL},
    {"BACKSPACE",   KEY_BACKSPACE},
    {"TAB",         KEY_TAB},
    {"Q",           KEY_Q},
    {"W",           KEY_W},
    {"E",           KEY_E},
    {"R",           KEY_R},
    {"T",           KEY_T},
    {"Y",           KEY_Y},
    {"U",           KEY_U},
    {"I",           KEY_I},
    {"O",           KEY_O},
    {"P",           KEY_P},
    {"LEFTBRACE",   KEY_LEFTBRACE},
    {"RIGHTBRACE",  KEY_RIGHTBRACE},
    {"ENTER",       KEY_ENTER},
    {"LEFTCTRL",    KEY_LEFTCTRL},
    {"A",           KEY_A},
    {"S",           KEY_S},
    {"D",           KEY_D},
    {"F",           KEY_F},
    {"G",           KEY_G},
    {"H",           KEY_H},
    {"J",           KEY_J},
    {"K",           KEY_K},
    {"L",           KEY_L},
    {"SEMICOLON",   KEY_SEMICOLON},
    {"APOSTROPHE",  KEY_APOSTROPHE},
    {"GRAVE",       KEY_GRAVE},
    {"LEFTSHIFT",   KEY_LEFTSHIFT},
    {"BACKSLASH",   KEY_BACKSLASH},
    {"Z",           KEY_Z},
    {"X",           KEY_X},
    {"C",           KEY_C},
    {"V",           KEY_V},
    {"B",           KEY_B},
    {"N",           KEY_N},
    {"M",           KEY_M},
    {"COMMA",       KEY_COMMA},
    {"DOT",         KEY_DOT},
    {"SLASH",       KEY_SLASH},
    {"RIGHTSHIFT",  KEY_RIGHTSHIFT},
    {"KPASTERISK",  KEY_KPASTERISK},
    {"LEFTALT",     KEY_LEFTALT},
    {"SPACE",       KEY_SPACE},
    {"CAPSLOCK",    KEY_CAPSLOCK},
    {"F1",          KEY_F1},
    {"F2",          KEY_F2},
    {"F3",          KEY_F3},
    {"F4",          KEY_F4},
    {"F5",          KEY_F5},
    {"F6",          KEY_F6},
    {"F7",          KEY_F7},
    {"F8",          KEY_F8},
    {"F9",          KEY_F9},
    {"F10",         KEY_F10},
    {"NUMLOCK",     KEY_NUMLOCK},
    {"SCROLLLOCK",  KEY_SCROLLLOCK},
    {"KP7",         KEY_KP7},
    {"KP8",         KEY_KP8},
    {"KP9",         KEY_KP9},
    {"KPMINUS",     KEY_KPMINUS},
    {"KP4",         KEY_KP4},
    {"KP5",         KEY_KP5},
    {"KP6",         KEY_KP6},
    {"KPPLUS",      KEY_KPPLUS},
    {"KP1",         KEY_KP1},
    {"KP2",         KEY_KP2},
    {"KP3",         KEY_KP3},
    {"KP0",         KEY_KP0},
    {"KPDOT",       KEY_KPDOT},
    {"102ND",       KEY_102ND},
    {"F11",         KEY_F11},
    {"F12",         KEY_F12},
    {"RO",          KEY_RO},
    {"KATAKANA",    KEY_KATAKANA},
    {"HIRAGANA",    KEY_HIRAGANA},
    {"HENKAN",      KEY_HENKAN},
    {"MUHENKAN",    KEY_MUHENKAN},
    {"KPJPCOMMA",   KEY_KPJPCOMMA},
    {"KPENTER",     KEY_KPENTER},
    {"RIGHTCTRL",   KEY_RIGHTCTRL},
    {"KPSLASH",     KEY_KPSLASH},
    {"SYSRQ",       KEY_SYSRQ},
    {"RIGHTALT",    KEY_RIGHTALT},
    {"LINEFEED",    KEY_LINEFEED},
    {"HOME",        KEY_HOME},
    {"UP",          KEY_UP},
    {"PAGEUP",      KEY_PAGEUP},
    {"LEFT",        KEY_LEFT},
    {"RIGHT",       KEY_RIGHT},
    {"END",         KEY_END},
    {"DOWN",        KEY_DOWN},
    {"PAGEDOWN",    KEY_PAGEDOWN},
    {"INSERT",      KEY_INSERT},
    {"DELETE",      KEY_DELETE},
    {"MACRO",       KEY_MACRO},
    {"MUTE",        KEY_MUTE},
    {"VOLUMEDOWN",  KEY_VOLUMEDOWN},
    {"VOLUMEUP",    KEY_VOLUMEUP},
    {"KPEQUAL",     KEY_KPEQUAL},
    {"KPPLUSMINUS", KEY_KPPLUSMINUS},
    {"PAUSE",       KEY_PAUSE},
    {"KPCOMMA",     KEY_KPCOMMA},
    {"HANGEUL",     KEY_HANGEUL},
    {"HANJA",       KEY_HANJA},
    {"YEN",         KEY_YEN},
    {"LEFTMETA",    KEY_LEFTMETA},
    {"RIGHTMETA",   KEY_RIGHTMETA},
    {"COMPOSE",     KEY_COMPOSE},
    {"F13",         KEY_F13},
    {"F14",         KEY_F14},
    {"F15",         KEY_F15},
    {"F16",         KEY_F16},
    {"F17",         KEY_F17},
    {"F18",         KEY_F18},
    {"F19",         KEY_F19},
    {"F20",         KEY_F20},
    {"F21",         KEY_F21},
    {"F22",         KEY_F22},
    {"F23",         KEY_F23},
    {"F24",         KEY_F24},
    {"MEDIA",       KEY_MEDIA}
};

    
void kbd_print_keys(void) {
    unsigned i;
    size_t num_keys = sizeof(kbd_list) / sizeof(struct kbd_keyval);
    printf("%lu keys available for mapping:\n", num_keys);

    for(i = 0; i < num_keys - 1; i++) {
        printf("%s, ", kbd_list[i].name);
    }
    printf("%s.\n", kbd_list[num_keys - 1].name);
}

int kbd_get_code(char *name) {
    unsigned i;
    size_t num_keys = sizeof(kbd_list) / sizeof(struct kbd_keyval);
    for(i = 0; i < num_keys; i++) {
        //printf("kbd_get_code: %d: source_name = \"%s\"\n", i, source_name);
        if(strcasecmp(kbd_list[i].name, name) == 0) {
            printf("FOUND! %s maps to %d (index %d)\n", name, kbd_list[i].code, i);
            return(kbd_list[i].code);
        }
    }
    return(-1);
}

/*
   Parse the options provided from the commandline
   Input example: "a1:CTRL,b1:SPACE,up:UP"
   (token:value,token:value,[..])

   @param char *options      Commandline option pointer
   @returns long 0 on success, nonzero on serious failure
*/
int kbdopt_parse(char *opts, pad_t *pads, size_t num_pads) {

    char *opts_cpy = strndup(opts, 8192);
    if(opts_cpy == NULL) {
        perror("kbdopt_parse: strndup: ");
        abort();
    }
    int failed = 0;
    char *token = NULL;
    printf("kbdopt_parse opts: %s\n", opts_cpy);

    token = strtok(opts_cpy, ":");

    char *button_tokens[4][8] = {
        {"a1", "b1", "start1", "select1", "up1", "down1", "left1", "right1"},
        {"a2", "b2", "start2", "select2", "up2", "down2", "left2", "right2"},
        {"a3", "b3", "start3", "select3", "up3", "down3", "left3", "right3"},
        {"a4", "b4", "start4", "select4", "up4", "down4", "left4", "right4"},
    };

    int i;

    for(;;)
    {
        //printf("BEGIN LOOP\n");
        if(token == NULL)
        {
            break;
        }

        printf("kbdopt_parse: token: \"%s\"\n", token);

        for(unsigned player = 0; player < num_pads; player++) { 
            pad_t *pad = &pads[player];

            for(i = 0; i < 8; i++) {
                if(strcasecmp(token, button_tokens[player][i]) == 0) {
//                    printf("Case match suckaaaaa\n");
                    token = strtok(NULL, ",");
                    if(token) {
//                        printf("Token!\n");
                        int keycode = kbd_get_code(token);
                        if(keycode == -1) {
                            fprintf(stderr, "%s: invalid token\n", token);
                            failed = 1;
                            continue;
                        }
                        pad->kbdsym[i] = keycode;

//                        printf("Mapping %s (player %u) to %u\n", 
//                               button_tokens[player][i],
//                               player,
//                               keycode);
                    }
                }
            }
        }

#if 0
        if(strcmp(token, "a1") == 0)
        {
            token = strtok(NULL, ",");
            if(token)
            {
                keycode = kbd_get_code(token);
            }
        }
        else if(strcmp(token, "ipd_id") == 0)
        {
            token = strtok(NULL, ",");
            if(token)
            {
                errno = 0;
                int value = strtol(token, NULL, 10 /* base */);
                if(errno != 0)
                {
                    perror("strtol in automap_parse_args");
                    failed = errno;
                    errno = 0;
                    break;
                }
                automap->ipd_id = value;
            }
        }
        else if(strcmp(token, "threshold_volts") == 0)
        {
            token = strtok(NULL, ",");
            if(token)
            {
                errno = 0;
                double value = strtod(token, NULL);
                if(errno != 0)
                {
                    perror("strtol in automap_parse_args");
                    failed = errno;
                    errno = 0;
                    break;
                }
                automap->ipd_threshold = value;
            }
        }
#endif

        token = strtok(NULL, ":");
    }

    free(opts_cpy);

    return(failed);
}

