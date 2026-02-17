CC = clang
CFLAGS = 
LDFLAGS_DYNAMIC = -ldl

# Имена библиотек
LIBS = libgcd1.so libgcd2.so libconvert1.so libconvert2.so

# Основные цели
all: $(LIBS) test_static test_dynamic

# Сборка библиотек
libgcd1.so: libgcd1.c library.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

libgcd2.so: libgcd2.c library.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

libconvert1.so: libconvert1.c library.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@

libconvert2.so: libconvert2.c library.h
	$(CC) $(CFLAGS) -shared -fPIC $< -o $@


test_static: test_static.c libgcd1.c libconvert1.c library.h
	$(CC) $(CFLAGS) -o test_static test_static.c -L. -lgcd1 -lconvert1


test_dynamic: test_dynamic.c library.h
	$(CC) $(CFLAGS) $< $(LDFLAGS_DYNAMIC) -o $@


# Очистка
clean:
	rm -f *.o *.so test_static test_dynamic
