
#undef UNICODE

#define WIN32_LEAN_AND_MEAN
#include <iostream>

#include <windows.h>
#include <strsafe.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>

#include "json.h"

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/video/video.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/contrib/contrib.hpp"
#include "opencv2/core/core_c.h"
#include "opencv2/highgui/highgui_c.h"
#include "opencv2/imgproc/imgproc_c.h"
#include <opencv2/video/background_segm.hpp>

// Need to link with Ws2_32.lib
#pragma comment (lib, "Ws2_32.lib")
// #pragma comment (lib, "Mswsock.lib")

#include "parameters.cpp"

using namespace Json;
using namespace std;
using namespace cv;

static Size block_size(16, 16);

static int LK_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket);
static int FB_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket);
static int Dense_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket);
static void prelearn_bg_sub(CvCapture* capture, BackgroundSubtractor* bg_sub);


//routine_handler(file_name, bg_sub_flag, flow_type_flag, classifier_type_flag)
static int routine_handler(const char* file_name, int bg_sub_flag, int flow_type_flag, SOCKET informer_socket)
{
	CvCapture* capture = cvCaptureFromFile(file_name); //apparently is also capable of catching streams
	BackgroundSubtractor* bg_sub;

	if (bg_sub_flag > 0) bg_sub = new BackgroundSubtractorMOG2();// mog2_history, varThreshold, bShadowDetection);
	else bg_sub = new BackgroundSubtractorMOG(mog_history, nmixtures, backgroundRatio, noiseSigma);
	try
	{
		prelearn_bg_sub(capture, bg_sub);
	}
	catch (exception e)
	{
		if (e.what() == "Not enough frames in the video.")
		{
			//inform_not_enough_frames
		}
	}
	
	switch (flow_type_flag)
	{
	case 0: return LK_routine(capture, bg_sub, informer_socket); 
	case 1: return FB_routine(capture, bg_sub, informer_socket); 
	case 2: return Dense_routine(capture, bg_sub, informer_socket); 
	}
}

static void prelearn_bg_sub(CvCapture* capture, BackgroundSubtractor* bg_sub) //works
{
	Mat frame;

	Mat placeholder_mask_mat;
	for (int counter = 0; counter < LEARNING_FRAMES; counter++)
	{
		if (cvGrabFrame(capture) == 0) throw exception("Not enough frames in the video.");
		frame = cvRetrieveFrame(capture);
		bg_sub->operator()(frame, placeholder_mask_mat, learning_rate);
	}
}

