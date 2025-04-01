# enru_change

Автоматическая смена раскладки,если вы вводите текст и забыли сменить русский на английский или наоборот.
Первым делом настроем X11, а точнее обработку клавиатуры:


README.md
Addn N-gramm generate for russian words
6 hours ago
enru_change.c
Add files via upload
2 minutes ago
genen3.py
Add files via upload
3 minutes ago
genru3.py
Add files via upload
3 minutes ago
ngram_stats_en.h.xz
Add files via upload
3 minutes ago
ngram_stats_ru.h.xz

# Версия 0.2 (puntoswitch, я уже рядом)
 1) https://norvig.com/mayzner.html скачивает N-грамм.
 2) Так как для русского языка я не нашел N-граммы, то будем обучать сами. Скачивайте порядка 20-30 книг на русском и на английском языках.
 3) генерируем частоты. У меня настроена программа и скрипт на на первые 5 символов (5-грамм) (Если лень, то с репозитории они лежат. xz -d ngram_stats_en.h.xz - чтобы распаковать)
```
python genru3.py book-ru/* > ngram_stats_ru.h
python genen3.py book-en/* > ngram_stats_en.h
```
 4) Сборка пакета: gcc enru_change.c -o enru_change -lm $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) -lX11 -lXtst -lpthread

Реализовано:
1) смена раскладки en\ru
2) замена введеных неправильных символов
 
 TODO:
 пока замена работает с en на ru, т.к. в моей системе xmodmap -pke | grep Cyrillic не видит. ru_keysym_map[] дополните и будет рбаотать.

# Версия 0.1

```
sudo cat /proc/bus/input/devices | grep -A5 "keyboard"
sudo chmod a+r /dev/input/event*
```
* - заменить на ту,где ваша клавиатура живет.

Сборка:
```
gcc -o enru_change enru_change.c $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) -lX11 -lXtst -lpthread
```
Полная поддержка UTF8 , но пока смена раскладки жестко вшита (ctrl+shift), подедрживается только 2 языка (ru,en). 
 - Алгоритм анализа ситуации: была взята за основу книга Гарри Потера (англ язык) и какой-то порно шедевр (русский язык). Были сгенерированы все возможные комбинации из 2-уз букв. Сгенерированные конбинации удалялась ,если в книгах находились они в начале слов. Оставшееся было помещено в ru_impossible и en_impossible соответственно.
 - Уже реализована функция удаления введенной неверной комбинации, а вот ввод правильной комбинации, пока неверно работает,но в коде есть.


Result:
Первая строка - ъочу ввести эту строку
Вторая строка - как отработала программа
```
✘  odity@viva  ~  rfr ;t z vjue nfr jib,bnmcz yfxfd 'njn ntrcn
✘  odity@viva  ~  как же я мог так b,bnmcz чав этот текст
```

Да, не идеал, но работаем над алгоритмом
