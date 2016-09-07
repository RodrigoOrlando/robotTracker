#include <stdio.h>
#include <opencv2/opencv.hpp>
#include <sstream> 
#include <math.h>

#define PI 3.14159265

using namespace cv;
Mat image,hsvImg,grayImg,canny_output;
vector<vector<Point> > contours;
vector<Vec4i> hierarchy;
Mat selection;
int hueL=83;
int satL=0;
int valL=228;
int hueH=112;
int satH=38;
int valH=255;

int thresh = 100;
int max_thresh = 255;
RNG rng(12345);


void Foo(int val,void* data);
void getPose(vector<double> cx, vector<double> cy);
int main(int argc, char** argv )
{
    if ( argc != 2 )
    {
        printf("usage: DisplayImage.out <Image_Path>\n");
        return -1;
    }

        image = imread( argv[1], 1 );

        if ( !image.data )
        {
            printf("No image data \n");
            return -1;
        }
        Mat lowercolor;
        Mat uppercolor;
        namedWindow("DisplayImage", WINDOW_NORMAL );
        resizeWindow("DisplayImage", 620, 720);
        //namedWindow("Selection", WINDOW_NORMAL );
        
        createTrackbar("HueL","DisplayImage",&hueL,180,Foo);
        createTrackbar("SatL","DisplayImage",&satL,255,Foo);
        createTrackbar("ValL","DisplayImage",&valL,255,Foo);
        createTrackbar("HueH","DisplayImage",&hueH,180,Foo);
        createTrackbar("SatH","DisplayImage",&satH,255,Foo);
        createTrackbar("ValH","DisplayImage",&valH,255,Foo);
        createTrackbar( " Canny thresh:", "DisplayImage", &thresh, max_thresh, Foo );

    while(1){
        image = imread( argv[1], 1 );
        
        cvtColor(image,hsvImg,CV_RGB2HSV);
        
        //Read trackbars
        hueL=getTrackbarPos("HueL","DisplayImage");
        satL=getTrackbarPos("SatL","DisplayImage");
        valL=getTrackbarPos("ValL","DisplayImage");
        hueH=getTrackbarPos("HueH","DisplayImage");
        satH=getTrackbarPos("SatH","DisplayImage");
        valH=getTrackbarPos("ValH","DisplayImage");
        Foo(hueL,NULL);
        
        int tecla=waitKey(0)&0xFF;
        if (tecla == 27){
            destroyAllWindows();
            break;
        };
    };

    return 0;
}


void Foo(int val,void* data){
    inRange(hsvImg, Scalar(hueL,satL,valL), Scalar(hueH,satH,valH),selection);
    bitwise_and(image,image,selection,selection);
    erode(selection,selection,Mat(),Point(-1,-1),1);
    dilate(selection,selection,Mat(),Point(-1,-1),4);
    cvtColor(selection,selection,CV_HSV2BGR);
    cvtColor(selection,grayImg,CV_BGR2GRAY);
    //imshow("Selection", selection);
    blur( grayImg, grayImg, Size(3,3) );
    Canny( grayImg, canny_output, thresh, thresh*2, 3 );
    /// Find contours
    findContours( canny_output, contours, hierarchy, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE, Point(0, 0) );
    /// Draw contours
    Mat drawing = Mat::zeros( canny_output.size(), CV_8UC3 );
    vector<double> cx(contours.size() );
    vector<double> cy(contours.size() );
    vector<Moments> mu(contours.size() );
    if (contours.size()==3) {    
        for( int i = 0; i< contours.size(); i++ )
         {
           Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
           drawContours( drawing, contours, i, color, 2, 8, hierarchy, 0, Point() );
           if (contourArea( contours[i] ) > 50){
                mu[i] = moments(contours[i]);
                cx[i] = mu[i].m10/mu[i].m00;
                cy[i] = mu[i].m01/mu[i].m00;
                circle(image, Point(cx[i],cy[i]), 8, Scalar(0,0,255), 1, 8, 0);
           }
         };
         getPose(cx,cy);
     }

    /// Show in a window
    //namedWindow( "Contours", WINDOW_NORMAL );
    //resizeWindow("Contours", 640, 480);
    //imshow( "Contours", drawing );
    imshow("DisplayImage", image);
};

void getPose(vector<double> cx, vector<double> cy){
    Point arrow, position;
    double midx, midy;
    double dist01=sqrt(pow(cx[0]-cx[1],2)+pow(cy[0]-cy[1],2));
    double dist12=sqrt(pow(cx[1]-cx[2],2)+pow(cy[1]-cy[2],2));
    double dist20=sqrt(pow(cx[2]-cx[0],2)+pow(cy[2]-cy[0],2));
    if (dist01<dist12 && dist01<dist20){
        midx=(int)(cx[0]+cx[1])/2;
        midy=(int)(cy[0]+cy[1])/2;
        arrow=Point((int)cx[2],(int)cy[2]);
        position=Point(midx,midy);
    }
    else if (dist12<dist01 && dist12<dist20)
        {
            midx=(int)(cx[1]+cx[2])/2;
            midy=(int)(cy[1]+cy[2])/2;
            arrow=Point((int)cx[0],(int)cy[0]);
            position=Point(midx,midy);
        } 
    else if (dist20<dist01 && dist20<dist12)
        {
            midx=(int)(cx[2]+cx[0])/2;
            midy=(int)(cy[2]+cy[0])/2;
            arrow=Point((int)cx[1],(int)cy[1]);
            position=Point(midx,midy);
        }
    double dx=arrow.x-midx;
    double dy=midy-arrow.y;
    double angle = atan2 (dy,dx) * 180/PI;
    arrowedLine(image, position, arrow, Scalar(0,0,255), 2);
    std::ostringstream s;
    s << "(" << midx << "," << midy << ")";
    std::string pos = s.str();
    std::ostringstream a;
    a << (int)angle << " deg";
    std::string ang = a.str();  
    circle(image, Point(midx,midy), 2, Scalar(255,0,0), 1, 8, 0);
    putText(image, ang, Point((int)midx+40,(int)midy+70), FONT_HERSHEY_PLAIN, 2.4, Scalar( 0,0,255), 3);
    putText(image, pos, Point((int)midx+40,(int)midy+30), FONT_HERSHEY_PLAIN, 2.4, Scalar( 255,0,0), 3);
};