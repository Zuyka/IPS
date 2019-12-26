#include "pch.h"
#include <fstream>
#include <vector>
#include "Fragmentation.h"
#include <cilk\cilk.h>
#include <cilk\reducer_vector.h>

/// вектор, содержащий box-ы, €вл€ющиес€ частью рабочего пространства
cilk::reducer< cilk::op_vector<Box> > solution;
/// вектор, содержащий box-ы, не €вл€ющиес€ частью рабочего пространства
cilk::reducer< cilk::op_vector<Box> > not_solution;
/// вектор, содержащий box-ы, наход€щиес€ на границе между "рабочим" и "нерабочим" пространством
cilk::reducer< cilk::op_vector<Box> > boundary;
/// вектор, хран€щий box-ы, анализируемые на следующей итерации алгоритма
cilk::reducer< cilk::op_vector<Box> > temporary_boxes;



extern double g_l1_max;
extern double g_l2_max;
extern double g_l1_min;
extern double g_l2_min;
extern double g_l0;

extern double g_precision;

/// вектор, содержащий box-ы, €вл€ющиес€ частью рабочего пространства
/// вектор, содержащий box-ы, не €вл€ющиес€ частью рабочего пространства
//std::vector<Box> not_solution;
/// вектор, содержащий box-ы, наход€щиес€ на границе между "рабочим" и "нерабочим" пространством
//std::vector<Box> boundary;
/// вектор, хран€щий box-ы, анализируемые на следующей итерации алгоритма
//std::vector<Box> temporary_boxes;

/// функции gj()
//------------------------------------------------------------------------------------------
double g1(double x1, double x2)
{
	return (x1*x1 + x2 * x2 - g_l1_max * g_l1_max);
}

//------------------------------------------------------------------------------------------
double g2(double x1, double x2)
{
	return (g_l1_min*g_l1_min - x1 * x1 - x2 * x2);
}

//------------------------------------------------------------------------------------------
double g3(double x1, double x2)
{
	return (x1*x1 + x2 * x2 - g_l2_max * g_l2_max);
}

//------------------------------------------------------------------------------------------
double g4(double x1, double x2)
{
	return (g_l2_min*g_l2_min - x1 * x1 - x2 * x2);
}


//------------------------------------------------------------------------------------------
low_level_fragmentation::low_level_fragmentation(double& min_x, double& min_y, double& x_width, double& y_height)
{
	current_box = Box(min_x, min_y, x_width, y_height);
}

