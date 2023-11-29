/* -----------------------------------------------------*/
/* Fichier  : LaneChangeDetector.cpp                    */
/* Role     : Détection de changement de ligne sur route*/
/* -----------------------------------------------------*/
#include <opencv2/opencv.hpp>
#include <iostream>
#include <stdio.h>

using namespace cv;
using namespace std;

int main( int argc, char** argv )
{
	Mat frame, modifiedFrame;
	if (argc != 2)
	{
		cout <<"LaneChangeDetector video.XXX"<<endl;
		cout <<"video.XXX : video a traiter "<<endl;
		exit(0);
	}
	VideoCapture cap(argv[1]);
	
	int fourcc = cv::VideoWriter::fourcc('a','v','c','1'); // codec vidéo
	double fps = 30.0;
	Size frameSize(640, 480);
	VideoWriter writer("../res/output.mp4", fourcc, fps, frameSize, true);

	if(!cap.isOpened())
	{
		return -1;
	}
	// Display Image
	namedWindow("video", WINDOW_AUTOSIZE);
  
	for(;;) // Boucle d'Acquisition
	{
		cap >> frame;     
		if (frame.empty()) break;

		cvtColor(frame, modifiedFrame, cv::COLOR_BGR2HSV);
		Mat whiteMask;
		inRange(modifiedFrame, Scalar(0, 0, 200), Scalar(255, 20, 255), whiteMask); // pour les lignes blanches
		Mat yellowMask;
		inRange(modifiedFrame, Scalar(20, 80, 80), Scalar(30, 255, 255), yellowMask); // pour les lignes jaunes
		Mat combinedMask;
		bitwise_or(whiteMask, yellowMask, combinedMask);
		
		// masquage d'une grande partie de la video pour ne voir que la route
		// dimensions de l'image
		int height = combinedMask.rows;
		int width = combinedMask.cols;

		// points du polygone
		vector<Point> vertices;
		vertices.push_back(Point(0, height*0.6));
		vertices.push_back(Point(0, height*0.4));
		vertices.push_back(Point(width * 0.40, height * 0.1)); // changer coef si besoin
		vertices.push_back(Point(width * 0.70, height * 0.1));
		vertices.push_back(Point(width, height*0.4));
		vertices.push_back(Point(width, height*0.6));
		

		Mat mask = Mat::zeros(combinedMask.size(), combinedMask.type());

		// créer un polygone à partir des sommets
		vector<vector<Point>> pts = {vertices};
		fillPoly(mask, pts, Scalar(255)); // remplir le polygone avec du blanc

		bitwise_and(combinedMask, mask, combinedMask);
		
		Canny(combinedMask, combinedMask, 100, 200); //détection des bords de la video
		
		//détection de lignes avec Hough
		int r = 1; //1
		int threshold = 20; //20
		int min_line_len = 100; //100
		int max_line_gap = 400; //400

		vector<Vec4i> linesP;
		HoughLinesP(combinedMask, linesP, r, CV_PI/180, threshold, min_line_len, max_line_gap);
		vector<Vec4i> leftLines;
		vector<Vec4i> rightLines;

		double avg_LSlope = 0.0, avg_RSlope = 0.0;
		int L_count = 0, R_count = 0;
		Point avg_leftPoint = Point(0, 0);
		Point avg_rightPoint = Point(0, 0);

		for (size_t i = 0; i < linesP.size(); i++)
		{
			Vec4i l = linesP[i];
			double slope = ((double)l[3] - l[1]) / ((double)l[2] - l[0]);

			if (slope <= 0)
			{
				leftLines.push_back(l);
				avg_LSlope += slope;
				avg_leftPoint += Point((l[0]+l[2])/2, (l[1]+l[3])/2); // ajout du point moyen de la ligne
				L_count++;
			}
			else 
			{
				rightLines.push_back(l);
				avg_RSlope += slope;
				avg_rightPoint += Point((l[0]+l[2])/2, (l[1]+l[3])/2); // ajout du point moyen de la ligne
				R_count++;
			}
		}
		
		if (L_count != 0)
		{
			avg_LSlope /= L_count;
			avg_leftPoint /= L_count;
			// Calcul des points de début et de fin pour les lignes moyennes
			Point left_line_p1 = Point(avg_leftPoint.x - 1000, avg_leftPoint.y - 1000 * avg_LSlope);
			Point left_line_p2 = Point(avg_leftPoint.x + 1000, avg_leftPoint.y + 1000 * avg_LSlope);
			line(frame, left_line_p1, left_line_p2, Scalar(0,255,0), 2, LINE_AA);
			
		}
		if (R_count != 0)
		{
			avg_RSlope /= R_count;
			avg_rightPoint /= R_count;
			Point right_line_p1 = Point(avg_rightPoint.x - 1000, avg_rightPoint.y - 1000 * avg_RSlope);
			Point right_line_p2 = Point(avg_rightPoint.x + 1000, avg_rightPoint.y + 1000 * avg_RSlope);
			line(frame, right_line_p1, right_line_p2, Scalar(255,0,0), 2, LINE_AA);
		}
		
		if (avg_RSlope > 0.9 || avg_LSlope < -0.9)
		{
			putText(frame, "Franchissement de ligne", Point(50, 50), FONT_HERSHEY_SIMPLEX, 0.7, Scalar(0, 0, 255), 2);
		}
		imshow("video", frame);
		writer.write(frame);
		if(waitKey(1) >= 0) break;
	}
	// Wait until user exits the program
	waitKey(0);
	return 0;
}

