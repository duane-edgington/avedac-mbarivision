/******************************************************************************
 * Author: Martin Godec
 *         godec@icg.tugraz.at
 * Feature calculation from Juergen Gall
 *         gall@vision.ee.ethz.ch
 ******************************************************************************/

#include "features.h"
#include <vector>
#include <iostream>

using namespace std;

#define NUM_BINS 9
#define PI 3.14159265f

HoG::HoG() {
	bins = NUM_BINS;
	binsize = (PI * 80.0f)/float(bins);;

	g_w = 5;
	cv::Mat Gauss = cv::Mat( g_w, g_w, CV_32FC1 );
	double a = -(g_w-1)/2.0;
	double sigma2 = 2*(0.5*g_w)*(0.5*g_w);
	double count = 0; 
	IplImage tmp_Gauss = Gauss;
	for(int x = 0; x<g_w; ++x) {
		for(int y = 0; y<g_w; ++y) {
			double tmp = exp(-( (a+x)*(a+x)+(a+y)*(a+y) )/sigma2);
			count += tmp;
			cvSet2D( &tmp_Gauss, x, y, cv::Scalar(tmp) );
		}
	} 
	cvConvertScale( &tmp_Gauss, &tmp_Gauss, 1.0/count);

	ptGauss = new float[g_w*g_w];
	int i = 0;
	for(int y = 0; y<g_w; ++y)
		for(int x = 0; x<g_w; ++x)
			ptGauss[i++] = Gauss.at<float>(x,y);

}


void HoG::extractOBin(cv::Mat &Iorient, cv::Mat &Imagn, std::vector<cv::Mat>& out, int off) {
	double* desc = new double[bins];

	// reset output image (border=0) and get pointers
	uchar** ptOut     = new uchar*[bins];
	uchar** ptOut_row = new uchar*[bins];
	for(int k=off; k<bins+off; ++k) {
		IplImage tmp_out = out[k];
		cvSetZero( &tmp_out );
		cvGetRawData( &tmp_out, (uchar**)&(ptOut[k-off]));
	}

	// get pointers to orientation, magnitude
	int step;
	uchar* ptOrient;
	uchar* ptOrient_row; 
	IplImage tmp_Iorient = Iorient; 
	cvGetRawData( &tmp_Iorient, (uchar**)&(ptOrient), &step);
	step /= sizeof(ptOrient[0]);

	uchar* ptMagn;
	uchar* ptMagn_row;
	IplImage tmp_Imagn = Imagn; 
	cvGetRawData( &tmp_Imagn, (uchar**)&(ptMagn));

	int off_w = int(g_w/2.0);
	for(int l=0; l<bins; ++l)
		ptOut[l] += off_w*step;

	for(int y=0;y<Iorient.rows-g_w; y++, ptMagn+=step, ptOrient+=step) {

		// Get row pointers
		ptOrient_row = &ptOrient[0];
		ptMagn_row = &ptMagn[0];
		for(int l=0; l<bins; ++l)
			ptOut_row[l] = &ptOut[l][0]+off_w;

		for(int x=0; x<Iorient.cols-g_w; ++x, ++ptOrient_row, ++ptMagn_row) {

			calcHoGBin( ptOrient_row, ptMagn_row, step, desc );

			for(int l=0; l<bins; ++l) {
				*ptOut_row[l] = (uchar)desc[l];
				++ptOut_row[l];
			}
		}

		// update pointer
		for(int l=0; l<bins; ++l)
			ptOut[l] += step;
	}

	delete[] desc;
	delete[] ptOut;
	delete[] ptOut_row;
}

