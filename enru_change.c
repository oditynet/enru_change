//gcc -o enru_change enru_change.c $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) -lX11 -lXtst -lpthread
#include <gtk/gtk.h>
#include <libappindicator/app-indicator.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <pthread.h>
#include <string.h>
#include <wctype.h>
#include <locale.h>
#include <time.h>
#include <X11/Xlib.h>
#include <X11/extensions/XTest.h>
#include <X11/keysym.h>
#include <ctype.h>
#include <wchar.h>
#include <stdlib.h> // Для mblen()

#define KEYBOARD_DEV "/dev/input/event4"
#define BUFFER_SIZE 128
#define THRESHOLD 0.3

typedef struct {
    wchar_t buffer[BUFFER_SIZE];
    int index;
    time_t last_switch;
} InputState;

typedef struct {
    int keycode;
    wchar_t en_lower;
    wchar_t en_upper;
    wchar_t ru_lower;
    wchar_t ru_upper;
} KeyLayout;

static InputState input_state = {0};
static char current_layout[3] = "us";
static int ctrl_pressed = 0;
static int shift_pressed = 0;
static Display *xdisplay = NULL;

KeyLayout layout_map[] = {
    {KEY_Q,    L'q', L'Q', L'й', L'Й'},
    {KEY_W,    L'w', L'W', L'ц', L'Ц'},
    {KEY_E,    L'e', L'E', L'у', L'У'},
    {KEY_R,    L'r', L'R', L'к', L'К'},
    {KEY_T,    L't', L'T', L'е', L'Е'},
    {KEY_Y,    L'y', L'Y', L'н', L'Н'},
    {KEY_U,    L'u', L'U', L'г', L'Г'},
    {KEY_I,    L'i', L'I', L'ш', L'Ш'},
    {KEY_O,    L'o', L'O', L'щ', L'Щ'},
    {KEY_P,    L'p', L'P', L'з', L'З'},
    {KEY_A,    L'a', L'A', L'ф', L'Ф'},
    {KEY_S,    L's', L'S', L'ы', L'Ы'},
    {KEY_D,    L'd', L'D', L'в', L'В'},
    {KEY_F,    L'f', L'F', L'а', L'А'},
    {KEY_G,    L'g', L'G', L'п', L'П'},
    {KEY_H,    L'h', L'H', L'р', L'Р'},
    {KEY_J,    L'j', L'J', L'о', L'О'},
    {KEY_K,    L'k', L'K', L'л', L'Л'},
    {KEY_L,    L'l', L'L', L'д', L'Д'},
    {KEY_Z,    L'z', L'Z', L'я', L'Я'},
    {KEY_X,    L'x', L'X', L'ч', L'Ч'},
    {KEY_C,    L'c', L'C', L'с', L'С'},
    {KEY_V,    L'v', L'V', L'м', L'М'},
    {KEY_B,    L'b', L'B', L'и', L'И'},
    {KEY_N,    L'n', L'N', L'т', L'Т'},
    {KEY_M,    L'm', L'M', L'ь', L'Ь'},
    {KEY_COMMA,L',', L'<', L'б', L'Б'},
    {KEY_DOT,  L'.', L'>', L'ю', L'Ю'},
    {KEY_SLASH,L'/', L'?', L'.', L','},
    {0}
};

const char *ru_impossible[] = {
    "щы", "щэ", "щъ", "щю", "щя", "чы", "чэ", "чъ", 
    "чю", "чя", "шы", "шэ", "шъ", "шю", "шя", "жы", 
    "жэ", "жъ", "жю", "жя", "цщ", "цэ", "цъ", "ць", 
    "цю", "ця", "фы", "фэ", "фъ", "фь", "хы", "хэ", 
    "хъ", "хь", "хю", "хя", "йй", NULL
};

const char *en_impossible[] = {
    "zx", "qj", "qv", "qk", "qy", "qz", "wq", "wv", 
    "wx", "wz", "vq", "vx", "vz", "jq", "jx", "jz", 
    "jk", "jv", "jp", "jb", "jm", "jf", "jg", "jh", 
    "jl", "jt", "jd", "zq", "zj", "zv", "zp", "zb", 
    "zm", "zf", "zg", "zh", "zl", "zt", "zd", "zn", 
    "zs", "zr", "zc", "zx", "zv", "fq", "fx", "fz", 
    "fj", "fk", "qq", NULL
};

