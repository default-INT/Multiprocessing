// GaussST.cpp : Этот файл содержит функцию "main". Здесь начинается и заканчивается выполнение программы.
//

#include <random>
#include <iostream>
#include <iomanip>
#include <chrono>
#include <Windows.h>

// Макрос в виндовых инклудах скрывает std::numeric_limits<std::streamsize>::max() из limits
#undef max()

// Для распараллеливания по столбцам удобно конкатенировать A и B
int gauss(float *ab, float *x, int equations_count);
// Несколько вспомогаательных функций
int input_int_value(int default_value);
void output_system(float *ab, int equations_count);
void output_solution(float *x, int equations_count);

// Региональные настройки
std::locale c_locale, ru_locale;

int main()
{
	// Установка русской кодовой страницы для Windows консоли
	SetConsoleCP(1251);
	SetConsoleOutputCP(1251);
	// Установка региональных настроек для стандартных функций и потоков ввода/вывода C++ с сохранением культуро-независимой C-локали на случай нужды,
	// например, вывода больших чисел без разделителя тысяч
	ru_locale = std::locale("Russian", std::locale::all);
	c_locale = std::locale::global(ru_locale);
	std::cin.imbue(ru_locale);
	std::cout.imbue(ru_locale);

	// Инициализация генератора псевдо-случайных чисел
	std::mt19937 random_generator(5287U);
	// Для генерации каждый раз другой последовательности нужно вызвать
	// random_generator.seed(seedValue);
	// со значением, зависящим от времени
	std::uniform_int_distribution<int> int_distribution(0, 10);
	
	// Запрос пользователя кол-ва уравнений
																	//std::cout << "Enter system dimension ";
																	//equations_count = input_int_value(0);
	int equations_count = 5;
	if (0 >= equations_count) return 0;
	int col_count = equations_count + 1;

	// Создание в хипе динамического 2-мерного массива
	// Не забываем про конкатенацию матрицы A и вектора B
	float *ab = new float[equations_count * col_count];
	
	// Рандомная генерация коэффициентов для матрицы A
	for (int row_index = 0; row_index <= equations_count - 1; ++row_index)
		for (int col_index = 0; col_index <= equations_count - 1; ++col_index) {
			int r = int_distribution(random_generator);
			ab[row_index * col_count + col_index] = (float)r;
		}
			
	// Рандомная генерация коэффициентов для вектора B
	for (int row_index = 0; row_index <= equations_count - 1; ++row_index) {
		int r = int_distribution(random_generator);
		ab[row_index * col_count + equations_count] = (float)r;
	}

	// Создание в хипе динамического массива для результатов расчета
	float *x = new float[equations_count];

	// Смотрим исходную систему
															output_system(ab, equations_count);

	// Засекаем время
	std::chrono::time_point<std::chrono::high_resolution_clock> time_start = std::chrono::high_resolution_clock::now();

	// Решаем систему
	if (gauss(ab, x, equations_count) != 0) {
		// При неудаче вываливаемся с ошибкой
		std::cout << "Error" << std::endl;
		return 1;
	}

	// Смотрим сколько потрачено
	std::chrono::time_point<std::chrono::high_resolution_clock> time_end = std::chrono::high_resolution_clock::now();

	// Смотрим результат
															output_solution(x, equations_count);

	// Смотрим затраты времени
	std::chrono::nanoseconds delta_time = std::chrono::duration_cast<std::chrono::nanoseconds>(time_end - time_start);
	std::cout << "Time: " << delta_time.count() * 0.001f << "us" << std::endl;

	// Освобождаем выделенную память (за собой всегда нужно убирать) в порядке, обратном выделению
	delete[] x;
	delete[] ab;

	std::cout << "Press Enter to exit" << std::endl;
	std::getchar();
	return 0;
}

