<--- 28 лабораторная! --->

Реализуйте простой HTTP-клиент. Он принимает один параметр командной строки – URL.
Клиент делает запрос по указанному URL и выдает тело ответа на терминал как текст (т.е. если в ответе HTML, то распечатывает его исходный текст без форматирования).
Вывод производится по мере того, как данные поступают из HTTP-соединения. Когда будет выведено более экрана (более 25 строк) данных,
клиент должен продолжить прием данных, но должен остановить вывод и выдать приглашение Press space to scroll down.

При нажатии пользователем клиент должен вывести следующий экран данных.
Для одновременного считывания данных с терминала и из сетевого соединения используйте системный вызов select.

<--- 29 лабораторная! --->

Реализуте задачу упр. 28, используя системные вызовы aio_read/aio_write.

<--- 30 лабораторная! --->

Реализуйте задачу упр. 28, используя две нити, одну для считывания данных из сетевого соединения, другую для взаимодействия с пользователем.