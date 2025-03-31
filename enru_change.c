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
#include <alsa/asoundlib.h>
#include <math.h>
//#include <X11/keysymdef.h>

#define KEYBOARD_DEV "/dev/input/event3"
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
    {0, 0} // Терминатор
};


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
//    {KEY_SPACE,L' ', L' ', L' ', L' '},
    {0}
};

const char *ru_impossible[] = {
    "аа","ае","аё","аз","аи","ам","ао","ау","ац","ач","аш","ащ","аъ","аы","аь","аю","ая","бб","бв","бг","бд","бж","бз","бй","бк","бм","бн","бп","бс","бт","бф","бх","бц","бч","бш","бщ","бъ","бэ","бю","бя","вй","ву","вф","вш","вщ","вь","вю","гб","гв","гг","гё","гж","гз","гй","гк","гм","гп","гс","гт","гф","гх","гц","гч","гш","гщ","гъ","гы","гь","гэ","гю","гя","дб","дг","дд","дз","дй","дк","дм","дп","дс","дт","дф","дх","дц","дч","дш","дщ","дъ","дь","дэ","дю","еа","еб","ее","еи","ек","ен","ео","еп","ет","еу","еф","ец","еч","еш","еъ","еы","еь","еэ","ею","ея","ёа","ёв","ёг","ёд","ёе","ёё","ёж","ёз","ёи","ёй","ёк","ёл","ём","ён","ёо","ёп","ёр","ёс","ёт","ёу","ёф","ёх","ёц","ёч","ёш","ёщ","ёъ","ёы","ёь","ёэ","ёю","ёя","жб","жв","жж","жз","жй","жл","жн","жп","жр","жс","жт","жф","жх","жц","жч","жш","жщ","жъ","жы","жь","жэ","жю","жя","зб","зг","зё","зж","зз","зи","зй","зк","зп","зс","зт","зф","зх","зц","зч","зш","зщ","зъ","зь","зэ","зю","зя","иа","ие","иё","иж","ии","ий","ио","ип","иу","иф","иц","ич","иш","иъ","иы","иь","иэ","ию","ия","йа","йб","йв","йг","йд","йё","йж","йз","йи","йй","йк","йл","йм","йн","йо","йп","йр","йс","йт","йу","йф","йх","йц","йч","йш","йщ","йъ","йы","йь","йэ","йю","йя","кб","кг","кд","кё","кж","кз","кй","кк","км","кп","кф","кц","кч","кш","кщ","къ","кы","кь","кэ","кю","кя","лв","лг","лд","лз","лй","лк","лл","лм","лн","лп","лр","лс","лт","лф","лх","лц","лч","лш","лщ","лъ","лы","ль","лэ","мб","мв","мд","мж","мз","мй","мк","мп","мр","мс","мт","мф","мх","мц","мч","мш","мщ","мъ","мь","мю","нб","нв","нг","нд","нж","нз","нй","нк","нл","нм","нн","нп","нс","нт","нф","нх","нц","нч","нш","нщ","нъ","нь","нэ","оа","ое","оё","ои","ой","оу","оц","ош","оъ","оы","оь","оэ","ою","оя","пб","пв","пг","пд","пё","пж","пз","пй","пк","пм","пн","пп","пс","пт","пф","пх","пц","пш","пщ","пъ","пю","рб","рг","рд","рё","рз","рй","рк","рл","рм","рн","рп","рс","рф","рх","рц","рч","рш","рщ","ръ","рь","сё","сй","сф","сш","сщ","сь","сэ","тб","тг","тд","тж","тз","тй","тл","тм","тн","тп","тт","тф","тх","тц","тч","тш","тщ","тъ","ть","тэ","тю","уа","уё","уи","уо","уу","уц","ущ","уъ","уы","уь","уэ","ую","уя","фб","фв","фг","фд","фё","фж","фз","фй","фк","фм","фн","фп","фс","фт","фф","фх","фц","фч","фш","фщ","фъ","фю","фя","хб","хг","хд","хё","хж","хз","хй","хк","хн","хп","хс","хт","хф","хх","хц","хч","хш","хщ","хъ","хы","хь","хэ","хю","хя","цб","цг","цд","цё","цж","цз","ци","цй","цк","цл","цм","цн","цп","цр","цс","цт","цу","цф","цх","цц","цч","цш","цщ","цъ","ць","цэ","цю","ця","чб","чв","чг","чд","чж","чз","чй","чк","чн","чп","чс","чф","чх","чц","чч","чш","чщ","чъ","чы","чэ","чю","чя","шб","шв","шг","шд","шж","шз","шй","шн","шр","шс","шф","шх","шц","шч","шш","шщ","шъ","шы","шь","шэ","шю","шя","ща","щб","щв","щг","щд","щж","щз","щи","щй","щк","щл","щм","щн","що","щп","щр","щс","щт","щф","щх","щц","щч","щш","щщ","щъ","щы","щь","щэ","щю","щя","ъа","ъб","ъв","ъг","ъд","ъе","ъё","ъж","ъз","ъи","ъй","ък","ъл","ъм","ън","ъо","ъп","ър","ъс","ът","ъу","ъф","ъх","ъц","ъч","ъш","ъщ","ъъ","ъы","ъь","ъэ","ъю","ъя","ыа","ыб","ыв","ыг","ыд","ые","ыё","ыж","ыз","ыи","ый","ык","ыл","ым","ын","ыо","ып","ыр","ыс","ыт","ыу","ыф","ых","ыц","ыч","ыш","ыщ","ыъ","ыы","ыь","ыэ","ыю","ыя","ьа","ьб","ьв","ьг","ьд","ье","ьё","ьж","ьз","ьи","ьй","ьк","ьл","ьм","ьн","ьо","ьп","ьр","ьс","ьт","ьу","ьф","ьх","ьц","ьч","ьш","ьщ","ьъ","ьы","ьь","ьэ","ью","ья","эа","эб","эв","эг","эд","эе","эё","эж","эз","эи","эо","эр","эу","эф","эц","эч","эш","эщ","эъ","эы","эь","ээ","эю","эя","юа","юб","юв","юд","юе","юё","юз","юи","юй","юк","юл","юм","юо","юп","юс","ют","юу","юф","юх","юц","юч","юш","ющ","юъ","юы","юь","юэ","юю","юя","яа","яд","яе","яё","яж","яи","яй","як","ял","яп","ят","яу","яф","ях","яц","яч","яш","ящ","яъ","яы","яь","яэ","яю","яя", NULL
};

