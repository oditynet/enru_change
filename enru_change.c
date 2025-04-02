#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <float.h>
#include <string.h>
//#include "/usr/include/X11/keysymdef.h"
#include <stdbool.h>


#include "ngram_stats_ru.h"
#include "ngram_stats_en.h"

#define BUFFER_SIZE 4
#define KEYBOARD_DEV "/dev/input/event4"
#define MAX_NGRAM_LENGTH 4


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
//#include <stdlib.h> // Для mblen()
//#include <alsa/asoundlib.h>
//#include <math.h>

typedef struct {
    int keycode;
    wchar_t lower_en;
    wchar_t upper_en;
    wchar_t lower_ru;
    wchar_t upper_ru;
} KeyMap;

KeyMap key_layout[] = {
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
    {KEY_SEMICOLON,  L';', L':', L'ж', L'Ж'},    // ;\
    {KEY_APOSTROPHE, L'\'', L'\"', L'э', L'Э'}, 
    {KEY_LEFTBRACE,  L'[', L'{', L'х', L'Х'},    // [
    {KEY_BACKSLASH,  L'\\', L'|', L'ъ', L'Ъ'},  // \
    {KEY_GRAVE,      L'`', L'~', L'ё', L'Ё'},    // ~
    {KEY_PERIOD,     L'.', L'>', L'ю', L'Ю'},    // .
    {KEY_COMMA,      L',', L'<', L'б', L'Б'},    // ,
    
    {0}  // Терминатор
};
static struct {
    wchar_t wc;
    KeySym keysym;
} ru_keysym_map[] = {
    {L'й', XK_Cyrillic_shorti}, {L'Й', XK_Cyrillic_SHORTI},
    {L'ц', XK_Cyrillic_tse}, {L'Ц',XK_Cyrillic_TSE},
    {L'у', XK_Cyrillic_u}, {L'У',XK_Cyrillic_U},
    {L'к', XK_Cyrillic_ka}, {L'К',XK_Cyrillic_KA},
    {L'е', XK_Cyrillic_ie}, {L'Е',XK_Cyrillic_IE},
    {L'н', XK_Cyrillic_en}, {L'Н',XK_Cyrillic_EN},
    {L'г', XK_Cyrillic_ghe}, {L'Г',XK_Cyrillic_GHE},
    {L'ш', XK_Cyrillic_sha}, {L'Ш',XK_Cyrillic_SHA},
    {L'щ', XK_Cyrillic_shcha}, {L'Щ',XK_Cyrillic_SHCHA},
    {L'з', XK_Cyrillic_ze}, {L'З',XK_Cyrillic_ZE},
    {L'х', XK_Cyrillic_ha}, {L'Х',XK_Cyrillic_HA},
    {L'ъ', XK_Cyrillic_hardsign}, {L'Ъ',XK_Cyrillic_HARDSIGN},
    {L'ф', XK_Cyrillic_ef}, {L'Ф',XK_Cyrillic_EF},
    {L'ы', XK_Cyrillic_yeru}, {L'Ы',XK_Cyrillic_yeru},
    {L'в', XK_Cyrillic_ve}, {L'В',XK_Cyrillic_VE},
    {L'а', XK_Cyrillic_a}, {L'А',XK_Cyrillic_A},
    {L'п', XK_Cyrillic_pe}, {L'П',XK_Cyrillic_PE},
    {L'р', XK_Cyrillic_er}, {L'Р',XK_Cyrillic_ER},
    {L'о', XK_Cyrillic_o}, {L'О',XK_Cyrillic_O},
    {L'л', XK_Cyrillic_el}, {L'Л',XK_Cyrillic_EL},
    {L'д', XK_Cyrillic_de}, {L'Д',XK_Cyrillic_DE},
    {L'ж', XK_Cyrillic_zhe}, {L'Ж',XK_Cyrillic_ZHE},
    {L'э', XK_Cyrillic_e}, {L'Э',XK_Cyrillic_E},
    {L'й', XK_Cyrillic_io}, {L'Й',XK_Cyrillic_IO},
    {L'я', XK_Cyrillic_ya}, {L'Я',XK_Cyrillic_YA},
    {L'ч', XK_Cyrillic_che}, {L'Ч',XK_Cyrillic_CHE},
    {L'с', XK_Cyrillic_es}, {L'С',XK_Cyrillic_ES},
    {L'м', XK_Cyrillic_em}, {L'М',XK_Cyrillic_EM},
    {L'и', XK_Cyrillic_i}, {L'И',XK_Cyrillic_I},
    {L'т', XK_Cyrillic_te}, {L'Т',XK_Cyrillic_TE},
    {L'ь', XK_Cyrillic_softsign}, {L'Ь',XK_Cyrillic_SOFTSIGN},
    {L'б', XK_Cyrillic_be}, {L'Б',XK_Cyrillic_BE},
    {L'ю', XK_Cyrillic_yu}, {L'Ю',XK_Cyrillic_YU},
    {L'q', XK_Cyrillic_shorti}, {L'Q', XK_Cyrillic_SHORTI},
    {L'W', XK_Cyrillic_tse}, {L'W',XK_Cyrillic_TSE},
    {L'e', XK_Cyrillic_u}, {L'E',XK_Cyrillic_U},
    {L'r', XK_Cyrillic_ka}, {L'R',XK_Cyrillic_KA},
    {L't', XK_Cyrillic_ie}, {L'T',XK_Cyrillic_IE},
    {L'y', XK_Cyrillic_en}, {L'Y',XK_Cyrillic_EN},
    {L'u', XK_Cyrillic_ghe}, {L'U',XK_Cyrillic_GHE},
    {L'i', XK_Cyrillic_sha}, {L'I',XK_Cyrillic_SHA},
    {L'o', XK_Cyrillic_shcha}, {L'O',XK_Cyrillic_SHCHA},
    {L'p', XK_Cyrillic_ze}, {L'P',XK_Cyrillic_ZE},
    {L'a', XK_Cyrillic_ef}, {L'A',XK_Cyrillic_EF},
    {L's', XK_Cyrillic_yeru}, {L'S',XK_Cyrillic_yeru},
    {L'd', XK_Cyrillic_ve}, {L'D',XK_Cyrillic_VE},
    {L'f', XK_Cyrillic_a}, {L'F',XK_Cyrillic_A},
    {L'g', XK_Cyrillic_pe}, {L'G',XK_Cyrillic_PE},
    {L'h', XK_Cyrillic_er}, {L'H',XK_Cyrillic_ER},
    {L'j', XK_Cyrillic_o}, {L'J',XK_Cyrillic_O},
    {L'k', XK_Cyrillic_el}, {L'K',XK_Cyrillic_EL},
    {L'l', XK_Cyrillic_de}, {L'L',XK_Cyrillic_DE},
    {L'z', XK_Cyrillic_ya}, {L'Z',XK_Cyrillic_YA},
    {L'x', XK_Cyrillic_che}, {L'X',XK_Cyrillic_CHE},
    {L'c', XK_Cyrillic_es}, {L'C',XK_Cyrillic_ES},
    {L'v', XK_Cyrillic_em}, {L'V',XK_Cyrillic_EM},
    {L'b', XK_Cyrillic_i}, {L'B',XK_Cyrillic_I},
    {L'n', XK_Cyrillic_te}, {L'N',XK_Cyrillic_TE},
    {L'm', XK_Cyrillic_softsign}, {L'M',XK_Cyrillic_SOFTSIGN},
    {0, 0} // Терминатор
};