void update_indicator(AppIndicator *indicator) {
    const char *icon = strcmp(current_layout, "ru") == 0 ? 
        "flag-ru" : "flag-us";
    app_indicator_set_icon(indicator, icon);
}

void send_backspaces(int count) {
    KeyCode backspace = XKeysymToKeycode(xdisplay, XK_BackSpace);
    for(int i = 0; i < count; i++) {
        XTestFakeKeyEvent(xdisplay, backspace, True, CurrentTime);
        XTestFakeKeyEvent(xdisplay, backspace, False, CurrentTime);
        XFlush(xdisplay);
        usleep(1000);
    }
}

void send_string(const wchar_t *str) {
    char mbstr[BUFFER_SIZE * 4];
    wcstombs(mbstr, str, sizeof(mbstr));
    
    KeyCode shift_code = XKeysymToKeycode(xdisplay, XK_Shift_L);
    XKeyEvent event = {0};
    
    for(char *p = mbstr; *p; p++) {
        KeySym sym = XStringToKeysym(p);
        KeyCode code = XKeysymToKeycode(xdisplay, sym);
        
        if(isupper(*p) || (sym >= XK_exclam && sym <= XK_asciitilde)) {
            XTestFakeKeyEvent(xdisplay, shift_code, True, CurrentTime);
        }
        
        XTestFakeKeyEvent(xdisplay, code, True, CurrentTime);
        XTestFakeKeyEvent(xdisplay, code, False, CurrentTime);
        
        if(shift_pressed) {
            XTestFakeKeyEvent(xdisplay, shift_code, False, CurrentTime);
        }
        
        usleep(1000);
    }
}

void change_layout(AppIndicator *indicator, const char *lang) {
    if(strcmp(lang, current_layout)) {
        wchar_t fixed[BUFFER_SIZE] = {0};
        for(int i = 0; i < input_state.index; i++) {
            for(KeyLayout *p = layout_map; p->keycode; p++) {
                if(input_state.buffer[i] == p->ru_lower || 
                   input_state.buffer[i] == p->ru_upper) {
                    fixed[i] = (input_state.buffer[i] == p->ru_upper) ? 
                        p->en_upper : p->en_lower;
                    break;
                }
            }
        }
        send_backspaces(input_state.index);
        send_string(fixed);
    }

    char cmd[64];
    snprintf(cmd, sizeof(cmd), "setxkbmap %s", lang);
    system(cmd);
    strcpy(current_layout, lang);
    input_state.last_switch = time(NULL);
    input_state.index = 0;
    wmemset(input_state.buffer, 0, BUFFER_SIZE);
    update_indicator(indicator);
}

int is_impossible(const char *seq, const char **dict) {
    for(int i = 0; dict[i]; i++) {
        //printf("Match found: %s == %s\n", seq, dict[i]);
        if(strstr(seq, dict[i])) return 1;
    }
    return 0;
}

