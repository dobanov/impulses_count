#!/bin/sh

RRDTOOL=/usr/bin/rrdtool
DATABASE=/opt/var/lib/rrd/rrdwater_new.rrd
PERIOD=300

read COLD_NOW < /tmp/cold
read HOT_NOW < /tmp/hot

IMAGE_PATH=/var/www/html/rrd

TITLE_TEXT='water'

UNDER_TEXT='hot'
UNDER_TEXT2='cold'

MIN=0

if ! [ -f $DATABASE ]
        then
                $RRDTOOL create $DATABASE -s $PERIOD DS:hot:DERIVE:600:U:U \
                DS:cold:DERIVE:600:U:U \
                RRA:AVERAGE:0.5:1:576 \
                RRA:AVERAGE:0.5:6:672 \
                RRA:AVERAGE:0.5:24:732 \
                RRA:AVERAGE:0.5:144:1460
        fi

$RRDTOOL update $DATABASE N:$HOT_NOW:$COLD_NOW
# ============================================================================

DRAW_GRAPHIC()
{

        NOW_HOUR=`date +%H`
        NOW_MIN=`date +%M`
        NOW_SEC=`date +%S`

        case $2 in
        day)
                 TIME_TEXT="day"
                 ;;
        week)
                 TIME_TEXT="week"
                 ;;
        month)
                 TIME_TEXT="month"
                 ;;
        year)
                 TIME_TEXT="year"
                 ;;
        esac

        $RRDTOOL graph $IMAGE_PATH/$1 \
        -s -1$2 \
        -e now \
        -a PNG \
        -t "$TITLE_TEXT - $TIME_TEXT" \
        -l $MIN \
        -r \
        -E \
        -i \
        -R light \
        --zoom 1.2 \
        -w 550 \
        -h 172 \
        DEF:hot=$DATABASE:hot:AVERAGE \
        DEF:cold=$DATABASE:cold:AVERAGE \
        VDEF:hot_total=hot,TOTAL \
        VDEF:cold_total=cold,TOTAL \
        LINE1:hot#ff0000:"$UNDER_TEXT" \
        GPRINT:hot_total:"total\: %4.0lf" \
        LINE1:cold#0000ff:"$UNDER_TEXT2" \
        GPRINT:cold_total:"total\: %4.0lf \n" \
        COMMENT:"time  \: $NOW_HOUR\:$NOW_MIN\:$NOW_SEC \n"
}

DRAW_GRAPHIC 'dwater_.png' 'day' 1> /dev/null
DRAW_GRAPHIC 'wwater_.png' 'week' 1> /dev/null
DRAW_GRAPHIC 'mwater_.png' 'month' 1> /dev/null
DRAW_GRAPHIC 'ywater_.png' 'year' 1> /dev/null