typedef struct {
    wchar_t en_buffer[BUFFER_SIZE + 1];
    wchar_t ru_buffer[BUFFER_SIZE + 1];
    int length;
    double current_prob_en;
    double current_prob_ru;
} InputContext;

InputContext ctx = {{0}, {0}, 0, 0.0, 0.0};
int shift_pressed = 0;
int ctrl_pressed = 0;
double min_prob_en = 0.0001;
double min_prob_ru = 0.0001;
static Display *xdisplay = NULL;

static char current_layout[3] = "us";

void init_min_probs() {
    double min_en = DBL_MAX;
    for (size_t i = 0; i < sizeof(ngram_stats_en)/sizeof(NGramStat_en); i++) {
        if (ngram_stats_en[i].percentage > 0 && ngram_stats_en[i].percentage < min_en) {
            min_en = ngram_stats_en[i].percentage;
        }
    }
    if (min_en != DBL_MAX) min_prob_en = min_en;

    double min_ru = DBL_MAX;
    for (size_t i = 0; i < sizeof(ngram_stats_ru)/sizeof(NGramStat_ru); i++) {
        if (ngram_stats_ru[i].percentage > 0 && ngram_stats_ru[i].percentage < min_ru) {
            min_ru = ngram_stats_ru[i].percentage;
        }
    }
    if (min_ru != DBL_MAX) min_prob_ru = min_ru;
}

