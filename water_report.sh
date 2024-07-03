#!/bin/sh

# Параметр, указывающий текущий период (например, day, month, week)
period="$1"

# Путь к файлам счетчиков
HOT_LAST_FILE="/tmp/hot_last_$period"
HOT_TOTAL_FILE="/tmp/hot"
COLD_LAST_FILE="/tmp/cold_last_$period"
COLD_TOTAL_FILE="/tmp/cold"

# Создание файлов счетчиков, если они не существуют
touch "$HOT_LAST_FILE"
touch "$COLD_LAST_FILE"

# Чтение значений счетчиков
read HOT_LAST < "$HOT_LAST_FILE"
read HOT_TOTAL < "$HOT_TOTAL_FILE"
read COLD_LAST < "$COLD_LAST_FILE"
read COLD_TOTAL < "$COLD_TOTAL_FILE"

# Если файлы не были созданы, установить значения по умолчанию в ноль
if [ -z "$HOT_LAST" ]; then
    HOT_LAST=0
fi

if [ -z "$COLD_LAST" ]; then
    COLD_LAST=0
fi

# Вычисление текущего периода
HOT_THIS_PERIOD=$((HOT_TOTAL - HOT_LAST))
COLD_THIS_PERIOD=$((COLD_TOTAL - COLD_LAST))

# Запись значений счетчиков
echo "$COLD_TOTAL" > "$COLD_LAST_FILE"
echo "$HOT_TOTAL" > "$HOT_LAST_FILE"

# Формирование сообщения
MESSAGE="за $period расход ${HOT_THIS_PERIOD}0 литров горячей воды и ${COLD_THIS_PERIOD}0 литров холодной"

~/bin/tg_send -b bbbbb:xxxxxx -c yyyyy,vvvvvvvv,aaaaaa -m "$MESSAGE"