static int sparse_flow_analyzer(Mat current_gray_frame, Mat united_mask, SOCKET informer_socket, CvPoint2D32f* cornersA, CvPoint2D32f* cornersB, int corner_count, char* features_found, float* feature_errors)
{
	int resval = 0;
	Mat dh = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);
	Mat dv = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);
	Mat dd = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);

	for (int x = 0; x < current_gray_frame.rows - 1; x++)
	{
		for (int y = 0; y < current_gray_frame.cols - 1; y++)
		{
			dh.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) + (float)current_gray_frame.at<uchar>(x + 1, y) - (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
			dv.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) + (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y) - (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
			dd.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) - (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y) + (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
		}
	}

	double d_max = 0, d_min = 0;
	minMaxLoc(dh, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dh = dh*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dh = dh * 255;
	}
	minMaxLoc(dv, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dv = dv*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dv = dv * 255;
	}
	minMaxLoc(dd, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dd = dd*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dd = dd * 255;
	}

	Mat tmp;
	bitwise_not(united_mask, tmp);
	double current_color_mean = mean(current_gray_frame, tmp)[0]; //average of stationary pixels
	resize(tmp.t(), tmp, Size(tmp.cols - 1, tmp.rows - 1));
	double current_h_mean = mean(dh, tmp)[0];
	double current_v_mean = mean(dv, tmp)[0];
	double current_d_mean = mean(dd, tmp)[0];
	bitwise_not(tmp, tmp);

	Mat video_frame;
	cvtColor(current_gray_frame, video_frame, CV_GRAY2BGR);
	Mat video_dh;
	cvtColor(dh, video_dh, CV_GRAY2BGR);
	Mat video_dv;
	cvtColor(dv, video_dv, CV_GRAY2BGR);
	Mat video_dd;
	cvtColor(dd, video_dd, CV_GRAY2BGR);

	for (int y = 0; y < current_gray_frame.cols; y += block_size.height)
	{
		for (int x = 0; x < current_gray_frame.rows; x += block_size.width)
		{
			Rect rect(y, x, block_size.width, block_size.height);

			double local_color_mean;
			double local_h_mean;
			double local_v_mean;
			double local_d_mean;

			try
			{
				local_color_mean = mean(Mat(current_gray_frame, rect), Mat(united_mask, rect))[0];
				local_h_mean = mean(Mat(dh, rect), Mat(tmp, rect))[0];
				local_v_mean = mean(Mat(dv, rect), Mat(tmp, rect))[0];
				local_d_mean = mean(Mat(dd, rect), Mat(tmp, rect))[0];
			}
			catch (Exception e)
			{
				break;
			}

			double current_local_color_deviation = (local_color_mean - current_color_mean)*(local_color_mean - current_color_mean);
			double current_local_h_deviation = (local_h_mean - current_h_mean)*(local_h_mean - current_h_mean);
			double current_local_v_deviation = (local_v_mean - current_v_mean)*(local_v_mean - current_v_mean);
			double current_local_d_deviation = (local_d_mean - current_d_mean)*(local_d_mean - current_d_mean);

			double cumulative_color_deviation = 0.0;
			//double cumulative_angle_deviation = 0.0;
			//double cumulative_travel_deviation = 0.0;
			double cumulative_h_deviation = 0.0;
			double cumulative_v_deviation = 0.0;
			double cumulative_d_deviation = 0.0;

			double mask_mean = mean(Mat(united_mask, rect))[0];
			//float avg_r[subblocks_x_amount][subblocks_y_amount];
			//float avg_phi[subblocks_x_amount][subblocks_y_amount];


			for (int section_x = 0; section_x < subblocks_x_amount; section_x++)
			{
				for (int section_y = 0; section_y < subblocks_y_amount; section_y++)
				{
					//float avg_x = 0;
					//float avg_y = 0;

					for (int i = section_x*block_size.height / subblocks_x_amount; i < (1 + section_x)*block_size.height / subblocks_x_amount; i++)
					{
						for (int j = section_y*block_size.width / subblocks_y_amount; j < (1 + section_y)*block_size.width / subblocks_y_amount; j++)
						{
							cumulative_color_deviation += ((double)current_gray_frame.at<uchar>(x + i, y + j) - local_color_mean)*((double)current_gray_frame.at<uchar>(x + i, y + j) - local_color_mean);
							cumulative_h_deviation += (local_h_mean - (double)dh.at<float>(x + i, y + j))*(local_h_mean - (double)dh.at<float>(x + i, y + j));
							cumulative_v_deviation += (local_v_mean - (double)dv.at<float>(x + i, y + j))*(local_v_mean - (double)dv.at<float>(x + i, y + j));
							cumulative_d_deviation += (local_d_mean - (double)dd.at<float>(x + i, y + j))*(local_d_mean - (double)dd.at<float>(x + i, y + j));
						}
					}
				}
			}

			cumulative_color_deviation = sqrt(cumulative_color_deviation) / block_size.area();
			cumulative_h_deviation = sqrt(cumulative_h_deviation) / block_size.area();
			cumulative_v_deviation = sqrt(cumulative_v_deviation) / block_size.area();
			cumulative_d_deviation = sqrt(cumulative_d_deviation) / block_size.area();

			current_local_color_deviation = sqrt(current_local_color_deviation);
			current_local_h_deviation = sqrt(current_local_h_deviation);
			current_local_v_deviation = sqrt(current_local_v_deviation);
			current_local_d_deviation = sqrt(current_local_d_deviation);

			if (cumulative_color_deviation_min < cumulative_color_deviation &&
				cumulative_color_deviation < cumulative_color_deviation_max &&

				current_local_color_deviation_min < current_local_color_deviation &&
				current_local_color_deviation < current_local_color_deviation_max &&

				current_local_h_deviation_min < current_local_h_deviation &&
				current_local_h_deviation < current_local_h_deviation_max &&

				current_local_v_deviation_min < current_local_v_deviation &&
				current_local_v_deviation < current_local_v_deviation_max &&

				current_local_d_deviation_min < current_local_d_deviation &&
				current_local_d_deviation < current_local_d_deviation_max &&

				cumulative_h_deviation_min < cumulative_h_deviation &&
				cumulative_h_deviation < cumulative_h_deviation_max &&

				cumulative_v_deviation_min < cumulative_v_deviation &&
				cumulative_v_deviation < cumulative_v_deviation_max &&

				cumulative_d_deviation_min < cumulative_d_deviation &&
				cumulative_d_deviation < cumulative_d_deviation_max &&

				local_h_mean_min < local_h_mean &&
				local_h_mean < local_h_mean_max &&

				local_v_mean_min < local_v_mean &&
				local_v_mean < local_v_mean_max &&

				local_d_mean_min < local_d_mean &&
				local_d_mean < local_d_mean_max &&

				mask_mean_min < mask_mean &&
				mask_mean < mask_mean_max)
			{
				rectangle(video_frame, rect, CV_RGB(255, 0, 0));
				rectangle(video_dh, rect, CV_RGB(255, 0, 0));
				rectangle(video_dv, rect, CV_RGB(255, 0, 0));
				rectangle(video_dd, rect, CV_RGB(255, 0, 0));
				//arrowedLine(video_frame,
				//	Point(y + block_size.width / 2, x + block_size.height / 2),
				//	Point(cvRound(y + block_size.width / 2 + travel_mean*sin(angle_mean)), cvRound(x + block_size.height / 2 + travel_mean*cos(angle_mean))), CV_RGB(0, 255, 0));
				double avg_dx = 0;
				double avg_dy = 0;
				int features_in_block = 0;
				for (int i = 0; i < corner_count; ++i)
				{
					if (features_found[i] == 0 || feature_errors[i] > MAX_COUNT)
						continue;
					if (rect.contains(cornersA[i]))
					{
						features_in_block++;
						avg_dx += cornersA[i].x - cornersB[i].x;
						avg_dy += cornersA[i].y - cornersB[i].y;
					}
					if (rect.contains(cornersB[i]))
					{
						features_in_block++;
						avg_dx += cornersA[i].x - cornersB[i].x;
						avg_dy += cornersA[i].y - cornersB[i].y;
					}
				}
				if (features_in_block > 0)
				{
					arrowedLine(video_frame,
						Point(y + block_size.width / 2, x + block_size.height / 2),
						Point(cvRound(y + block_size.width / 2 - avg_dx / features_in_block), cvRound(x + block_size.height / 2 - avg_dy / features_in_block)), CV_RGB(0, 255, 0));
				}
				if (!resval)
				{
					resval = 1;
					int iSendResult = send(informer_socket, SMOKE_ALARM, strlen(SMOKE_ALARM), 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						system("pause");
						closesocket(informer_socket);
						WSACleanup();
						return 1;
					}
				}
			}
		}
	}
	//imshow("dh", video_dh);
	//imshow("dv", video_dv);
	//imshow("dd", video_dd);
	imshow("Video", video_frame);
	if (waitKey(1000 / FRAMERATE) >= 0)
		return -1;
	return resval;
}

