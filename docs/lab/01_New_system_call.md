# 01. Создание собственного сервиса ядра

## Цель
- изучить механизм диспетчеризации сервисов ядра
- научиться добавлять собственный сервис, включая:
    - функцию сервиса
    - строки в таблицах диспетчеризации
- научиться вызывать сервисы ядра напрямую (минуя стандартные обертки из ntdll.dll)

## Материалы для изучения
- [Howto: Implementation of new system service calls (I)](https://www.dcl.hpi.uni-potsdam.de/research/WRK/2007/07/howto-implementation-of-new-system-service-calls/)
- [Howto: Implementation of new system service calls (II)](https://www.dcl.hpi.uni-potsdam.de/research/WRK/2007/07/howto-implementation-of-new-system-service-calls-ii/)
- [Howto: Implementation of new system service calls (III)](https://www.dcl.hpi.uni-potsdam.de/research/WRK/2008/03/howto-implementation-of-new-system-service-calls-iii/)
- [WRK System Service Calls Made Simple](https://www.dcl.hpi.uni-potsdam.de/research/WRK/2009/03/implementation-of-a-new-system-service-call-2009-update/)

## Необходимые нструменты
- пакет WRK
- виртуальная машина c Windows 2003 
- пакет Windows Debugging Tools
- утилита systablegen с сайта [https://wrktools.codeplex.com](https://archive.codeplex.com/?p=wrktools). Её также можно найти в папке **Resources\\wrktools\\systablegen**
- Visual Studio 2017 с установленной поддержкой сборки C++ программ для Windows XP