int gauss(float *ab, float *x, int equations_count) {
	int col_count = equations_count + 1;

	// Выделение памяти под буффер для коэффициентов под/над "опорным" элементом
	float *coefficients_buffer = new float[equations_count];

	float divider = 1.0f;
	// result = 0 - все хорошо, решение нашлось
	int result = 0;
	// Номер "опорного" элемента
	int base_element_index;

	// 1. Прямой проход. Цикл по "опорным" элементам (т.е. по строкам) вниз
	for (base_element_index = 0; base_element_index <= equations_count - 1; ++base_element_index) {

		// 1.1. Делаем "опорный" элемент главной диагонали равным 1.0
		divider = ab[base_element_index * col_count + base_element_index];
		// Примечание: если "опорный" элемент = 0 - система не решаема без перестановки строк. Я заморачиваться не буду, просто отвалюсь с сообщением об ошибке
		if (fabs(divider) < 1.0e-12f) {
			result = -1;
			break;
		}
		// В цикле по столбцам не забываем про конкатенацию матрицы A и вектора B
		for (int j = 0; j <= equations_count; ++j) ab[base_element_index * col_count + j] /= divider;
		// Для последней строки только делим, вычитать произведения не надо, поэтому break
		if (equations_count - 1 == base_element_index) break;

		// 1.2. Сохраняем значения под текущим опорным элементом в буфер
		for (int i = base_element_index + 1; i <= equations_count - 1; ++i)
			coefficients_buffer[i] = ab[i * col_count + base_element_index];

		// 1.3. Делаем элементы ниже опорного элемента равными 0.0. Цикл вначале по столбцам для будущего исключения конкуренции при распараллеливании
		// (не забываем про конкатенацию матрицы A и вектора B)
		for (int col_index = base_element_index; col_index <= equations_count; ++col_index) // 
			for (int i = base_element_index + 1; i <= equations_count - 1; ++i)
				ab[i * col_count + col_index] -= ab[base_element_index * col_count + col_index] * coefficients_buffer[i];
	}

	// Если "опорный" = 0 вываливаемся из функции
	if (result < 0) return result;

	// 2. Обратный проход. Цикл по "опорным" элементам (т.е. по строкам) вверх
		// Для нулевой строки действий не надо
	for (base_element_index = equations_count - 1; base_element_index > 0; --base_element_index) {

		// 2.1. Сохраняем значения над текущим "опорным" элементом в буфер
		for (int i = base_element_index - 1; i >= 0; --i)
			coefficients_buffer[i] = ab[i * col_count + base_element_index];

		// 2.2. Делаем элементы выше "опорного" элемента равными 0.0. Обрабатываемых столбцов только 2: над "опорным" и в B (не забываем про конкатенацию матрицы A и вектора B)
		int col_index = base_element_index;
		for (int i = base_element_index - 1; i >= 0; --i)
			ab[i * col_count + col_index] -= ab[base_element_index * col_count + col_index] * coefficients_buffer[i];
		col_index = equations_count;
		for (int i = base_element_index - 1; i >= 0; --i)
			ab[i * col_count + col_index] -= ab[base_element_index * col_count + col_index] * coefficients_buffer[i];
	}

	// Копирование результата
	for (int i = 0; i <= equations_count - 1; ++i)
		x[i] = ab[i * col_count + equations_count];

	// Освобождаем динамическую память
	delete[] coefficients_buffer;
	return result;
}

int input_int_value(int default_value) {
	int new_value = default_value;
	std::cin >> new_value;
	bool error_input = std::cin.fail();
	if (error_input) std::cin.clear();
	// Если нету #undef max() после инклудов, то std::cin.ignore(LLONG_MAX, '\n');
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
	return new_value;
}

void output_system(float *ab, int equations_count) {
	int col_count = equations_count + 1;
	for (int i = 0; i <= equations_count - 1; ++i) {
		for (int j = 0; j <= equations_count - 1; ++j)
			std::cout << std::setw(12) << ab[i * col_count + j] << ' ';
		std::cout << std::endl;
	}
	std::cout << std::endl;
	for (int i = 0; i <= equations_count - 1; ++i)
		std::cout << std::setw(12) << ab[i * col_count + equations_count] << std::endl;
	std::cout << std::endl;
}

void output_solution(float *x, int equations_count) {
	for (int i = 0; i <= equations_count - 1; ++i)
		std::cout << std::setw(12) << x[i] << std::endl;
	std::cout << std::endl;
}