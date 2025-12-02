# enru_change

Automatic layout change if you enter text and forget to change Russian to English or vice versa.
First of all, let's configure X11, or rather keyboard processing:

1) https://norvig.com/mayzner.html downloads N-gram.
2) Since I did not find an N-gram for the Russian language, we will teach ourselves. Download about 20-30 books in Russian and English.
3) generate frequencies. I have a program and a script configured for the first 5 characters (5-gram) (If you are lazy, they are in the repository. xz -d ngram_stats_en.h.xz - to unpack)
```
python genru3.py book-ru/* > ngram_stats_ru.h
python genen3.py book-en/* > ngram_stats_en.h
```
4) Manual configuration in the code, replace the address of your keyboard/event4 with yours. You can find the path with the command:
```
sudo cat /proc/bus/input/devices | grep -A5 -i "Keyboard"
```
And set the read beat
```
sudo chmod a+r /dev/input/event*
```
6) Package build:

```
gcc enru_change.c -o enru_change $(pkg-config --cflags --libs gtk+-3.0 appindicator3-0.1) -lX11 -lXtst -lpthread -std=c18
```

Implemented:
1) change the layout en\ru
2) replace entered incorrect characters
3) press the right Alt then the next word will be entered without auto-replacement \ If you press the Right and Left Alt, then until canceled by the same combination, auto-replacement is disabled.

Result:
First line - I want to enter this line
Second line - how the program worked
```
(Entered) Z wanted ...
(result) I wanted to write this text with the correct layout
```

Yes, it is not ideal, but we are working on the algorithm and training the model

HACK: if you want enter a password, then you will off a change: press Left Alt and Right Alt ...enter a password. Press Ralt and LAlt and  return cofig to back.
