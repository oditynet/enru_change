# enru_change

Автоматическая смена раскладки,если вы вводите текст и забыли сменить русский на английский или наоборот.
Первым делом настроем X11, а точнее обработку клавиатуры:
```
sudo cat /proc/bus/input/devices | grep -A5 "keyboard"
sudo chmod a+r /dev/input/event[...]
```
* - лучше заменить на ту,где ваша клваиатура живет.

Сборка:
```
gcc -o enru_change enru_change.c $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) -lX11 -lXtst -lpthread
```
Полная поддержка UTF8 , но пока смена раскладки жестко вшита (ctrl+shift), подедрживается только 2 языка (ru,en). 
 - Алгоритм анализа ситуации правильно,когда нужно сменить еще прост,но пишется.
 - Уже реалихована функция после смены языка замена той ерунды,что вы ввели автоматически (Пока только удаляет, но вставить не может)
