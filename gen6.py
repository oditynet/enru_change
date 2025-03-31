import sys
from collections import defaultdict

def process_file(input_file, output_file):
    # Соответствие длины символа и целевого столбца
    length_to_column = {
        1: '1/1:1',
        2: '2/2:2', 
        3: '3/3:3',
        4: '4/4:4',
        5: '5/5:5'
    }

    with open(input_file, 'r') as f:
        header = f.readline().strip().split('\t')
        
        # Словарь для хранения данных и сумм
        data = []
        totals = defaultdict(float)
        
        # Собираем данные и суммы
        for line in f:
            parts = line.strip().split('\t')
            if len(parts) < 2:
                continue

            char = parts[0]
            char_length = len(char)
            
            # Пропускаем невалидные длины
            if char_length not in length_to_column:
                continue
                
            col_name = length_to_column[char_length]
            if col_name not in header:
                continue
                
            col_idx = header.index(col_name)
            
            try:
                value = float(parts[col_idx])
                data.append((char, char_length, value))
                totals[char_length] += value
            except (IndexError, ValueError):
                continue

        # Генерируем C-структуру
        c_code = """#ifndef NGRAM_STATS_H
#define NGRAM_STATS_H

typedef struct {
    wchar_t character;
    int type;
    double percentage;
} NGramStat;

NGramStat ngram_stats[] = {
"""
        
        # Обрабатываем записи и считаем проценты
        for char, length, value in data:
            if totals[length] == 0:
                continue
                
            percent = round((value / totals[length]) * 100, 4)
            c_code += f'    {{L"{char}", {length}, {percent}f}},\n'

        c_code += "};\n\n#endif // NGRAM_STATS_H"

    with open(output_file, 'w') as f:
        f.write(c_code)

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Usage: python ngram_processor.py input.txt output.h")
        sys.exit(1)
    
    process_file(sys.argv[1], sys.argv[2])