static int LK_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket)
{
	int resval = 0;
	int smoke = 0;
	cvNamedWindow("Video");
	//cvNamedWindow("mask");//*/
	//image class           
	IplImage* frame = 0;

	//T, T-1 image     
	IplImage* current_frame = 0;
	IplImage* previous_frame = 0;

	//supposedly masks
	Mat current_mask;
	Mat previous_mask;
	Mat united_mask;
	IplImage* united_mask_image;

	//Optical Image     
	IplImage * current_gray_frame = 0;
	IplImage * previous_gray_frame = 0;
	IplImage * eig_image = 0;
	IplImage * tmp_image = 0;

	int corner_count = MAX_COUNT;
	CvPoint2D32f* current_corners = new CvPoint2D32f[MAX_COUNT];
	CvPoint2D32f* previous_corners = new CvPoint2D32f[MAX_COUNT];

	CvSize pyr_sz;
	CvSize img_sz;
	int win_size = 20;

	IplImage* current_pyramid = 0;
	IplImage* previous_pyramid = 0;

	char features_found[MAX_COUNT];
	float feature_errors[MAX_COUNT];

	//Routine Start     
	if (cvGrabFrame(capture) == 0) throw exception("The file ended unexpectedly.");
	frame = cvRetrieveFrame(capture);

	img_sz = cvSize(frame->width, frame->height);
	current_frame = cvCreateImage(img_sz, frame->depth, frame->nChannels);
	memcpy(current_frame->imageData, frame->imageData, sizeof(char)*frame->imageSize);
	previous_frame = cvCreateImage(img_sz, frame->depth, frame->nChannels);
	bg_sub->operator()(Mat(current_frame), previous_mask, learning_rate);
	eig_image = cvCreateImage(img_sz, frame->depth, frame->nChannels);
	tmp_image = cvCreateImage(img_sz, frame->depth, frame->nChannels);

	current_gray_frame = cvCreateImage(img_sz, IPL_DEPTH_8U, 1);
	previous_gray_frame = cvCreateImage(img_sz, IPL_DEPTH_8U, 1);
	pyr_sz = cvSize(current_gray_frame->width + 8, previous_gray_frame->height / 3);
	current_pyramid = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
	previous_pyramid = cvCreateImage(pyr_sz, IPL_DEPTH_32F, 1);
	while (true) {


		//capture a frame form cam        
		if (cvGrabFrame(capture) == 0)
			break;
		
		//copy to image class     
		memcpy(previous_frame->imageData, current_frame->imageData, sizeof(char)*frame->imageSize);
		frame = cvRetrieveFrame(capture);
		memcpy(current_frame->imageData, frame->imageData, sizeof(char)*frame->imageSize);


		bg_sub->operator()(Mat(current_frame), current_mask, learning_rate); 
		bitwise_or(current_mask, previous_mask, united_mask);
		
		united_mask_image = cvCreateImage(cvSize(united_mask.cols, united_mask.rows), IPL_DEPTH_8U, 1);
		united_mask_image = cvCloneImage(&(IplImage)united_mask);

		//RGB to Gray for Optical Flow     
		cvCvtColor(current_frame, current_gray_frame, CV_BGR2GRAY);
		cvCvtColor(previous_frame, previous_gray_frame, CV_BGR2GRAY);

		//     
		cvGoodFeaturesToTrack(current_gray_frame, eig_image, tmp_image, current_corners, &corner_count, 0.01, 5.0, united_mask_image, 3, 0, 0.04);
		cvFindCornerSubPix(current_gray_frame, current_corners, corner_count, cvSize(win_size, win_size), cvSize(-1, -1), cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.03));

		
		cvCalcOpticalFlowPyrLK(current_gray_frame, previous_gray_frame, current_pyramid, previous_pyramid, current_corners, previous_corners, corner_count, cvSize(win_size, win_size), 5, features_found, feature_errors, cvTermCriteria(CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 20, 0.3), 0);

		//TODO:implement classifier

		smoke = sparse_flow_analyzer(current_gray_frame, united_mask, informer_socket, current_corners, previous_corners, corner_count, features_found, feature_errors);
		if (smoke < 0) break;
		if (smoke > 0) resval = 1;

		previous_mask = current_mask.clone();
	}
	cvDestroyWindow("Video");
	//cvDestroyWindow("mask");//*/
	cvReleaseImage(&previous_frame);
	//////////////////////////////////////////////////////////////////////////     
	cvReleaseImage(&current_gray_frame);
	cvReleaseImage(&previous_gray_frame);
	cvReleaseImage(&eig_image);
	cvReleaseImage(&tmp_image);
	delete current_corners;
	delete previous_corners;
	cvReleaseImage(&current_pyramid);
	cvReleaseImage(&previous_pyramid);
	return resval;
}