//------------------------------------------------------------------------------------------
low_level_fragmentation::low_level_fragmentation(const Box& box)
{
	current_box = box;
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::VerticalSplitter(const Box& box, boxes_pair& vertical_splitter_pair)
{
	double xmin, ymin, width, height;
	box.GetParameters(xmin, ymin, width, height);
	vertical_splitter_pair.first = Box(xmin, ymin, width / 2.0, height);
	vertical_splitter_pair.second = Box(xmin + width / 2.0, ymin, width / 2.0, height);
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::HorizontalSplitter(const Box& box, boxes_pair& horizontal_splitter_pair)
{
	double xmin, ymin, width, height;
	box.GetParameters(xmin, ymin, width, height);
	horizontal_splitter_pair.first = Box(xmin, ymin, width, height / 2.0);
	horizontal_splitter_pair.second = Box(xmin, ymin + height / 2.0, width, height / 2.0);
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::GetNewBoxes(const Box& box, boxes_pair& new_pair_of_boxes)
{
	double width, height;
	box.GetWidhtHeight(width, height);

	if (abs(width) >= abs(height))
	{
		VerticalSplitter(box, new_pair_of_boxes);
	}
	else
	{
		HorizontalSplitter(box, new_pair_of_boxes);
	}
}

//------------------------------------------------------------------------------------------
unsigned int low_level_fragmentation::FindTreeDepth()
{
	double box_diagonal = current_box.GetDiagonal();

	if (box_diagonal <= g_precision)
	{
		return 0;
	}
	else
	{
		boxes_pair new_boxes;
		// допустим, разобьем начальную область по ширине
		VerticalSplitter(current_box, new_boxes);
		unsigned int tree_depth = 1;

		box_diagonal = new_boxes.first.GetDiagonal();

		if (box_diagonal <= g_precision)
		{
			return tree_depth;
		}
		else
		{
			for (;;)
			{
				GetNewBoxes(new_boxes.first, new_boxes);
				++tree_depth;
				box_diagonal = new_boxes.first.GetDiagonal();

				if (box_diagonal <= g_precision)
				{
					break;
				}
			}
			return tree_depth;
		}
	}
}

//------------------------------------------------------------------------------------------
int low_level_fragmentation::ClasifyBox(const min_max_vectors& vects)
{
	int count = 0;

	for (int i = 0; i < vects.second.size(); i++)
	{
		if (vects.second[i] < 0)
			count += 1;
		if (vects.first[i] > 0)
			return 1;//класс not solution
	}

	if (count == vects.second.size())
		return 0;//класс solution	

	if (vects.first[0] == 0 && vects.second[0] == 0)
		return 2;//класс boundary

	return 3;//класс temporary boxes
}

//------------------------------------------------------------------------------------------
void low_level_fragmentation::GetBoxType(const Box& box)
{
	min_max_vectors min_max_vecs;
	boxes_pair new_pair_of_boxes;

	GetMinMax(box, min_max_vecs);
	int res = ClasifyBox(min_max_vecs);

	switch (res)
	{
		case 0:
		{
			solution->push_back(box);
			break;
		}
		case 1:
		{
			not_solution->push_back(box);
			break;
		}
		case 2:
		{
			boundary->push_back(box); break;
		}
		case 3:
		{
			GetNewBoxes(box, new_pair_of_boxes);
			temporary_boxes->push_back(new_pair_of_boxes.first);
			temporary_boxes->push_back(new_pair_of_boxes.second);
			break;
		}
	}
}

	//------------------------------------------------------------------------------------------
	high_level_analysis::high_level_analysis(double& min_x, double& min_y, double& x_width, double& y_height) :
		low_level_fragmentation(min_x, min_y, x_width, y_height) {}

	//------------------------------------------------------------------------------------------
	high_level_analysis::high_level_analysis(Box& box) : low_level_fragmentation(box) {}

	//------------------------------------------------------------------------------------------
	void high_level_analysis::GetMinMax(const Box& box, min_max_vectors& min_max_vecs)
	{
		std::vector<double> g_min;
		std::vector<double> g_max;

		double a1min, a2min, a1max, a2max;
		double xmin, xmax, ymin, ymax;

		box.GetParameters(xmin, ymin, xmax, ymax);

		xmax = xmin + xmax;
		ymax = ymin + ymax;

		double curr_box_diagonal = box.GetDiagonal();

		if (curr_box_diagonal <= g_precision)
		{
			g_min.push_back(0);
			g_max.push_back(0);

			min_max_vecs.first = g_min;
			min_max_vecs.second = g_max;

			return;
		}

		// MIN
		// функци€ g1(x1,x2)
		a1min = __min(abs(xmin), abs(xmax));
		a2min = __min(abs(ymin), abs(ymax));
		g_min.push_back(g1(a1min, a2min));

		// функци€ g2(x1,x2)
		a1min = __max(abs(xmin), abs(xmax));
		a2min = __max(abs(ymin), abs(ymax));
		g_min.push_back(g2(a1min, a2min));

		// функци€ g3(x1,x2)
		a1min = __min(abs(xmin - g_l0), abs(xmax - g_l0));
		a2min = __min(abs(ymin), abs(ymax));
		g_min.push_back(g3(a1min, a2min));

		// функци€ g4(x1,x2)
		a1min = __max(abs(xmin - g_l0), abs(xmax - g_l0));
		a2min = __max(abs(ymin), abs(ymax));
		g_min.push_back(g4(a1min, a2min));

		// MAX
		// функци€ g1(x1,x2)
		a1max = __max(abs(xmin), abs(xmax));
		a2max = __max(abs(ymin), abs(ymax));
		g_max.push_back(g1(a1max, a2max));

		// функци€ g2(x1,x2)
		a1max = __min(abs(xmin), abs(xmax));
		a2max = __min(abs(ymin), abs(ymax));
		g_max.push_back(g2(a1max, a2max));

		// функци€ g3(x1,x2)
		a1max = __max(abs(xmin - g_l0), abs(xmax - g_l0));
		a2max = __max(abs(ymin), abs(ymax));
		g_max.push_back(g3(a1max, a2max));

		// функци€ g4(x1,x2)
		a1max = __min(abs(xmin - g_l0), abs(xmax - g_l0));
		a2max = __min(abs(ymin), abs(ymax));
		g_max.push_back(g4(a1max, a2max));

		min_max_vecs.first = g_min;
		min_max_vecs.second = g_max;
	}

	//------------------------------------------------------------------------------------------
	void high_level_analysis::GetSolution()
	{
		current_box = Box(-g_l1_max, 0, g_l2_max + g_l0 + g_l1_max, __min(g_l1_max, g_l2_max));
		std::vector<Box> current_boxes;
		temporary_boxes->push_back(current_box);
		std::vector<Box> test;

		int level = FindTreeDepth();
		for (int i = 0; i < (level + 1); ++i)
		{
			temporary_boxes.move_out(current_boxes);
			temporary_boxes.set_value(test);
			cilk_for(int j = 0; j < current_boxes.size(); ++j)
				GetBoxType(current_boxes[j]);
		}

	}


	//------------------------------------------------------------------------------------------
	void WriteResults(const char* file_names[])
	{
		double x_min, y_min, width, height;
		std::vector <Box> temp;

		std::ofstream fsolution(file_names[0]);
		std::ofstream fboundary(file_names[1]);
		std::ofstream fnot_solution(file_names[2]);

		solution.move_out(temp);
		for (int i = 0; i < temp.size(); i++)
		{

			temp[i].GetParameters(x_min, y_min, width, height);
			fsolution << x_min << " " << y_min << " " << width << " " << height << '\n';
		}

		temp.clear();
		boundary.move_out(temp);
		for (int i = 0; i < temp.size(); i++)
		{
			temp[i].GetParameters(x_min, y_min, width, height);
			fboundary << x_min << " " << y_min << " " << width << " " << height << '\n';
		}

		temp.clear();
		not_solution.move_out(temp);
		for (int i = 0; i < temp.size(); i++)
		{
			temp[i].GetParameters(x_min, y_min, width, height);
			fnot_solution << x_min << " " << y_min << " " << width << " " << height << '\n';
		}

		fsolution.close();
		fboundary.close();
		fnot_solution.close();

	}
