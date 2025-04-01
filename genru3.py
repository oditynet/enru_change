import sys
import re
from collections import defaultdict
import traceback

def analyze_ngrams(files):
    # Сбор всех слов из всех файлов
    russian_letters = set('АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ')
    word_re = re.compile(r'(?u)\b[А-Яа-яЁё]+\b')
    words = []
    
    for file_path in files:
        try:
            with open(file_path, 'r', encoding='utf-8') as f:
                text = f.read().upper()
                words.extend(word_re.findall(text))
        except Exception as e:
            print(f"Ошибка при обработке файла {file_path}: {str(e)}", file=sys.stderr)
            traceback.print_exc()
            continue

    # Инициализация структур данных
    ngram_counts = {n: defaultdict(int) for n in [1, 2, 3, 4, 5]}
    totals = {n: 0 for n in [1, 2, 3, 4, 5]}

    # Отдельный подсчет для каждой N-граммы
    for n in [1, 2, 3, 4, 5]:
        for word in words:
            if len(word) < n:
                continue
                
            ngram = word[:n]
            if all(c in russian_letters for c in ngram):
                ngram_counts[n][ngram] += 1
                totals[n] += 1

    # Сортировка результатов
    def russian_sort_key(item):
        order = 'АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ'
        return tuple(order.index(c) for c in item[0])

    results = []
    for n in [1, 2, 3, 4, 5]:
        sorted_ngrams = sorted(ngram_counts[n].items(), key=russian_sort_key)
        for ngram, count in sorted_ngrams:
            if totals[n] > 0:
                percentage = (count / totals[n]) * 100
                results.append((ngram, n, percentage))
    
    return results

def main():
    if len(sys.argv) < 2:
        print("Usage: python ngram_analyzer.py <file1> [file2 ...]")
        sys.exit(1)

    stats = analyze_ngrams(sys.argv[1:])

    # Генерация C-структуры
    print("typedef struct {")
    print("    wchar_t character[6];")  # 4 символа + терминатор
    print("    int type;")
    print("    double percentage;")
    print("} NGramStat_ru;\n")
    print("NGramStat_ru ngram_stats_ru[] = {")
    for chars, n, prob in stats:
        print(f'    {{L"{chars}", {n}, {prob:.4f}f}},')
    print("};\n")

if __name__ == "__main__":
    main()
