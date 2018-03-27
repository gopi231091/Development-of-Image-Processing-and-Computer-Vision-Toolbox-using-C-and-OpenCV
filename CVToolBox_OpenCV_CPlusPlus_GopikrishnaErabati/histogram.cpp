#include "histogram.h"

Histogram::Histogram()
{
    // Prepare arguments for 1D histogram
    histSize[0]= 256;
    hranges[0]= 0.0;
    hranges[1]= 255.0;
    ranges[0]= hranges;
    channels[0]= 0; // by default, we look at channel 0
}

// Computes the 1D histogram.
cv::MatND Histogram::getHistogram(const cv::Mat &image) {

    cv::MatND hist;

    // Compute histogram
    cv::calcHist(&image,
        1,			// histogram of 1 image only
        channels,	// the channel used
        cv::Mat(),	// no mask is used
        hist,		// the resulting histogram
        1,			// it is a 1D histogram
        histSize,	// number of bins
        ranges		// pixel value range
    );

    return hist;
}

// Computes the 1D histogram and returns an image of it.
cv::Mat Histogram::getHistogramImage(const cv::Mat &image){

    // Compute histogram first
    hist= getHistogram(image);

    // Get min and max bin values
    maxVal=0;
    minVal=0;
    cv::minMaxLoc(hist, &minVal, &maxVal, 0, 0);

    // Image on which to display histogram
    cv::Mat histImg(histSize[0], histSize[0], CV_8U,cv::Scalar(255));

    // set highest point at 90% of nbins
    int hpt = static_cast<int>(0.9*histSize[0]);

    // Draw vertical line for each bin
    for( int h = 0; h < histSize[0]; h++ ) {

        float binVal = hist.at<float>(h);
        int intensity = static_cast<int>(binVal*hpt/maxVal);
        cv::line(histImg,cv::Point(h,histSize[0]),cv::Point(h,histSize[0]-intensity),cv::Scalar::all(0));
    }

    return histImg;
}

// Equalizes the source image.
cv::Mat Histogram::equalize(const cv::Mat &image) {

    cv::Mat result;
    cv::equalizeHist(image,result);

    return result;
}

// Stretches the source image.
cv::Mat Histogram::stretch(const cv::Mat &image, int minValue) {

    // Compute histogram first
    cv::MatND hist= getHistogram(image);

    // find left extremity of the histogram
    int imin= 0;
    for( ; imin < histSize[0]; imin++ ) {
       // std::cout<<hist.at<float>(imin)<<std::endl;
        if (hist.at<float>(imin) > minValue)
            break;
    }

    // find right extremity of the histogram
    int imax= histSize[0]-1;
    for( ; imax >= 0; imax-- ) {

        if (hist.at<float>(imax) > minValue)
            break;
    }

    // Create lookup table
    int dims[1]={256};
    cv::MatND lookup(1,dims,CV_8U);

    for (int i=0; i<256; i++) {

        if (i < imin) lookup.at<uchar>(i)= 0;
        else if (i > imax) lookup.at<uchar>(i)= 255;
        else lookup.at<uchar>(i)= static_cast<uchar>(255.0*(i-imin)/(imax-imin)+0.5);
    }

    // Apply lookup table
    cv::Mat result;
    result= applyLookUp(image,lookup);

    return result;
}

// Applies a lookup table transforming an input image into a 1-channel image
cv::Mat Histogram::applyLookUp(const cv::Mat& image, const cv::MatND& lookup) {

    // Set output image (always 1-channel)
    cv::Mat result(image.rows,image.cols,CV_8U);
    cv::Mat_<uchar>::iterator itr= result.begin<uchar>();

    // Iterates over the input image
    cv::Mat_<uchar>::const_iterator it= image.begin<uchar>();
    cv::Mat_<uchar>::const_iterator itend= image.end<uchar>();

    // Applies lookup to each pixel
    for ( ; it!= itend; ++it, ++itr) {

        *itr= lookup.at<uchar>(*it);
    }

    return result;
}