const char *en_impossible[] = {
    "ae","ak","ao","aq","ax","ay","az","bb","bc","bd","bf","bg","bh","bj","bk","bm","bn","bp","bq","bs","bt","bv","bw","bx","bz","cb","cc","cd","cf","cg","cj","ck","cm","cn","cp","cq","cs","ct","cv","cw","cx","cy","cz","db","dc","dd","df","dg","dh","dj","dk","dl","dm","dn","dp","dq","ds","dt","dv","dx","dz","ej","ek","eo","ep","et","eu","ew","ez","fb","fc","fd","ff","fg","fh","fj","fk","fm","fn","fp","fq","fs","ft","fv","fw","fx","fy","fz","gb","gc","gd","gf","gg","gj","gh","gk","gm","gn","gp","gq","gs","gt","gv","gw","gx","gz","hb","hc","hd","hf","hg","hh","hj","hk","hl","hn","hp","hq","hr","hs","ht","hv","hw","hx","hz","ia","ib","ie","ih","ii","ij","ik","io","ip","iq","iu","iw","ix","iy","iz","jb","jc","jd","jf","jg","jh","jj","jk","jl","jm","jn","jp","jq","jr","js","jt","jv","jw","jx","jy","jz","kb","kc","kd","kf","kg","kh","kj","kk","kl","km","ko","kp","kq","kr","ks","kt","ku","kv","kw","kx","ky","kz","lb","lc","ld","lf","lg","lh","lj","lk","ll","lm","ln","lp","lq","lr","ls","lt","lv","lw","lx","lz","mb","md","mf","mg","mh","mj","mk","ml","mn","mp","mq","ms","mt","mv","mw","mx","mz","nb","nc","nd","nf","ng","nh","nj","nk","nl","nn","np","nq","nr","ns","nt","nv","nw","nx","ny","nz","oe","og","oj","oq","os","ox","oz","pb","pc","pd","pf","pg","pj","pk","pm","pn","pp","pq","ps","pv","pw","px","pz","qa","qb","qc","qd","qe","qf","qg","qh","qi","qj","qk","ql","qm","qn","qo","qp","qq","qr","qs","qt","qv","qw","qx","qy","qz","rb","rc","rd","rf","rg","rh","rj","rk","rl","rm","rn","rp","rq","rr","rs","rt","rv","rw","rx","ry","rz","sb","sd","sf","sg","sj","sr","ss","sv","sx","sz","tb","tc","td","tf","tg","tj","tk","tl","tm","tn","tp","tq","ts","tt","tv","tx","tz","ua","uc","ud","ue","uf","uh","ui","uj","uk","ul","uo","uq","uu","uv","uw","ux","uy","uz","vb","vd","vf","vg","vh","vj","vk","vl","vm","vn","vp","vq","vr","vs","vt","vu","vv","vw","vx","vy","vz","wb","wc","wd","wf","wg","wj","wk","wl","wm","wn","wp","wq","ws","wt","wu","wv","ww","wx","wy","wz","xa","xb","xc","xd","xe","xf","xg","xh","xi","xj","xk","xl","xm","xn","xo","xp","xq","xr","xs","xt","xu","xv","xw","xx","xy","xz","yb","yc","yd","yf","yg","yh","yi","yj","yk","yl","ym","yn","yp","yq","yr","ys","yt","yu","yw","yx","yy","yz","zb","zc","zd","ze","zf","zg","zh","zj","zk","zl","zm","zn","zp","zq","zr","zs","zt","zu","zv","zw","zx","zy","zz", NULL
};

