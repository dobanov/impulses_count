с помощью этой программы можно вести учет водоснабжения на raspberri pi подключив напрямую счетчики воды.

считает импульсы со счетчиков воды которые работают по релейной схеме. счетчик воды посылает импульс при прохождении через него 10 литров жидкости. красный провод от счетчика нужно подключить к земле, белый - к pin. для примера, 23 pin - счетчик холодной воды, 17 - горячей. после получения импульса в файл перезаписывается текущее значение увеличенное на 1. для учета холодной воды - файл cold.txt, горячей - hot.txt . при подключении счетчика к raspberry pi между землей и сигнальным проводом желательно установить керамический конденсатор 104.

![image](https://github.com/dobanov/impulses_count/assets/117526546/91c49a8b-3524-42a2-bee7-199e827616f9)

так же не забыть выполнить на rapsberry pi активацию pin командами 

echo 17 > /sys/class/gpio/export ; echo in > /sys/class/gpio/gpio17/direction

echo 23 > /sys/class/gpio/export ; echo in > /sys/class/gpio/gpio23/direction