static int dense_flow_analyzer(Mat current_gray_frame, Mat united_mask, Mat flow, SOCKET informer_socket)
{
	int resval = 0; 
	Mat dh = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);
	Mat dv = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);
	Mat dd = Mat(current_gray_frame.rows - 1, current_gray_frame.cols - 1, CV_32FC1);

	for (int x = 0; x < current_gray_frame.rows - 1; x++)
	{
		for (int y = 0; y < current_gray_frame.cols - 1; y++)
		{
			dh.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) + (float)current_gray_frame.at<uchar>(x + 1, y) - (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
			dv.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) + (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y) - (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
			dd.at<float>(x, y) = ((float)current_gray_frame.at<uchar>(x, y) - (float)current_gray_frame.at<uchar>(x, y + 1) - (float)current_gray_frame.at<uchar>(x + 1, y) + (float)current_gray_frame.at<uchar>(x + 1, y + 1))*0.5;
		}
	}

	double d_max = 0, d_min = 0;
	minMaxLoc(dh, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dh = dh*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dh = dh * 255;
	}
	minMaxLoc(dv, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dv = dv*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dv = dv * 255;
	}
	minMaxLoc(dd, &d_min, &d_max);
	if ((d_max - d_min)>0)
	{
		dd = dd*(1.0 / (d_max - d_min)) - d_min / (d_max - d_min);
		//dd = dd * 255;
	}

	Mat tmp;
	bitwise_not(united_mask, tmp);
	double current_color_mean = mean(current_gray_frame, tmp)[0]; //average of stationary pixels
	resize(tmp.t(), tmp, Size(tmp.cols - 1, tmp.rows - 1));
	double current_h_mean = mean(dh, tmp)[0];
	double current_v_mean = mean(dv, tmp)[0];
	double current_d_mean = mean(dd, tmp)[0];
	bitwise_not(tmp, tmp);

	Mat video_frame;
	cvtColor(current_gray_frame, video_frame, CV_GRAY2BGR);
	Mat video_dh;
	cvtColor(dh, video_dh, CV_GRAY2BGR);
	Mat video_dv;
	cvtColor(dv, video_dv, CV_GRAY2BGR);
	Mat video_dd;
	cvtColor(dd, video_dd, CV_GRAY2BGR);

	//cout << "current_color_mean " << current_color_mean << " current_h_mean " << current_h_mean << " current_v_mean " << current_v_mean << " current_d_mean " << current_d_mean << endl;

	for (int y = 0; y < current_gray_frame.cols; y += block_size.height)
	{
		for (int x = 0; x < current_gray_frame.rows; x += block_size.width)
		{
			Rect rect(y, x, block_size.width, block_size.height);

			double local_color_mean;
			double local_h_mean;
			double local_v_mean;
			double local_d_mean;

			try
			{
				local_color_mean = mean(Mat(current_gray_frame, rect), Mat(united_mask, rect))[0];
				local_h_mean = mean(Mat(dh, rect), Mat(tmp, rect))[0];
				local_v_mean = mean(Mat(dv, rect), Mat(tmp, rect))[0];
				local_d_mean = mean(Mat(dd, rect), Mat(tmp, rect))[0];
			}
			catch (Exception e)
			{
				break;
			}

			//cout << " local_color_mean " << local_color_mean << " local_h_mean " << local_h_mean << " local_v_mean " << local_v_mean << " local_d_mean " << local_d_mean << endl;

			double current_local_color_deviation = (local_color_mean - current_color_mean)*(local_color_mean - current_color_mean);
			double current_local_h_deviation = (local_h_mean - current_h_mean)*(local_h_mean - current_h_mean);
			double current_local_v_deviation = (local_v_mean - current_v_mean)*(local_v_mean - current_v_mean);
			double current_local_d_deviation = (local_d_mean - current_d_mean)*(local_d_mean - current_d_mean);

			double cumulative_color_deviation = 0.0;
			double cumulative_angle_deviation = 0.0;
			double cumulative_travel_deviation = 0.0;
			double cumulative_h_deviation = 0.0;
			double cumulative_v_deviation = 0.0;
			double cumulative_d_deviation = 0.0;

			double mask_mean = mean(Mat(united_mask, rect))[0];
			float avg_r[subblocks_x_amount][subblocks_y_amount];
			float avg_phi[subblocks_x_amount][subblocks_y_amount];


			for (int section_x = 0; section_x < subblocks_x_amount; section_x++)
			{
				for (int section_y = 0; section_y < subblocks_y_amount; section_y++)
				{
					float avg_x = 0;
					float avg_y = 0;

					for (int i = section_x*block_size.height / subblocks_x_amount; i < (1 + section_x)*block_size.height / subblocks_x_amount; i++)
					{
						for (int j = section_y*block_size.width / subblocks_y_amount; j < (1 + section_y)*block_size.width / subblocks_y_amount; j++)
						{
							cumulative_color_deviation += ((double)current_gray_frame.at<uchar>(x + i, y + j) - local_color_mean)*((double)current_gray_frame.at<uchar>(x + i, y + j) - local_color_mean);
							cumulative_h_deviation += (local_h_mean - (double)dh.at<float>(x + i, y + j))*(local_h_mean - (double)dh.at<float>(x + i, y + j));
							cumulative_v_deviation += (local_v_mean - (double)dv.at<float>(x + i, y + j))*(local_v_mean - (double)dv.at<float>(x + i, y + j));
							cumulative_d_deviation += (local_d_mean - (double)dd.at<float>(x + i, y + j))*(local_d_mean - (double)dd.at<float>(x + i, y + j));

							const Point2f& fxy = flow.at<Point2f>(x + i, y + j);
							avg_x += fxy.x;
							avg_y += fxy.y;
						}
					}
					avg_x /= (block_size.height / 2)*(block_size.height / 2);
					avg_y /= (block_size.width / 2)*(block_size.width / 2);
					avg_r[section_x][section_y] = sqrt(avg_x*avg_x + avg_y*avg_y);
					avg_phi[section_x][section_y] = acos(avg_x / avg_r[section_x][section_y]);
				}
			}

			double angle_mean = 0;
			double travel_mean = 0;
			for (int section_x = 0; section_x < subblocks_x_amount; section_x++)
			{
				for (int section_y = 0; section_y < subblocks_y_amount; section_y++)
				{
					angle_mean += avg_phi[section_x][section_y];
					travel_mean += avg_r[section_x][section_y];
				}
			}
			for (int section_x = 0; section_x < subblocks_x_amount; section_x++)
			{
				for (int section_y = 0; section_y < subblocks_y_amount; section_y++)
				{
					cumulative_angle_deviation += (angle_mean - avg_phi[section_x][section_y])*(angle_mean - avg_phi[section_x][section_y]);
					cumulative_travel_deviation += (travel_mean - avg_r[section_x][section_y])*(travel_mean - avg_r[section_x][section_y]);
				}
			}

			cumulative_angle_deviation = sqrt(cumulative_angle_deviation) / (subblocks_x_amount*subblocks_y_amount);
			cumulative_travel_deviation = sqrt(cumulative_travel_deviation) / (subblocks_x_amount*subblocks_y_amount);

			cumulative_color_deviation = sqrt(cumulative_color_deviation) / block_size.area();
			cumulative_h_deviation = sqrt(cumulative_h_deviation) / block_size.area();
			cumulative_v_deviation = sqrt(cumulative_v_deviation) / block_size.area();
			cumulative_d_deviation = sqrt(cumulative_d_deviation) / block_size.area();

			current_local_color_deviation = sqrt(current_local_color_deviation);
			current_local_h_deviation = sqrt(current_local_h_deviation);
			current_local_v_deviation = sqrt(current_local_v_deviation);
			current_local_d_deviation = sqrt(current_local_d_deviation);

			if (cumulative_angle_deviation_min < cumulative_angle_deviation &&
				cumulative_angle_deviation < cumulative_angle_deviation_max &&

				cumulative_travel_deviation_min < cumulative_travel_deviation &&
				cumulative_travel_deviation < cumulative_travel_deviation_max &&

				angle_mean_min < angle_mean &&
				angle_mean < angle_mean_max &&

				travel_mean_min < travel_mean &&
				travel_mean < travel_mean_max &&

				cumulative_color_deviation_min < cumulative_color_deviation &&
				cumulative_color_deviation < cumulative_color_deviation_max &&

				current_local_color_deviation_min < current_local_color_deviation &&
				current_local_color_deviation < current_local_color_deviation_max &&

				current_local_h_deviation_min < current_local_h_deviation &&
				current_local_h_deviation < current_local_h_deviation_max &&

				current_local_v_deviation_min < current_local_v_deviation &&
				current_local_v_deviation < current_local_v_deviation_max &&

				current_local_d_deviation_min < current_local_d_deviation &&
				current_local_d_deviation < current_local_d_deviation_max &&

				cumulative_h_deviation_min < cumulative_h_deviation &&
				cumulative_h_deviation < cumulative_h_deviation_max &&

				cumulative_v_deviation_min < cumulative_v_deviation &&
				cumulative_v_deviation < cumulative_v_deviation_max &&

				cumulative_d_deviation_min < cumulative_d_deviation &&
				cumulative_d_deviation < cumulative_d_deviation_max &&

				local_h_mean_min < local_h_mean &&
				local_h_mean < local_h_mean_max &&

				local_v_mean_min < local_v_mean &&
				local_v_mean < local_v_mean_max &&

				local_d_mean_min < local_d_mean &&
				local_d_mean < local_d_mean_max &&

				mask_mean_min < mask_mean &&
				mask_mean < mask_mean_max)
			{
				rectangle(video_frame, rect, CV_RGB(255, 0, 0));
				rectangle(video_dh, rect, CV_RGB(255, 0, 0));
				rectangle(video_dv, rect, CV_RGB(255, 0, 0));
				rectangle(video_dd, rect, CV_RGB(255, 0, 0));
				arrowedLine(video_frame,
					Point(y + block_size.width / 2, x + block_size.height / 2),
					Point(cvRound(y + block_size.width / 2 + travel_mean*sin(angle_mean)), cvRound(x + block_size.height / 2 + travel_mean*cos(angle_mean))), CV_RGB(0, 255, 0));
				if (!resval)
				{
					resval = 1;
					int iSendResult = send(informer_socket, SMOKE_ALARM, strlen(SMOKE_ALARM), 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
						system("pause");
						closesocket(informer_socket);
						WSACleanup();
						return 1;
					}
				}
			}
		}
	}
	//imshow("dh", video_dh);
	//imshow("dv", video_dv);
	//imshow("dd", video_dd);
	imshow("Video", video_frame);
	if (waitKey(1000/FRAMERATE) >= 0)
		return -1;
	return resval;
}

