/*
 *	g++ -o text_box text_box.cpp `pkg-config --libs opencv`
 *	Example: ./text_box 800 600 100 450 200 350 top center 1 1 255 0 0 1 8 car 1234
 */

#include <opencv/cv.h>
#include <opencv/highgui.h>
using namespace std;
using namespace cv;

int
main(int argc, char **argv)
{
	const char usage[] = " <x_size> <y_size> <x_left> <y_top> <x_right> <y_bottom> {top|bottom} {center|left|right}"
						 " <font> <scale> <Red> <Green> <Blue> <thickness> <line> <text>\n";
	const char example[] = " 800 600 100 450 200 350 top center 1 1 255 0 0 1 8 car 1234\n";
	if (argc < 2 || strcmp(argv[1], "-h") == 0) {
		cout << "Usage:\n" << argv[0] << usage << "\n";
		cout << "Example:\n./" << argv[0] << example << "\n";
		exit(0);
	}
	if (argc < 17) {
		cout << "Insufficient number of arguments\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	string window_name = string(argv[0]);
	int window_x_size = atoi(argv[1]);
	int window_y_size = atoi(argv[2]);
	Mat img(window_y_size, window_x_size, CV_8UC3, Scalar::all(0));
	int x_left = atoi(argv[3]);
	int y_top = atoi(argv[4]);
	int x_right = atoi(argv[5]);
	int y_bottom = atoi(argv[6]);
	if (x_left < 0 || y_top >= window_y_size || x_right >= window_x_size || y_bottom < 0) {
		cout << "Invalid bounding box coordinates\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	bool location_top = (strcmp(argv[7], "top") == 0);
	if (strcmp(argv[7], "top") != 0 && strcmp(argv[7], "bottom") != 0) {
		cout << "Text location must be top or bottom\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	int alignment = strcmp(argv[8], "center") == 0 ? 0 : strcmp(argv[8], "left") == 0 ? -1 : strcmp(argv[8], "right") == 0 ? 1 : 9999;
	if (alignment == 9999) {
		cout << "Text alignment must be center, left or right\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	int font = atoi(argv[9]);
	if (font < 0 || (font > 7 && font < 16) || font > 23) {
		cout << "Font type must be in range 0..7 or 16..23\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	double scale = atof(argv[10]);
	int red = atoi(argv[11]);
	int green = atoi(argv[12]);
	int blue = atoi(argv[13]);
	if (red < 0 || red > 255 || green < 0 || green > 255 || blue < 0 || blue > 255) {
		cout << "Red, Green and Blue must be in range 0..255\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	Scalar box_color(blue, green, red);
	Scalar font_color(255, 255, 255);
	int thickness = atoi(argv[14]);
	if (thickness < 0) {
		cout << "Thickness must be 1 or more\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	int line = atoi(argv[15]);
	if (line != 8 && line != 4 && line != CV_AA) {
		cout << "Line type must be 8 (8-connected line), 4 (4-connected line) or 16 (antialiased line)\n" << "Usage:\n" << argv[0] << usage;
		exit(0);
	}
	string text = argv[16];
	for (int i = 17; i < argc; i++) {
		text += " " + string(argv[i]);
	}
	cv::Point p;
	int baseLine;	// y-coordinate of the baseline relative to the bottom-most text point

	// Draw the bounding box
	rectangle(img, cv::Point(x_left, img.rows - y_top), cv::Point(x_right, img.rows - y_bottom), box_color, thickness);

	Size text_size = getTextSize(text, font, scale, thickness, &baseLine);
	if (alignment == 1)
		p.x = x_right - text_size.width;
	else if (alignment == 0)
		p.x = (x_left + x_right)/2 - text_size.width/2;
	else
		p.x = x_left;
	if (location_top)
		p.y = img.rows - y_top - 1;
	else
		p.y = img.rows - y_bottom + text_size.height + 4;

	// Draw the text box
	rectangle(img, p, p + cv::Point(text_size.width, - text_size.height - 3), box_color, CV_FILLED);
	putText(img, text, p + cv::Point(0, -1), font, scale, font_color, thickness, line);

	imshow(window_name, img);
	cout << "\nPress \"Esc\" key to continue...\n";
	while(waitKey() != 27);
	img.release();
	return 0;
}
