#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <inttypes.h>
#include <string.h>

uint64_t give_me_rand(void)
{
	static uint64_t seed = 0;
	if (seed == 0) {
		seed = (uint64_t)time(NULL);
		if (seed == 0) {
			seed = 1;
		}
	}

	uint64_t a = 16807ULL;
	uint64_t m = 0x7fffffffULL;

	seed = (seed * a) % m;
	return seed;
}

int main(int argc, char* argv[])
{
	const char *symbols = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789!@#$%^&*()-+[]{}?";
	uint64_t symbols_count = strlen(symbols);

	const uint64_t MAX_PASSWORD_LENGTH = 255;

	// Предварительный расчет лимита для защиты от смещения по модулю (делается один раз)
	uint64_t max_rand = 0x7ffffffeULL;
	uint64_t limit = (max_rand / symbols_count) * symbols_count;

	// -----------------------------------------------------------------
	// Сценарий 1: Аргументов нет, генерируем один пароль по умолчанию
	// -----------------------------------------------------------------
	if (argc == 1) {
		uint64_t default_length = 16;
		char password[MAX_PASSWORD_LENGTH + 1];

		for (uint64_t j = 0; j < default_length; j++) {
			uint64_t rand_val;
			do {
				rand_val = give_me_rand();
			} while (rand_val >= limit);
			password[j] = symbols[rand_val % symbols_count];
		}
		password[default_length] = '\0';
		printf("%s\n", password);

		return 0; // Успешно завершаем
	}

	// -----------------------------------------------------------------
	// Сценарий 2: Есть аргументы, обрабатываем каждый по очереди
	// -----------------------------------------------------------------
	for (int i = 1; i < argc; i++) {
		char *end;
		errno = 0;

		// 1. Проверка на отрицательное число
		if (argv[i][0] == '-') {
			fprintf(stderr, "ERROR: аргумент '%s' не может быть отрицательным\n", argv[i]);
			return 1;
		}

		// 2. Парсинг
		unsigned long long parsed_value = strtoull(argv[i], &end, 10);

		// 3. Валидация
		if (errno == ERANGE || end == argv[i] || *end != '\0' || parsed_value == 0 || parsed_value > MAX_PASSWORD_LENGTH) {
			fprintf(stderr, "Ошибка: '%s' не является корректной длиной (допустимо 1-%" PRIu64 ")\n", argv[i], MAX_PASSWORD_LENGTH);
			fprintf(stderr, "Использование: %s [длина1] [длина2] ...\n", argv[0]);
			return 1; // Прерываем выполнение при первой же ошибке (строгий режим)
		}

		uint64_t current_length = (uint64_t)parsed_value;

		// 4. Генерация пароля для текущей длины
		char password[MAX_PASSWORD_LENGTH + 1];

		for (uint64_t j = 0; j < current_length; j++) { // Используем 'j', чтобы не конфликтовать с 'i'
			uint64_t rand_val;
			do {
				rand_val = give_me_rand();
			} while (rand_val >= limit);

			password[j] = symbols[rand_val % symbols_count];
		}

		password[current_length] = '\0';

		// 5. Вывод
		printf("%s\n", password);
	}

	return 0;
}