void analyze_input(AppIndicator *indicator) {
    if(time(NULL) - input_state.last_switch < 2) return;

    char normalized[BUFFER_SIZE * 4] = {0};
    size_t converted = wcstombs(normalized, input_state.buffer, sizeof(normalized));
    
    if(converted == (size_t)-1) {
        fprintf(stderr, "Ошибка конвертации в многобайтовую строку\n");
        return;
    }

    const char **target_dict = strcmp(current_layout, "ru") != 0 
        ? en_impossible 
        : ru_impossible;
    
    int impossible = 0;
    int len = strlen(normalized);
    
    // Правильный обход многобайтовых символов
    for(int i = 0; i < len; ) {
        int char1_len = mblen(&normalized[i], len - i);
        if(char1_len <= 0) break;

        if(i + char1_len < len) {
            int char2_len = mblen(&normalized[i + char1_len], len - (i + char1_len));
            if(char2_len <= 0) break;

            // Формируем пару из двух символов
            char pair[5] = {0};
            memcpy(pair, &normalized[i], char1_len);
            memcpy(pair + char1_len, &normalized[i + char1_len], char2_len);
            
            printf("Checking pair: %s\n", pair);
            
            if(is_impossible(pair, target_dict)) {
                impossible++;
                printf("Невозможная комбинация: %s\n", pair);
            }
        }
        i += char1_len;
    }

    if(impossible > 0) {
        float ratio = impossible / (float)(converted > 1 ? converted - 1 : 1);
        if(ratio > THRESHOLD) {
            change_layout(indicator, 
                strcmp(current_layout, "ru") ? "ru" : "us");
        }
    }
}
void process_key_event(AppIndicator *indicator, int code, int shift) {
    if(input_state.index >= BUFFER_SIZE-1) return;

    wchar_t wc = 0;
    for(KeyLayout *p = layout_map; p->keycode; p++) {
        if(code == p->keycode) {
            if(strcmp(current_layout, "ru") == 0) {
                wc = shift ? p->ru_upper : p->ru_lower;
            } else {
                wc = shift ? p->en_upper : p->en_lower;
            }
            break;
        }
    }
    printf("%d\n",wc);
    if(wc) {
        input_state.buffer[input_state.index++] = wc;
        if(input_state.index >= 2) analyze_input(indicator);
    }
}

void* keyboard_listener(void *arg) {
    AppIndicator *indicator = (AppIndicator*)arg;
    int fd = open(KEYBOARD_DEV, O_RDONLY);
    struct input_event ev;

    while(1) {
        if(read(fd, &ev, sizeof(ev)) > 0) {
            if(ev.type == EV_KEY && ev.value == 1) {
                switch(ev.code) {
                    case KEY_LEFTCTRL:
                    case KEY_RIGHTCTRL:
                        ctrl_pressed = 1;
                        break;
                    case KEY_LEFTSHIFT:
                    case KEY_RIGHTSHIFT:
                        shift_pressed = 1;
                        break;
                    default:
                        printf("Press - %d\n",ev.code);
                        process_key_event(indicator, ev.code, shift_pressed);
                }
                
                if(ctrl_pressed && shift_pressed) {
                    change_layout(indicator, 
                        strcmp(current_layout, "ru") ? "ru" : "us");
                    ctrl_pressed = shift_pressed = 0;
                }
            }
            else if(ev.value == 0) {
                switch(ev.code) {
                    case KEY_LEFTCTRL:
                    case KEY_RIGHTCTRL:
                        ctrl_pressed = 0;
                        break;
                    case KEY_LEFTSHIFT:
                    case KEY_RIGHTSHIFT:
                        shift_pressed = 0;
                        break;
                }
            }
        }
        usleep(1000);
    }
    close(fd);
    return NULL;
}

GtkMenu* create_tray_menu(AppIndicator *indicator) {
    GtkMenu *menu = GTK_MENU(gtk_menu_new());
    GtkWidget *ru = gtk_menu_item_new_with_label("Русский");
    GtkWidget *en = gtk_menu_item_new_with_label("English");
    GtkWidget *quit = gtk_menu_item_new_with_label("Выход");

    g_signal_connect(ru, "activate", 
        G_CALLBACK(change_layout), "ru");
    g_signal_connect(en, "activate", 
        G_CALLBACK(change_layout), "us");
    g_signal_connect(quit, "activate", G_CALLBACK(gtk_main_quit), NULL);

    gtk_menu_shell_append(GTK_MENU_SHELL(menu), ru);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), en);
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), gtk_separator_menu_item_new());
    gtk_menu_shell_append(GTK_MENU_SHELL(menu), quit);

    gtk_widget_show_all(GTK_WIDGET(menu));
    return menu;
}

int main(int argc, char **argv) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    xdisplay = XOpenDisplay(NULL);
    
    gtk_init(&argc, &argv);
    AppIndicator *indicator = app_indicator_new(
        "smart-kb", "flag-us",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS
    );
    
    app_indicator_set_menu(indicator, create_tray_menu(indicator));
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    
    pthread_t thread;
    pthread_create(&thread, NULL, keyboard_listener, indicator);
    
    gtk_main();
    XCloseDisplay(xdisplay);
    return 0;
}