static int FB_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket)
{
	int resval = 0;
	int smoke = 0;
	cvNamedWindow("Video");
	IplImage* frame = 0;

	//T, T-1 image     
	Mat previous_gray_frame;
	Mat current_gray_frame;

	//supposedly masks
	Mat current_mask;
	Mat previous_mask;
	Mat united_mask;
	IplImage* united_mask_image;

	//flow
	Mat flow;
	int downscale_coefficient = 1; //this means that flow will be calculated for every downscale_coefficient`th pixel. set it to 1 to make it for every pixel.

	if (cvGrabFrame(capture) == 0) throw exception("The file ended unexpectedly.");
	frame = cvRetrieveFrame(capture);
	resize(Mat(frame), previous_gray_frame, Size(frame->width / downscale_coefficient, frame->height / downscale_coefficient));
	cvtColor(previous_gray_frame, previous_gray_frame, CV_BGR2GRAY);

	bg_sub->operator()(Mat(frame), previous_mask, learning_rate);

	while (true) {


		//capture a frame form cam        
		if (cvGrabFrame(capture) == 0)
			break;

		frame = cvRetrieveFrame(capture);
		

		bg_sub->operator()(Mat(frame), current_mask, learning_rate);
		bitwise_or(current_mask, previous_mask, united_mask);

		//united_mask_image = cvCreateImage(cvSize(united_mask.cols, united_mask.rows), IPL_DEPTH_8U, 1);
		//united_mask_image = cvCloneImage(&(IplImage)united_mask);

		resize(Mat(frame), current_gray_frame, Size(frame->width / downscale_coefficient, frame->height / downscale_coefficient));
		cvtColor(current_gray_frame, current_gray_frame, CV_BGR2GRAY);

		calcOpticalFlowFarneback(previous_gray_frame, current_gray_frame, flow, 0.5, 3, 15, 3, 5, 1.2, 0);
		
		smoke = dense_flow_analyzer(current_gray_frame, united_mask, flow, informer_socket);
		if (smoke < 0) break;
		if (smoke > 0) resval = 1;

		previous_mask = current_mask.clone();
		current_gray_frame.copyTo(previous_gray_frame);
	}
	//cvDestroyWindow("dh");
	//cvDestroyWindow("dv");
	//cvDestroyWindow("dd");
	cvDestroyWindow("Video");
	return resval;
}

