all: build_server build_client

build_server:
# -ggdb - добавить отладочную информацию
# -O0   - установить уровень оптимизации в ноль
# -o    - результат компиляции и сборки
	gcc -ggdb -O0 -o server server.c

build_client:
	gcc -ggdb -O0 -o client client.c
