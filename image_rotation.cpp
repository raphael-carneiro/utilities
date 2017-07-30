/*
 *	g++ -o image_rotation image_rotation.cpp `pkg-config --libs opencv`
 */

#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace std;

cv::Mat
rotate(cv::Mat src, cv::Point pt, double angle)
{
    cv::Mat dst;
    cv::Mat r = getRotationMatrix2D(pt, angle, 1.0);
    cv::warpAffine(src, dst, r, cv::Size(src.cols, src.rows), cv::INTER_NEAREST);
    return dst;
}

int
main(int argc, char **argv)
{
	double rotation_angle = 15.; // degrees
	int size_x = 350, size_y = 350; // pixels
	int rot_size_x = 100, rot_size_y = 100; // pixels
	int x_rotation = size_x / 2;
	int y_rotation = size_y / 2;
	char input_name[256] = "/home/rcarneiro/Documents/red_rect.png";
	char output_name[256] = "/home/rcarneiro/Documents/red_rect_nearest.png";
	cv::Rect roi;
	cv::Point pt = cv::Point(x_rotation, y_rotation);
	cv::Mat sample_input;
	cv::Mat sample_output;
	cv::Mat sample_aux;

	sample_input = cv::imread(input_name);
	sample_aux = rotate(sample_input, pt, rotation_angle);
	// ROI point is on the top-left corner
	roi = cv::Rect(cv::Point(x_rotation - rot_size_x / 2, y_rotation - rot_size_y / 2), cv::Size(rot_size_x, rot_size_y));
	sample_output = sample_aux(roi);
	cv::imwrite(output_name, sample_output);
	sample_input.release();
	sample_output.release();
	sample_aux.release();
	return 0;
}
