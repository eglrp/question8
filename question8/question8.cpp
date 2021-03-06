// question8.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

using namespace cv;
using namespace std;


int main(int argc, char **argv)
{
	ifstream fin("data.txt");//the path of the data
	ofstream fout("calibration_result2.txt");//result file
	cout << "start to find the corner………………";

	/*find the corner*/
	int image_count = 0;
	Size image_size;//the size of image
	Size board_size = Size(9, 6);//the number of the corner
	vector<Point2f> image_points_buf;//the coordiante of the corner
	vector<vector<Point2f>> image_points_seq;//the lib of the corner
	string filename;
	int count = -1;

	while (getline(fin, filename))
	{
		image_count++;

		/*supervise the input*/
		cout << "image_count = " << image_count << endl;
		Mat imageInput = imread(filename);

		/*get the width and height of the data when reading the first image*/
		if (image_count == 1)
		{
			image_size.width = imageInput.cols;
			image_size.height = imageInput.rows;
			cout << "image_size.width = " << image_size.width << endl;
			cout << "image_size.height = " << image_size.height << endl;
		}

		/*call the function*/
		if (0 == findChessboardCorners(imageInput, board_size, image_points_buf))
		{
			cout << "can not find chessboard corners!\n";
			exit(1);
		}
		else
		{
			Mat view_gray;
			cvtColor(imageInput, view_gray, CV_RGB2BGR);
			///*refine the pix*/
			//cornerSubPix(view_gray, image_points_buf, Size(5, 5), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
			image_points_seq.push_back(image_points_buf);

			/*show the corner at the image*/
			drawChessboardCorners(view_gray, board_size, image_points_buf, false);
			/*imshow("Corners", view_gray);
			waitKey(500);*/
		}
	}

	cout << "start calibratrion";
	/*the imformation of the chessboard*/
	Size square_size = Size(10, 10);//the size of the square
	vector<vector<Point3f>> object_points;//the lib of the coordinate in world system

	/*camera parameters*/
	Mat cameraMatrix = Mat(3, 3, CV_32FC1, Scalar::all(0));//Internal matrix
	vector<int> point_counts;//the number of corner each image
	Mat distCoeffs = Mat(1, 5, CV_32FC1, Scalar::all(0));//disortion coefficient
	vector<Mat> tvecsMat;//transformation vector
	vector<Mat> rvecsMat;//rotation vector

	/*initialize the 3D coordinate*/
	for (size_t i = 0; i < image_count; i++)
	{
		vector<Point3f> tempPointSet;
		for (size_t j = 0; j < board_size.height; j++)
		{
			for (size_t k = 0; k < board_size.width; k++)
			{
				Point3f realPoint;
				realPoint.x = j * square_size.width;
				realPoint.y = k * square_size.height;
				realPoint.z = 0;
				tempPointSet.push_back(realPoint);
			}
		}
		object_points.push_back(tempPointSet);
	}

	/*initialize the number of the corners*/
	for (size_t i = 0; i < image_count; i++)
	{
		point_counts.push_back(board_size.width*board_size.height);
	}

	/*call the calibration function*/
	//calibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat, 0);
	mycalibrateCamera(object_points, image_points_seq, image_size, cameraMatrix, distCoeffs, rvecsMat, tvecsMat);

	/*save the reslut*/
	std::cout << "start to save the reslut………………" << endl;
	Mat rotation_matrix = Mat(3, 3, CV_32FC1, Scalar::all(0)); /* the rotation matrix */
	fout << "internal matrix:" << endl;
	fout << cameraMatrix << endl << endl;
	fout << "disortion coefficient:\n";
	fout << distCoeffs << endl << endl << endl;
	for (int i = 0; i < image_count; i++)
	{
		fout << "The transpotation matrix of No." << i + 1 << endl;
		fout << tvecsMat[i] << endl;
		/* transform the rotation vector to rotation matrix */
		Rodrigues(rvecsMat[i], rotation_matrix);
		fout << "The rotation matrix of No." << i + 1 << endl;
		fout << rotation_matrix << endl;
		fout << "The rotation vector of No." << i + 1 << endl;
		fout << rvecsMat[i] << endl << endl;
	}
	std::cout << "completed" << endl;
	fout << endl;



	//Evaluation of calibration results  
	cout << "start evaluating the calibration results………………\n";
	double total_err = 0.0; /* the totol error */
	double err = 0.0; /* average error */
	vector<Point2f> image_points2; /* Save the recalculated projection point */
	cout << "\tCalibration error for each image:\n";
	fout << "Calibration error for each image:\n";
	for (size_t i = 0; i < image_count; i++)
	{
		vector<Point3f> tempPointSet = object_points[i];
		/* Through the obtained internal and external camera parameters, the three-dimensional points
		in the space are re-projected to obtain a new projection point. */
		projectPoints(tempPointSet, rvecsMat[i], tvecsMat[i], cameraMatrix, distCoeffs, image_points2);
		/* Calculate the error between the new projection point and the old projection point*/
		vector<Point2f> tempImagePoint = image_points_seq[i];
		Mat tempImagePointMat = Mat(1, tempImagePoint.size(), CV_32FC2);
		Mat image_points2Mat = Mat(1, image_points2.size(), CV_32FC2);
		for (int j = 0; j < tempImagePoint.size(); j++)
		{
			image_points2Mat.at<Vec2f>(0, j) = Vec2f(image_points2[j].x, image_points2[j].y);
			tempImagePointMat.at<Vec2f>(0, j) = Vec2f(tempImagePoint[j].x, tempImagePoint[j].y);
		}
		err = norm(image_points2Mat, tempImagePointMat, NORM_L2);
		total_err += err /= point_counts[i];
		std::cout << "The average error of the No." << i + 1 << "\t" << err << "pixels" << endl;
		fout << "The average error of the No." << i + 1 << "\t" << err << "\t" << "pixels" << endl;
	}
	std::cout << "Overall average error:" << total_err / image_count << "pixels" << endl;
	fout << "Overall average error:" << total_err / image_count << "pixels" << endl;
	std::cout << "completed!" << endl;


	return 0;
}

