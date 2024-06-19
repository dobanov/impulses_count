#!/bin/sh

read HOT_LAST < /tmp/hot_last_day
read HOT_TOTAL < /tmp/hot

read COLD_LAST < /tmp/cold_last_day
read COLD_TOTAL < /tmp/cold

HOT_THIS_DAY=$(($HOT_TOTAL-$HOT_LAST))
COLD_THIS_DAY=$(($COLD_TOTAL-$COLD_LAST))

echo $COLD_TOTAL > /tmp/cold_last_day
echo $HOT_TOTAL > /tmp/hot_last_day

MESSAGE="за сутки расход ${HOT_THIS_DAY}0 литров горячей воды и ${COLD_THIS_DAY}0 литров холодной"

~/bin/tg_send -b bbbbb:xxxxxx -c yyyyy,vvvvvvvv,aaaaaa -m "$MESSAGE"
