#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <wchar.h>
#include <wctype.h>
#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <math.h>
#include <wctype.h>

#include "ngram_stats.h"
#define BUFFER_SIZE 5
#define KEYBOARD_DEV "/dev/input/event4"

typedef struct {
    int keycode;
    wchar_t lower;
    wchar_t upper;
} KeyMap;

KeyMap key_layout[] = {
    {KEY_Q, L'q', L'Q'}, {KEY_W, L'w', L'W'}, 
    {KEY_E, L'e', L'E'}, {KEY_R, L'r', L'R'},
    {KEY_T, L't', L'T'}, {KEY_Y, L'y', L'Y'},
    {KEY_U, L'u', L'U'}, {KEY_I, L'i', L'I'},
    {KEY_O, L'o', L'O'}, {KEY_P, L'p', L'P'},
    {KEY_A, L'a', L'A'}, {KEY_S, L's', L'S'},
    {KEY_D, L'd', L'D'}, {KEY_F, L'f', L'F'},
    {KEY_G, L'g', L'G'}, {KEY_H, L'h', L'H'},
    {KEY_J, L'j', L'J'}, {KEY_K, L'k', L'K'},
    {KEY_L, L'l', L'L'}, {KEY_Z, L'z', L'Z'},
    {KEY_X, L'x', L'X'}, {KEY_C, L'c', L'C'},
    {KEY_V, L'v', L'V'}, {KEY_B, L'b', L'B'},
    {KEY_N, L'n', L'N'}, {KEY_M, L'm', L'M'},
    {0, L'\0', L'\0'}
};

typedef struct {
    wchar_t buffer[BUFFER_SIZE + 1];
    int length;
    double total_prob;
} InputContext;

InputContext ctx = {0};
int shift_pressed = 0;

void reset_context() {
    ctx.length = 0;
    ctx.total_prob = 0.0;
    wmemset(ctx.buffer, 0, BUFFER_SIZE + 1);
}

double get_ngram_probability(const wchar_t* sequence, int length) {
    if (length < 1 || length > 5) return 0.0;

    // Нормализация последовательности
    wchar_t normalized[5];
    for (int i = 0; i < length; i++) {
        normalized[i] = towupper(sequence[i]);
    }
    normalized[length] = L'\0';

    // Поиск в статистике
    for (size_t i = 0; i < sizeof(ngram_stats)/sizeof(NGramStat); i++) {
        if (ngram_stats[i].type == length && 
            wcsncmp(ngram_stats[i].character, normalized, length) == 0) {
            return ngram_stats[i].percentage;
        }
    }
    return 0.0;
}

void process_keystroke(int code) {
    if (code == KEY_LEFTSHIFT || code == KEY_RIGHTSHIFT) {
        shift_pressed = 1;
        return;
    }

    wchar_t current_char = L'\0';
    for (KeyMap *p = key_layout; p->keycode; p++) {
        if (p->keycode == code) {
            current_char = shift_pressed ? p->upper : p->lower;
            break;
        }
    }

    if (current_char && ctx.length < BUFFER_SIZE) {
        ctx.buffer[ctx.length++] = current_char;
        ctx.buffer[ctx.length] = L'\0';

        // Рассчитываем вероятность для всей текущей последовательности
        ctx.total_prob = 0.0;
        for (int len = 1; len <= ctx.length; len++) {
            ctx.total_prob += get_ngram_probability(ctx.buffer, len);
        }
    }
}

void print_results() {
    if (ctx.length == 0) return;

    double avg = ctx.total_prob / ctx.length;
    wprintf(L"\nСлово: %ls", ctx.buffer);
    wprintf(L"\nСредняя вероятность: %.4f", avg);

    if (avg < 0.01) {
        wprintf(L" \n[!] Возможна ошибка раскладки!");
    }
    wprintf(L"\n");
}

int main() {
    setlocale(LC_ALL, "ru_RU.UTF-8");
    int fd = open(KEYBOARD_DEV, O_RDONLY);
    
    if (fd == -1) {
        perror("Ошибка открытия устройства");
        return 1;
    }

    struct input_event ev;
    while (1) {
        read(fd, &ev, sizeof(ev));
        
        if (ev.type == EV_KEY) {
            if (ev.value == 0) { // Отпускание клавиши
                if (ev.code == KEY_LEFTSHIFT || ev.code == KEY_RIGHTSHIFT) {
                    shift_pressed = 0;
                }
                continue;
            }

            switch (ev.code) {
                case KEY_SPACE:
                case KEY_ENTER:
                    print_results();
                    reset_context();
                    break;
                case KEY_LEFTSHIFT:
                case KEY_RIGHTSHIFT:
                    break;
                default:
                    process_keystroke(ev.code);
                    break;
            }
        }
    }

    close(fd);
    return 0;
}