void update_indicator(AppIndicator *indicator) {
    const char *icon = strcmp(current_layout, "ru") == 0 ?
        "flag-ru" : "flag-us";
    app_indicator_set_icon(indicator, icon);
}

void send_backspaces(int count) {
    KeyCode backspace = XKeysymToKeycode(xdisplay, XK_BackSpace);
    for(int i = 0; i < count; i++) {
        printf("del1\n");
        XTestFakeKeyEvent(xdisplay, backspace, True, CurrentTime);
        XTestFakeKeyEvent(xdisplay, backspace, False, CurrentTime);
        XFlush(xdisplay);
        usleep(1000);
    }
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


		KeyCode code = XKeysymToKeycode(xdisplay, sym);
		if(code == 0) {
			fprintf(stderr, "Не найден KeyCode для символа: %lc\n", *p);
			continue;
		}

		// Определяем необходимость Shift
		Bool need_shift = iswupper(*p) ||
			(sym >= XK_exclam && sym <= XK_asciitilde);

		// Эмуляция нажатия
		if(need_shift) {
			XTestFakeKeyEvent(xdisplay, shift_code, True, CurrentTime);
		}

		XTestFakeKeyEvent(xdisplay, code, True, CurrentTime);
		XTestFakeKeyEvent(xdisplay, code, False, CurrentTime);

		if(need_shift) {
			XTestFakeKeyEvent(xdisplay, shift_code, False, CurrentTime);
		}

		XFlush(xdisplay);
		usleep(10000); // Задержка для стабильности
    }
}