double get_ngram_probability(const wchar_t* word, int is_russian) {
    int len = wcslen(word);
    if (len == 0) return 0.0;

    int check_len = len > MAX_NGRAM_LENGTH ? MAX_NGRAM_LENGTH : len;
    wchar_t target[MAX_NGRAM_LENGTH + 1] = {0};
    wcsncpy(target, word + (len - check_len), check_len);
    
    // Нормализация с поддержкой русского
    for (int i = 0; i < check_len; i++) {
        target[i] = towupper(target[i]);
        if (is_russian && target[i] >= L'а' && target[i] <= L'я') {
            target[i] -= 32; // Ручное преобразование для кириллицы
        }
    }

    if (is_russian) {
        for (size_t i = 0; i < sizeof(ngram_stats_ru)/sizeof(NGramStat_ru); i++) {
            if (ngram_stats_ru[i].type == check_len && 
                wcscmp(ngram_stats_ru[i].character, target) == 0) {
                return ngram_stats_ru[i].percentage;
            }
        }
        return min_prob_ru;
    } else {
        for (size_t i = 0; i < sizeof(ngram_stats_en)/sizeof(NGramStat_en); i++) {
            if (ngram_stats_en[i].type == check_len && 
                wcscmp(ngram_stats_en[i].character, target) == 0) {
                return ngram_stats_en[i].percentage;
            }
        }
        return min_prob_en;
    }
}

void process_keystroke(int code) {
    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
        shift_pressed = 1;
        return;
    }

    wchar_t en_char = L'\0';
    wchar_t ru_char = L'\0';
    
    for (KeyMap *p = key_layout; p->keycode; p++) {
        if (p->keycode == code) {
            wprintf(L"[F+]: %d\n", code);
            en_char = shift_pressed ? p->upper_en : p->lower_en;
            ru_char = shift_pressed ? p->upper_ru : p->lower_ru;
            break;
        }
    }

    if (en_char == L'\0' && ru_char == L'\0') return;

    // Обновление буфера
    if (ctx.length < BUFFER_SIZE) {
        ctx.en_buffer[ctx.length] = en_char;
        ctx.ru_buffer[ctx.length] = ru_char;
        ctx.length++;
    } else {
        for (int i = 0; i < BUFFER_SIZE-1; i++) {
            ctx.en_buffer[i] = ctx.en_buffer[i+1];
            ctx.ru_buffer[i] = ctx.ru_buffer[i+1];
        }
        ctx.en_buffer[BUFFER_SIZE-1] = en_char;
        ctx.ru_buffer[BUFFER_SIZE-1] = ru_char;
    }
    
    ctx.en_buffer[ctx.length] = L'\0';
    ctx.ru_buffer[ctx.length] = L'\0';

    ctx.current_prob_en = get_ngram_probability(ctx.en_buffer, 0);
    ctx.current_prob_ru = get_ngram_probability(ctx.ru_buffer, 1);
}
void change_layout(const char *lang) {
    char cmd[64];
    snprintf(cmd, sizeof(cmd), "setxkbmap %s", lang);
    system(cmd);
    strcpy(current_layout, lang);
}

void send_backspaces(int count) {
    KeyCode backspace = XKeysymToKeycode(xdisplay, XK_BackSpace);
    for(int i = 0; i <= count; i++) {
//        printf("del\n");
        XTestFakeKeyEvent(xdisplay, backspace, True, CurrentTime);
        XTestFakeKeyEvent(xdisplay, backspace, False, CurrentTime);
    //    XFlush(xdisplay);
        usleep(50000);
    }
}

bool is_en(wchar_t ch)
{
    return (ch >= L'A' && ch <= L'Z') || (ch >= L'a' && ch <= L'z');
}

void send_string(const wchar_t *str) {
    //char mbstr[BUFFER_SIZE * 4];
   // wcstombs(mbstr, str, sizeof(mbstr));

    KeyCode shift_code = XKeysymToKeycode(xdisplay, XK_Shift_L);
   // XKeyEvent event = {0};

    for(const wchar_t *p = str; *p; p++) {
	KeySym sym = 0;
	for(int i = 0; ru_keysym_map[i].wc; i++) {
	    if(ru_keysym_map[i].wc == *p) {
		sym = ru_keysym_map[i].keysym;
		break;
	    }
	}

    KeyCode code;
    if (0 != is_en(*p)){
       char wc[6];
       int l=wctomb(wc,*p);
       code = XKeysymToKeycode(xdisplay, XStringToKeysym(wc));
    }
    else
       code = XKeysymToKeycode(xdisplay, sym);
	//KeyCode code = XKeysymToKeycode(xdisplay, sym);
	if(code == 0) {
	    fprintf(stderr, "Не найден KeyCode для символа: %lc\n", *p);
	    continue;
	}

	// Эмуляция нажатия
//	if(need_shift) {
//	    XTestFakeKeyEvent(xdisplay, shift_code, True, CurrentTime);
//	}

	XTestFakeKeyEvent(xdisplay, code, True, CurrentTime);
	XTestFakeKeyEvent(xdisplay, code, False, CurrentTime);

	//if(need_shift) {
	//    XTestFakeKeyEvent(xdisplay, shift_code, False, CurrentTime);
	//}

	//XFlush(xdisplay);
	usleep(50000); // Задержка для стабильности
    }
    XFlush(xdisplay);
}

