#!/bin/sh

# Параметр, указывающий текущий период (например, day, month, week)
period="$1"

# Путь к файлам счетчиков
HOT_LAST_FILE="/tmp/hot_last_$period"
COLD_LAST_FILE="/tmp/cold_last_$period"

# Проверка существования файлов и создание их при необходимости
if [ ! -e "$HOT_LAST_FILE" ]; then
    touch "$HOT_LAST_FILE"
    HOT_LAST=0
else
    read HOT_LAST < "$HOT_LAST_FILE"
fi

if [ ! -e "$COLD_LAST_FILE" ]; then
    touch "$COLD_LAST_FILE"
    COLD_LAST=0
else
    read COLD_LAST < "$COLD_LAST_FILE"
fi

# Чтение значений счетчиков
read HOT_TOTAL < "/tmp/hot"
read COLD_TOTAL < "/tmp/cold"

# Вычисление текущего периода
HOT_THIS_PERIOD=$((HOT_TOTAL - HOT_LAST))
COLD_THIS_PERIOD=$((COLD_TOTAL - COLD_LAST))

# Запись значений счетчиков
echo "$COLD_TOTAL" > "$COLD_LAST_FILE"
echo "$HOT_TOTAL" > "$HOT_LAST_FILE"

# Формирование сообщения
MESSAGE="за $period расход ${HOT_THIS_PERIOD}0 литров горячей воды и ${COLD_THIS_PERIOD}0 литров холодной"

~/bin/tg_send -b bbbbb:xxxxxx -c yyyyy,vvvvvvvv,aaaaaa -m "$MESSAGE"
