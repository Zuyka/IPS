#include <cilk/cilk.h>
#include <cilk/cilk_api.h>
#include <cilk/reducer_max.h>
#include <cilk/reducer_min.h>
#include <cilk/reducer_vector.h>
#include <chrono>

using namespace std::chrono;

/// ‘ункци€ ReducerMaxTest() определ€ет максимальный элемент массива,
/// переданного ей в качестве аргумента, и его позицию
/// mass_pointer - указатель исходный массив целых чисел
/// size - количество элементов в массиве
void ReducerMaxTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_max_index<long, int>> maximum;
	cilk_for(long i = 0; i < size; ++i)
	{
		maximum->calc_max(i, mass_pointer[i]);
	}
	printf("Maximal element = %d has index = %d\n\n",
		maximum->get_reference(), maximum->get_index_reference());
}

void ReducerMinTest(int *mass_pointer, const long size)
{
	cilk::reducer<cilk::op_min_index<long, int>> minimum;
	cilk_for(long i = 0; i < size; ++i)
	{
		minimum->calc_min(i, mass_pointer[i]);
	}
	printf("Minimal element = %d has index = %d\n\n",
		minimum->get_reference(), minimum->get_index_reference());
}

/// ‘ункци€ ParallelSort() сортирует массив в пор€дке возрастани€
/// begin - указатель на первый элемент исходного массива
/// end - указатель на последний элемент исходного массива
void ParallelSort(int *begin, int *end)
{

	if (begin != end)
	{
		--end;
		int *middle = std::partition(begin, end, std::bind2nd(std::less<int>(), *end));
		std::swap(*end, *middle);
		cilk_spawn ParallelSort(begin, middle);
		ParallelSort(++middle, ++end);
		cilk_sync;
	}

	
}

void lab1(long mass_size_1)
{
	long i;
	//mass_size_1 = 10000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size_1];

	for (i = 0; i < mass_size_1; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	mass_begin = mass;
	mass_end = mass_begin + mass_size_1;
	ReducerMaxTest(mass, mass_size_1);
	ReducerMinTest(mass, mass_size_1);


	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<float> duration = (t2 - t1);
	printf("Duration is: %f seconds\n\n", duration.count());

	ReducerMaxTest(mass, mass_size_1);
	ReducerMinTest(mass, mass_size_1);

	delete[]mass;
}

void CompareForAndCilk_For(size_t sz) {

	std::vector<int> v2;

	long i;

	cilk::reducer<cilk::op_vector<int>>red_vec;
	red_vec->push_back(rand() % 20000 + 1);

	// врем€ работы стандартного цикла for
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	for (i = 0; i < sz; ++i)
	{
		v2.push_back(rand() % 20000 + 1);
	}
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<float> duration = (t2 - t1);
	printf("Duration for is: %f seconds\n\n", duration.count());


	// врем€ работы параллельного цикла cilk_for от Intel Cilk Plus
	high_resolution_clock::time_point t11 = high_resolution_clock::now();

	cilk_for(long i = 0; i < sz; ++i)
	{
		red_vec->push_back(rand() % 20000 + 1);
	}

	int y;

	high_resolution_clock::time_point t22 = high_resolution_clock::now();
	duration = (t22 - t11);
	printf("Duration cilk_for is: %f seconds\n\n", duration.count());
}

int main()
{
	srand((unsigned)time(0));

	// устанавливаем количество работающих потоков = 4
	__cilkrts_set_param("nworkers", "4");

	// «адание 3

	/*lab1(10000);
	lab1(100000);
	lab1(500000);
	lab1(1000000);*/

	// «адание 4

	CompareForAndCilk_For(1000000);
	CompareForAndCilk_For(100000);
	CompareForAndCilk_For(10000);
	CompareForAndCilk_For(1000);
	CompareForAndCilk_For(500);
	CompareForAndCilk_For(100);
	CompareForAndCilk_For(50);
	CompareForAndCilk_For(10);


	/*long i;
	const long mass_size_1 = 10000;
	int *mass_begin, *mass_end;
	int *mass = new int[mass_size_1];

	for (i = 0; i < mass_size_1; ++i)
	{
		mass[i] = (rand() % 25000) + 1;
	}

	mass_begin = mass;
	mass_end = mass_begin + mass_size_1;
	ReducerMaxTest(mass, mass_size_1);
	ReducerMinTest(mass, mass_size_1);

	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	ParallelSort(mass_begin, mass_end);

	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);
	printf("Duration is: %d seconds\n\n", duration.count());

	ReducerMaxTest(mass, mass_size_1);
	ReducerMinTest(mass, mass_size_1);

	delete[]mass; */
	system("pause");
	return 0;
}