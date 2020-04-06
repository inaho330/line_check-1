#include<unistd.h>
#include<stdio.h>
#include<opencv2/opencv.hpp>
#include<opencv2/imgproc.hpp>
#include<math.h>

using namespace cv;
using namespace std;
class MyLines{
	public:
		float rho;
		float theta;
		float x0;
		float y0;
		float my;
		float mx;
		float cx;
		float cy;
		MyLines(float r,float t){
			rho = r;
			theta = t;
			float a = cos (theta);
			float b = sin (theta);
			my = -(a/b);
			cy = rho/b;
			mx = -(b/a);
			cx = a*rho;
			// y = my * x + cy
			// x = mx * y * cx
			x0 = a * rho;
			y0 = b * rho;
		}
};

int main(int argc, char **argv)
{
    int i;
    float *line, rho, theta;
    double a,b, x0,y0;
    CvMemStorage *storage;
    CvSeq *lines = 0;
    CvPoint *point, pt1, pt2;

    CvCapture *videoCapture = cvCreateCameraCapture(0);

    IplImage *image;
    IplImage * gray;
    IplImage * prob;

	image = cvQueryFrame(videoCapture);
	CvSize frameSize = cvGetSize(image);
	float widthRate = frameSize.width * 0.05;
	float middleLine = frameSize.width * 0.5;
	float y1 = frameSize.height * 0.375;
	float y2 = frameSize.height * 0.5;
	float y3 = frameSize.height * 0.625;

    while(1){
        image = cvQueryFrame(videoCapture);
		gray = cvCreateImage(cvGetSize(image), IPL_DEPTH_8U, 1);
	cvRectangle(image,cvPoint(0,0),cvPoint(frameSize.width,frameSize.height/2),
		cvScalar(0, 0, 0),-1);
        cvInRangeS(image, cvScalar(150, 150, 150), cvScalar(255, 255, 255), gray);
        prob = cvCreateImage(cvGetSize(gray), IPL_DEPTH_8U, 1);
        prob = cvCloneImage(gray);

        cvCanny(gray, gray, 50, 200, 3);
        storage = cvCreateMemStorage(0);

		/*
        lines = 0;
        lines = cvHoughLines2(gray, storage, CV_HOUGH_PROBABILISTIC, 1, CV_PI/180, 50, 50, 10);
        for(i = 0; i < MIN(lines->total, 10); i++)
        {
            point = (CvPoint*)cvGetSeqElem(lines, i);
            cvLine(image, point[0], point[1], cvScalar(255, 0, 0), 3, 8, 0);
        }
		*/
		lines = cvHoughLines2 (gray, storage, CV_HOUGH_STANDARD, 1, CV_PI / 180, 50, 0, 0);
		vector<MyLines> lineList;
		for (i = 0; i < MIN (lines->total, 100); i++) {
			line = (float *) cvGetSeqElem (lines, i);
			rho = line[0];
			theta = line[1];

			a = cos (theta);
			b = sin (theta);
			x0 = a * rho;
			y0 = b * rho;

			bool isOneLine = false;
			if((theta > 0.3) && (theta < 2.8)){
				continue;		
			} 
			for (int i = 0; i<lineList.size(); ++i)
			{
				if(	fabs(lineList[i].y0 - y0) < widthRate &&
						fabs(lineList[i].theta-theta) < 0.3) {
						isOneLine = true;
						break;
				}
			}
			pt1.x = cvRound (x0 + 1000 * (-b));
			pt1.y = cvRound (y0 + 1000 * (a));
			pt2.x = cvRound (x0 - 1000 * (-b));
			pt2.y = cvRound (y0 - 1000 * (a));
			if (!isOneLine){
				cvLine (image , pt1, pt2, cvScalar(255, 255, 0), 1, 1, 0);
				lineList.push_back(MyLines(rho,theta));
			}
		}
		MyLines left(0,0),right(frameSize.width,0);
		for (int i = 0; i<lineList.size(); ++i)
		{
			fprintf(stderr,"|--rho:%.03f theta:%.03f x:%.03f y:%.03f\n",lineList[i].rho,lineList[i].theta,
					lineList[i].x0,lineList[i].y0);
			if(lineList[i].x0<middleLine){
				if(lineList[i].x0 > left.x0){
					left = lineList[i];	
				}	
			}else{
				if(lineList[i].x0 < right.x0){
					right = lineList[i];	
				}	
			}
		}
		fprintf(stderr,"left: %f %f right:%f %f\n",left.rho,left.theta,right.rho,right.theta);
		a = cos (left.theta);
		b = sin (left.theta);
		pt1.x = cvRound (left.x0 + 1000 * (-b));
		pt1.y = cvRound (left.y0 + 1000 * (a));
		pt2.x = cvRound (left.x0 - 1000 * (-b));
		pt2.y = cvRound (left.y0 - 1000 * (a));
		cvLine (image , pt1, pt2, cvScalar(255, 0, 0), 3, 8, 0);
		a = cos (right.theta);
		b = sin (right.theta);
		pt1.x = cvRound (right.x0 + 1000 * (-b));
		pt1.y = cvRound (right.y0 + 1000 * (a));
		pt2.x = cvRound (right.x0 - 1000 * (-b));
		pt2.y = cvRound (right.y0 - 1000 * (a));
		cvLine (image , pt1, pt2, cvScalar(255, 0, 0), 3, 8, 0);

		float lx1 = left.mx*y1+left.cx;
		float lx2 = left.mx*y2+left.cx;
		float lx3 = left.mx*y3+left.cx;
		float rx1 = right.mx*y1+right.cx;
		float rx2 = right.mx*y2+right.cx;
		float rx3 = right.mx*y3+right.cx;

		cvCircle(image ,cvPoint(lx1,y1),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint(lx2,y2),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint(lx3,y3),5,cvScalar(0, 0, 255),5,0);

		cvCircle(image ,cvPoint(rx1,y1),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint(rx2,y2),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint(rx3,y3),5,cvScalar(0, 0, 255),5,0);
		

		cvCircle(image ,cvPoint((rx1+lx1)/2,y1),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint((rx2+lx2)/2,y2),5,cvScalar(0, 0, 255),5,0);
		cvCircle(image ,cvPoint((rx3+lx3)/2,y3),5,cvScalar(0, 0, 255),5,0);

        cvNamedWindow("Hough_line_probalistic", CV_WINDOW_AUTOSIZE);
        cvShowImage("Hough_line_probalistic", image);
        int ret = cvWaitKey(100);
        if(ret > 0)
        break;
    }

    cvDestroyWindow("Hough_line_standard");
    cvDestroyWindow("Hough_line_standard");
    cvReleaseImage(&image);
    cvReleaseImage(&prob);
    cvReleaseImage(&gray);
    cvReleaseMemStorage(&storage);
    return 0;
}
