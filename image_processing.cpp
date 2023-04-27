

#include "image_processing.h"


CImageProcessor::CImageProcessor() {
	for(uint32 i=0; i<3; i++) {
		/* index 0 is 3 channels and indicies 1/2 are 1 channel deep */
		m_proc_image[i] = new cv::Mat();
	}
}

CImageProcessor::~CImageProcessor() {
	for(uint32 i=0; i<3; i++) {
		delete m_proc_image[i];
	}
}

cv::Mat* CImageProcessor::GetProcImage(uint32 i) {
	if(2 < i) {
		i = 2;
	}
	return m_proc_image[i];
}


static int counter;

int CImageProcessor::DoProcess(cv::Mat* image) {
	if(!image) return(EINVALID_PARAMETER);	
	static cv::Mat grayImage, mPrevImage, ResultImage, mBackgroundImage, mBackgroundImage2; // should be put into class in image_processing.h
	static bool init = false;
	int threshold = 35;
  double alpha = 0.5;

	if(image->channels() > 1) {
		cv::cvtColor( *image, grayImage, cv::COLOR_RGB2GRAY);
	} else {
		grayImage = *image;
	}

	if(!init) {
		mBackgroundImage2 = grayImage.clone();
		mBackgroundImage = grayImage.clone();
		//mBackgroundImage = cv::imread("dx.png");
		//cv::cvtColor( mBackgroundImage, mBackgroundImage, cv::COLOR_RGB2GRAY);
		init = true;
	}
	cv::addWeighted(mBackgroundImage2, alpha, grayImage, 1-alpha, 0, mBackgroundImage2);

	counter = (counter + 1) % 200; // after 200 images, apply a new background image (helps with testing out stuff at different locations)
	if(counter == 0) {
		mBackgroundImage = mBackgroundImage2.clone();
		printf("Background Update\n");
	}

	*m_proc_image[1] = mBackgroundImage.clone();

	ResultImage = image->clone();

	//if(mBackgroundImage.size() != cv::Size()) {
		// calculate absolute difference + thresholding
		cv::Mat diffImage, binaryImage;
		cv::absdiff(mBackgroundImage, grayImage, diffImage);
		cv::threshold(diffImage, binaryImage, threshold, 255, cv::THRESH_BINARY);

		// close gaps
		cv::Mat kernel = cv::Mat::ones(5,5, CV_8UC1);
		cv::morphologyEx(binaryImage, binaryImage, cv::MORPH_CLOSE, kernel);


		std::vector<std::vector<cv::Point>> contours;
 		std::vector<cv::Vec4i> hierarchy;

		cv::findContours(binaryImage, contours, hierarchy, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

		for (unsigned int idx = 0 ; idx < contours.size() ; idx++) {
			// area
			double area = cv::contourArea(contours[idx]);

			// bounding rectangle
			cv::Rect rect = cv::boundingRect(contours[idx]);

			// center of gravity
			cv::Moments moment = cv::moments(contours[idx]);
			double cx = moment.m10 / moment.m00;
			double cy = moment.m01 / moment.m00;

			//to draw counter to index idx in image
			if(area > 200.0) {
				cv::drawContours(ResultImage, contours, idx, cv::Scalar(255), 1, 8);
				cv::rectangle(ResultImage, rect, cv::Scalar(255,0,0));
				cv::Point2d cent(cx, cy);
				cv::circle(ResultImage, cent, 5, cv::Scalar(128,0,0), -1);
			}
		}
		// Labeling
		//cv::Mat stats, centroids, labelImage;
		//cv::connectedComponentsWithStats(binaryImage, labelImage, stats, centroids);

		// draw boxes into image
		//for(int i = 1 ; i < stats.rows ; i++) {
		//	int topLeftx = stats.at<int>(i,0);
		//	int topLefty = stats.at<int>(i,1);
		//	int width 	 = stats.at<int>(i,2);
		//	int height   = stats.at<int>(i,3);
		//	int area     = stats.at<int>(i,4);
		//	double cx = centroids.at<double>(i,0);
		//	double cy = centroids.at<double>(i,1);
		//	if(area > 100) {
		//		cv::Rect rect(topLeftx, topLefty, width, height);
		//		cv::rectangle(ResultImage, rect, cv::Scalar(255,0,0));
		//		cv::Point2d cent(cx, cy);
		//		cv::circle(ResultImage, cent, 5, cv::Scalar(128,0,0), -1);
		//	}
		//}
	//}
	//mPrevImage = grayImage.clone();
	*m_proc_image[0] = ResultImage.clone();
	*m_proc_image[1] = diffImage.clone();
	*m_proc_image[2] = mBackgroundImage2.clone();


  //cv::imwrite("dx.png", mBackgroundImage);
      //  cv::imwrite("dy.png", *m_proc_image[1]);

	return(SUCCESS);
}









