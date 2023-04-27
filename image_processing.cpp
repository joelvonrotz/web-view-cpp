

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

int CImageProcessor::DoProcess(cv::Mat* image) {
	int threshold = 63;
	if(!image) return(EINVALID_PARAMETER);	
	static cv::Mat grayImage, mPrevImage, ResultImage;

	if(image->channels() > 1) {
		cv::cvtColor( *image, grayImage, cv::COLOR_RGB2GRAY);
	} else {
		grayImage = *image;
	}
	ResultImage = image->clone();

	if(mPrevImage.size() != cv::Size()) {
		// calculate absolute difference + thresholding
		cv::Mat diffImage, binaryImage;
		cv::absdiff(mPrevImage, grayImage, diffImage);
		cv::threshold(diffImage, binaryImage, threshold, 255, cv::THRESH_BINARY);

		// close gaps
		cv::Mat kernel = cv::Mat::ones(5,5, CV_8UC1);
		cv::morphologyEx(binaryImage, binaryImage, cv::MORPH_CLOSE, kernel);

		// Labeling
		cv::Mat stats, centroids, labelImage;
		cv::connectedComponentsWithStats(binaryImage, labelImage, stats, centroids);

		// draw boxes into image
		for(int i = 1 ; i < stats.rows ; i++) {
			int topLeftx = stats.at<int>(i,0);
			int topLefty = stats.at<int>(i,1);
			int width 	 = stats.at<int>(i,2);
			int height   = stats.at<int>(i,3);

			int area     = stats.at<int>(i,4);

			double cx = centroids.at<double>(i,0);
			double cy = centroids.at<double>(i,1);

			cv::Rect rect(topLeftx, topLefty, width, height);
			cv::rectangle(ResultImage, rect, cv::Scalar(255,0,0));
			cv::Point2d cent(cx, cy);
			cv::circle(ResultImage, cent, 5, cv::Scalar(128,0,0), -1);
		}
	}
	mPrevImage = grayImage.clone();
	*m_proc_image[0] = ResultImage.clone();

        
      //  cv::imwrite("dx.png", *m_proc_image[0]);
      //  cv::imwrite("dy.png", *m_proc_image[1]);

	return(SUCCESS);
}









