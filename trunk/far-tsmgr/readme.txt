tsmgr: Terminal sessions manager

Плагин для управления сессиями Сервера терминалов
Работает только начиная с XP и выше.
Для управления сессиями терминала на удаленной машине нужно на ней
включить эту возможность в реестре:

[HKLM\SYSTEM\CurrentControlSet\Control\Terminal Server]
"AllowRemoteRPC"=dword:1

либо просто выполнить на ней файл "remote access.reg"

Умеет:
1) Отображать список сессий
2) Отключать сессию
3) Выгружать сессию
4) Посылать сообщение
5) Отображать информацию
6) Подключаться к серверам терминалов
7) Перегружать сервер
8) Выключать сервер

Спасибо за помошь:
VictorVG @ VikSoft.Ru создание русского lng файла

© 2010 Andrew Grechkin
	mailto: andrew.grechkin@gmail.com
	jabber: andrew.grechkin@gmail.com

Исходный код
	http://code.google.com/p/andrew-grechkin