void change_layout(AppIndicator *indicator, const char *lang) {
    if(strcmp(lang, "ru")) {
        wchar_t fixed[BUFFER_SIZE] = {0};
        for(int i = 0; i < input_state.index; i++) {
         // printf("! %c \n",input_state.buffer[i]);
            for(KeyLayout *p = layout_map; p->keycode; p++) {
                if(input_state.buffer[i] == p->ru_lower ||
                   input_state.buffer[i] == p->ru_upper) {
                    //printf("? %s %s\n",input_state.buffer[i],p->ru_upper);
                    fixed[i] = (input_state.buffer[i] == p->ru_upper) ?
                        p->en_upper : p->en_lower;
                    break;
                }
            }
        }
        send_backspaces(input_state.index);
       // printf("sss %s\n",fixed);
        send_string(fixed);
    }else {
		wchar_t fixed[BUFFER_SIZE] = {0};
		for(int i = 0; i < input_state.index; i++) {
		 // printf("! %c \n",input_state.buffer[i]);
			for(KeyLayout *p = layout_map; p->keycode; p++) {
				if(input_state.buffer[i] == p->en_lower ||
				   input_state.buffer[i] == p->en_upper) {
					//printf("? %s %s\n",input_state.buffer[i],p->en_upper);
					fixed[i] = (input_state.buffer[i] == p->en_upper) ?
						p->ru_upper : p->ru_lower;
					break;
				}
			}
		}
		send_backspaces(input_state.index);
		//printf("sss %s\n",fixed);
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

            //printf("Checking pair: %s\n", pair);

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
   // printf("%d\n",wc);
    if(wc) {
        input_state.buffer[input_state.index++] = wc;
        if(input_state.index >= 2) {analyze_input(indicator);input_state.index = 0;
        wmemset(input_state.buffer, 0, BUFFER_SIZE);}
    }
}

void* keyboard_listener(void *arg) {
    AppIndicator *indicator = (AppIndicator*)arg;
    int fd = open(KEYBOARD_DEV, O_RDONLY);
    struct input_event ev;
    int spice=0,enter=1;

    while(1) {
        if(read(fd, &ev, sizeof(ev)) > 0) {
            if(ev.type == EV_KEY && ev.value == 1) {
                if(spice == 1){
		   enter=1;
		}
                spice=0;
                switch(ev.code) {
                    case KEY_LEFTCTRL:
                    case KEY_RIGHTCTRL:
                        ctrl_pressed = 1;
                        break;
                    case KEY_LEFTSHIFT:
                    case KEY_RIGHTSHIFT:
                        shift_pressed = 1;
                        break;
                    case KEY_SPACE:

                        input_state.index = 0;
                        wmemset(input_state.buffer, 0, BUFFER_SIZE);
                       // printf("Press spice\n");
                        spice=1;
                        enter=0;
                        break;
                    case KEY_ENTER:
                        input_state.index = 0;
                        wmemset(input_state.buffer, 0, BUFFER_SIZE);
                       // printf("Press enter\n");

                        spice=1;
                        enter=0;
                        break;
                    default:
                        if (enter >0 && enter < 3){
                         //  printf("Press - %d\n",ev.code);
                           process_key_event(indicator, ev.code, shift_pressed);
                           enter+=1;
                           if (spice == 1)
                            enter=1;
                        }else { enter=0;}
                }

                if(ctrl_pressed && shift_pressed) {
                    change_layout(indicator,
                        strcmp(current_layout, "ru") ? "ru" : "us");
                    ctrl_pressed = shift_pressed = 0;
                    spice=1;
                    enter=0;
                    input_state.index = 0;
                    wmemset(input_state.buffer, 0, BUFFER_SIZE);
                    printf("change enru\n");
                }
            }
            else if(ev.value == 0) {
                switch(ev.code) {
                    case KEY_LEFTCTRL:
                    case KEY_RIGHTCTRL:
                       // printf("0 ctrl\n");
                       // printf("enter %d; spice %d\n",enter,spice);
                        ctrl_pressed = 0;
                        break;
                    case KEY_LEFTSHIFT:
                    case KEY_RIGHTSHIFT:
                      //  printf("0 shi\n");
                       // printf("enter %d; spice %d\n",enter,spice);
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
        "smart-kb", "input-keyboard",
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

