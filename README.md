# NMeaMSGParser
Парсер для GGA, GLL, GSA, GSV, RMC, VTG, GST, ZDA, DTM и GNS сообщений.

# Запуск кода
Код был написан и проверялся под Ubuntu. Для запуска кода зайти в корневую директорию директорию проекта в терминале и выполнить следующую команду:
 
    g++ -I include src/main.cpp src/gnss_parser.cpp -o gnss_app

в директории появиться файл gnss_app, который можно запустить выполнив в терминале:

    ./gnss_app

Данные на вход передаются в txt файле в директории data. При желании можно передать путь к файлу с входными данными в виде аргумента коммандной строки, например:

    ./gnss_app text.txt


Результаты выполнения кода запишутся в файл **data/output.txt**. Если такого файла или папки нет, они будут созданы.
Пример работы:

>Message itself is $GPGGA,092725.00,4717.11399,N,00833.91590,E,1,08,1.01,499.6,M,48.0,M,,*5B
>
>Message header type is GGA
>
>Message header device is GPS, SBAS or QZSS
>
>UTC time stamp for this message is : 09:27:25.00
>
>47 Degrees, 17 Minutes, 6.83940 Seconds Direction N direction
>
>8 Degrees, 33 Minutes, 54.95400 Seconds Direction E direction
>
>Altitude above mean sea level : 499.6 Unit of altitude is meter.
>
>Autonomous GNSS fix
>
>Number of used satellites : 08
>
>Horizontal Dilution of Precision is : 1.01
>
>Difference between ellipsoid and mean sea level : 48.0 Unit sepparation is meter
>
>Checksum is *5B







