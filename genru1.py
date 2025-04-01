import sys
import re
from collections import defaultdict
import traceback

def analyze_ngrams(files):
    # Инициализация счетчиков
    ngram_counts = {
        1: defaultdict(int),
        2: defaultdict(int),
        3: defaultdict(int),
        4: defaultdict(int)
    }
    totals = {1: 0, 2: 0, 3: 0, 4: 0}

    # Русский алфавит с учетом Ё в обоих регистрах
    russian_letters = set('АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯабвгдеёжзийклмнопрстуфхцчшщъыьэюя')

    # Улучшенное регулярное выражение для слов
    word_re = re.compile(r'(?u)\b[А-Яа-яЁё]+\b')

    for file_path in files:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                text = f.read().upper()  # Нормализуем регистр
                
                for word in word_re.findall(text):
                    if len(word) < 1:
                        continue
                    
                    # Обработка первой буквы (монограмма)
                    first_char = word[0]
                    if first_char in russian_letters:
                        ngram_counts[1][first_char] += 1
                        totals[1] += 1
                    
                    # Обработка n-грамм разной длины
                    for n in [2, 3, 4]:
                        for i in range(len(word) - n + 1):
                            ngram = word[i:i+n]
                            if all(c in russian_letters for c in ngram):
                                ngram_counts[n][ngram] += 1
                                totals[n] += 1

        except Exception as e:
            print(f"Ошибка при обработке файла {file_path}: {str(e)}", file=sys.stderr)
            traceback.print_exc()

    # Функция сортировки для русского алфавита
    def russian_sort_key(item):
        order = 'АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ'
        chars = item[0]
        return tuple(order.index(c) if c in order else 999 for c in chars)

    # Подготовка результатов
    results = []
    
    # Сбор статистики для всех n-грамм
    for n in [1, 2, 3, 4]:
        for ngram in sorted(ngram_counts[n].keys(), key=russian_sort_key):
            if totals[n] > 0:
                percentage = (ngram_counts[n][ngram] / totals[n]) * 100
                results.append((ngram, n, percentage))
    
    return results

def main():
    if len(sys.argv) < 2:
        print("Usage: python ngram_analyzer.py <file1> [file2 ...]")
        sys.exit(1)

    files = sys.argv[1:]
    stats = analyze_ngrams(files)

    # Вывод в формате C-структуры
    print("typedef struct {")
    print("    wchar_t character[6];")
    print("    int type;")
    print("    double percentage;")
    print("} NGramStat;")
    print("\nNGramStat ngram_stats[] = {")
    for item in stats:
        chars, n_type, perc = item
        print(f'    {{L"{chars}", {n_type}, {perc:.4f}f}},')
    
    print("};")

if __name__ == "__main__":
    main()
