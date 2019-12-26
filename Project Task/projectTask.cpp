#include "pch.h"
#include "fragmentation.h"
#include <locale.h>
#include <cilk/cilk_api.h>
#include <chrono>

using namespace std::chrono;

/// параметры начальной прямоугольной области
double g_l1_max = 14.0;
double g_l2_max = g_l1_max;
double g_l1_min = 6.0;
double g_l2_min = g_l1_min;
double g_l0 = 3.0;

/// точность аппроксимации рабочего пространства
double g_precision = 0.1;


int main()
{
	setlocale(LC_ALL, "Rus");

	high_level_analysis main_object;
	__cilkrts_end_cilk();
	__cilkrts_set_param("nworkers", "4");
	main_object.GetSolution();
	// Внимание! здесь необходимо определить пути до выходных файлов!
	const char* out_files[3] = { "solution.txt", "boundary.txt", "not_solution.txt" };
	high_resolution_clock::time_point t1 = high_resolution_clock::now();
	WriteResults(out_files);
	high_resolution_clock::time_point t2 = high_resolution_clock::now();

	duration<double> duration = (t2 - t1);

	printf("Duration is: %f seconds \n\n", duration.count());
	printf("Number of workers  %d \n\n" , __cilkrts_get_nworkers());
	return 0;
}