static int Dense_routine(CvCapture* capture, BackgroundSubtractor* bg_sub, SOCKET informer_socket)
{
	int resval = 0;
	int smoke = 0;
	cvNamedWindow("Video");
	IplImage* frame = 0;

	//T, T-1 image     
	Mat previous_gray_frame;
	Mat current_gray_frame;

	//supposedly masks
	Mat current_mask;
	Mat previous_mask;
	Mat united_mask;
	IplImage* united_mask_image;

	//flow
	Mat flow;
	int downscale_coefficient = 1;
	
	Ptr<DenseOpticalFlow> dual_tvl1_flow_calc = createOptFlow_DualTVL1();
	dual_tvl1_flow_calc->set("tau", tau);
	dual_tvl1_flow_calc->set("lambda", lambda);
	dual_tvl1_flow_calc->set("theta", theta);
	dual_tvl1_flow_calc->set("nscales", nscales);
	dual_tvl1_flow_calc->set("warps", warps);
	dual_tvl1_flow_calc->set("epsilon", epsilon);
	dual_tvl1_flow_calc->set("iterations", iterations);
	dual_tvl1_flow_calc->set("useInitialFlow", useInitialFlow);

	//Vector<Mat> blocks;
	
	if (cvGrabFrame(capture) == 0) throw exception("The file ended unexpectedly.");
	frame = cvRetrieveFrame(capture);
	resize(Mat(frame), previous_gray_frame, Size(frame->width / downscale_coefficient, frame->height / downscale_coefficient));
	cvtColor(previous_gray_frame, previous_gray_frame, CV_BGR2GRAY);

	bg_sub->operator()(Mat(frame), previous_mask, learning_rate);

	while (true) {


		//capture a frame form cam
		if (cvGrabFrame(capture) == 0)
			break;

		frame = cvRetrieveFrame(capture);

		bg_sub->operator()(Mat(frame), current_mask, learning_rate);
		bitwise_or(current_mask, previous_mask, united_mask);

		//united_mask_image = cvCreateImage(cvSize(united_mask.cols, united_mask.rows), IPL_DEPTH_8U, 1);
		//united_mask_image = cvCloneImage(&(IplImage)united_mask);

		resize(Mat(frame), current_gray_frame, Size(frame->width / downscale_coefficient, frame->height / downscale_coefficient));
		cvtColor(current_gray_frame, current_gray_frame, CV_BGR2GRAY);
		dual_tvl1_flow_calc->calc(previous_gray_frame, current_gray_frame, flow);

		smoke = dense_flow_analyzer(current_gray_frame, united_mask, flow, informer_socket);
		if (smoke < 0) break;
		if (smoke > 0) resval = 1;

		current_gray_frame.copyTo(previous_gray_frame);
		previous_mask = current_mask.clone();
	}
	//cvDestroyWindow("dh");
	//cvDestroyWindow("dv");
	//cvDestroyWindow("dd");
	cvDestroyWindow("Video");
	return resval;
}

static int myErrorHandler(int status, const char* func_name, const char* err_msg,
	const char* file_name, int line, void*)
{
	// Do whatever you want here
	return 0;
}