void print_results() {
    wprintf(L"Текущий буфер EN: %ls \n", ctx.en_buffer);
    wprintf(L"Текущий буфер RU: %ls \n", ctx.ru_buffer);
    wprintf(L"Вероятность EN: %.4f%% \n", ctx.current_prob_en);
    wprintf(L"Вероятность RU: %.4f%% \n", ctx.current_prob_ru);
    
    const double threshold = 0.002;
    if (fabs(ctx.current_prob_en - ctx.current_prob_ru) < threshold) {
        wprintf(L"Результат: неопределённый\n");
    } else if (ctx.current_prob_en > ctx.current_prob_ru) {
        wprintf(L"Результат: английский\n");
        if (strcmp(current_layout, "us") == 0)
    	    return;
        change_layout("us");
        send_backspaces(wcslen(ctx.ru_buffer));
        send_string(ctx.en_buffer);
    } else {
        wprintf(L"Результат: русский\n");
        if (strcmp(current_layout, "ru") == 0)
    	    return;
        
        change_layout("ru");
        send_backspaces(wcslen(ctx.en_buffer));
        send_string(ctx.ru_buffer);
    }
    wprintf(L"\n");
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

void* loops() {
    init_min_probs();

    int fd = open(KEYBOARD_DEV, O_RDONLY);
    if (fd == -1) {
        perror("Ошибка открытия устройства");
        exit(1);
    }
    

    int can=0;
    struct input_event ev;
    while (1) {
        read(fd, &ev, sizeof(ev));
        
        if (ev.type == EV_KEY) {
            if (ev.value == 0) { // Release
                if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                    shift_pressed = 0;
                }
            } else if (ev.value == 1) { // Press
                wprintf(L"[!]Press: %d\n", ev.code);
                switch (ev.code) {
                    case KEY_BACKSPACE:
                	can--;
                	ctx.length--;
                	break;
                    case KEY_SPACE:
                    case KEY_ENTER:
                        if (can <= MAX_NGRAM_LENGTH)
                            print_results();
                        memset(&ctx, 0, sizeof(InputContext));
                        can=0;
                        break;
                    case KEY_LEFTSHIFT:
                        shift_pressed = 1;
                        break;
                    case KEY_LEFTCTRL:
                        ctrl_pressed = 1;
                        break;
                    case KEY_RIGHTSHIFT:
                        break;
                    default:
                	if (can < MAX_NGRAM_LENGTH)
                	{
                	    //wprintf(L"Press: %d %d", can, ev.code);
                    	    process_keystroke(ev.code);
                    	}
                    	else if (can == MAX_NGRAM_LENGTH)
                    	{
                    	    print_results();
                    	}
                	can++;
                	break;
                }
                if(ctrl_pressed && shift_pressed) {
            	    if(strcmp(current_layout, "ru") == 0)
            		change_layout("us");
            	    else
            	        change_layout("ru");
                    shift_pressed = 0;
                    ctrl_pressed = 0;
                    printf("change enru\n");
                }
            }
        }
    }
    close(fd);
}
int main(int argc, char **argv) {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    xdisplay = XOpenDisplay(NULL);
    if(!xdisplay) {
        fprintf(stderr, "Error opening X display\n");
        return 1;
    }
    
    gtk_init(&argc, &argv);
    /*AppIndicator *indicator = app_indicator_new(
        "smart-kb", "input-keyboard",
        APP_INDICATOR_CATEGORY_APPLICATION_STATUS
    );

    app_indicator_set_menu(indicator, create_tray_menu(indicator));
    app_indicator_set_status(indicator, APP_INDICATOR_STATUS_ACTIVE);
    */
    pthread_t thread;
    pthread_create(&thread, NULL, loops,NULL);

    gtk_main();
    XCloseDisplay(xdisplay);
    
    return 0;
}
