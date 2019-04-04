/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   main.cpp
 * Author: User
 *
 * Created on 16 March 2019, 13:55
 */

#include <cstdlib>
#include <opencv2/opencv.hpp>
#include<vector>
#include <cmath>

#include "Car.hpp"
#include "AckermanModel.hpp"

using namespace std;
using namespace cv;

double distanceCalculate(Point p1, Point p2)
{
    double x = p1.x - p2.x; //calculating number to square in next step
    double y = p1.y - p2.y;
    double dist;

    dist = pow(x, 2) + pow(y, 2);       //calculating Euclidean distance
    dist = sqrt(dist);                  

    return dist;
}

int mx, my;
int mc[2] = {0};
int pointsCount = 1;
vector<Point> pathPoints;
Mat pathMatrix(1024, 1024, CV_8UC3, cv::Scalar(122,122,122));

void drawTire(cv::Mat image, cv::Point2f center, cv::Size2f scale, float angle){
  cv::ellipse(image, cv::RotatedRect(center, scale, angle), cv::Scalar(0,0,0), -1, 4);
}

void drawCar(cv::Mat image, Car car, float steerAngle, bool del){
  float widthT  = car.width/3;
  float heightT = car.height/6;
  cv::Scalar carColor[2] = {cv::Scalar(255, 0, 0), cv::Scalar(255, 255, 255)};
  if(del)
  { 
    carColor[0] = cv::Scalar(122, 122, 122);
    carColor[1] = cv::Scalar(122, 122, 122);
  }

  cv::Point2f vertices2f[4];
  cv::Point vertices[4];

  cv::RotatedRect(cv::Point2f(car.posx, car.posy), cv::Size2f(car.width + 10, car.height), car.angle).points(vertices2f);

  for(int i = 0; i < 4; ++i)
  {
    vertices[i] = vertices2f[i];
  }
  cv::fillConvexPoly(image, vertices, 4, carColor[0]);
  cv::putText(image, "SUBI", cv::Point2f(car.posx - 0.4 * car.width, car.posy + 0.3 * car.height), 1, 1, carColor[1],3);
  drawTire(image, cv::Point2f(car.posx -       car.width * 0.5 + widthT * 0.5, car.posy - 0.5 * car.height), cv::Size2f(widthT, heightT), 0);
  drawTire(image, cv::Point2f(car.posx -       car.width * 0.5 + widthT * 0.5, car.posy + 0.5 * car.height), cv::Size2f(widthT, heightT), 0);
  drawTire(image, cv::Point2f(car.posx + 0.5 * car.width -       widthT * 0.5, car.posy + 0.5 * car.height), cv::Size2f(widthT, heightT), steerAngle); 
  drawTire(image, cv::Point2f(car.posx + 0.5 * car.width -       widthT * 0.5, car.posy - 0.5 * car.height), cv::Size2f(widthT, heightT), steerAngle);
}

void mcb(int event, int x, int y, int flags, void* userdata)
{
    mx = x;
    my = y;
    if(event == EVENT_LBUTTONDOWN) mc[0] = 1;
    if(event == EVENT_LBUTTONUP) mc[0] = 0;
    if(event == EVENT_RBUTTONDOWN) mc[1] = 1;
    if(event == EVENT_RBUTTONUP) mc[1] = 0;
}

void onPathDrawing(int event, int x, int y, int flags, void* userdata)
{
    Point p = Point(x,y);
    
    if(MouseEventFlags::EVENT_FLAG_LBUTTON == event)
    {
        pointsCount++;
        pathPoints.push_back(p);
      //  pathPoints.size();
        
        circle(pathMatrix, Point(x,y), 0,  Scalar(0, 0, 255), 15);

        if(pointsCount == 1) return;
        
        Point prevPoint = pathPoints[pointsCount - 2];
        
        line(pathMatrix, p, prevPoint , Scalar(0,0,255), 10);

    }
}


void drawPath(Car& car)
{
    double minDist = 10000;
    pathPoints.push_back(Point(car.posx, car.posy));

    namedWindow("pathDrawer");
    circle(pathMatrix, Point(car.posx, car.posy), 20, Scalar(0, 0, 255), 2);
    cv::setMouseCallback("pathDrawer", onPathDrawing);
   
    DynamicInput currInput = {0};
    AckermanModel useAckerman;
    bool del = 0 ;
    int pointGoal = 1;

    while(true)
    {    
            imshow("pathDrawer", pathMatrix);
            char c = waitKey(10);
            if(del)
            {
            drawCar(pathMatrix, car, currInput.steerAngle, del);
            del = 0;
            }
            double angleToGoal = atan2((car.posy - pathPoints[pointGoal].y), (car.posx - pathPoints[pointGoal].x))*(180/M_PI);
            double distCarGoal = distanceCalculate(Point(car.posx, car.posy), pathPoints[pointGoal]);
            //std::cout<<distCarGoal<< " ";
             
            currInput.steerAngle = - car.angle*180/M_PI + angleToGoal;
            currInput.velocity = 5;
            std::cout<<currInput.steerAngle<< " " << car.angle<<endl;
            useAckerman.ackSteering(car, currInput);
            if(distCarGoal < 10 && pathPoints.size() > pointGoal){
                pointGoal++;
                currInput.velocity = 0;
            }
            //std::cout<<car.posx<<" "<<car.posy << " ";
            // circle(pathMatrix, Point(car.posx, car.posy), 20, Scalar(0,0,255),2);

            drawCar(pathMatrix, car, currInput.steerAngle, del);
            del = true;
            //wind.drawCar(State(car.posx, car.posy, car.angle));
       
        if(c == 27 )
        {
            break;
        }
    }
}




int main(int argc, char** argv)
{
    Mat m(512, 512, CV_8UC3);
//    m = imread("index.jpeg");
    
    DynamicInput currInput = {0};
    AckermanModel useAckerman;

    // namedWindow("cntr");
    // cv::setMouseCallback("cntr", mcb);
    
    Car car(50,50,0.0f);
    drawPath(car);

    int control = 0;
//     while(true)
//     {
// //        circle(m, Point(mx, my), 20, Scalar(0, 0, 255), 2);
// //        line(m, Point(50, 50), Point(mx, my), Scalar(0, 255, 0), 2);
        
        
//         circle(m, Point(car.posx, car.posy), 20, Scalar(0, 0, 255), 2);

//         m *= 0.95f;
//         imshow("cntr", m);
//         char c = waitKey(10);
//         if(c == 27) break;
        
//         // if(c == 'a') {
//         //     currInput.steerAngle += 2;
//         //     useAckerman.ackSteering(car, currInput);
//         //     std::cout<<"left"<<car.posx<<" "<<car.posy<<" "<<car.angle<<std::endl;
//         // }
//         // if(c == 's') {
//         //     currInput.velocity = -10;
//         //     useAckerman.ackSteering(car, currInput);
//         //     std::cout<<"down"<<car.posx<<" "<<car.posy<<" "<<car.angle<<std::endl;

//         // }

//         // if(c == 'd') {
//         //     currInput.steerAngle -= 2;
//         //     useAckerman.ackSteering(car, currInput);
//         //     std::cout<<"right"<<car.posx<<" "<<car.posy<<" "<<car.angle<<std::endl;

//         // }
//         // if(c == 'w') {
//         //     currInput.velocity = 10;
//         //     useAckerman.ackSteering(car, currInput);
//         //     std::cout<<"up"<<car.posx<<" "<<car.posy<<" "<<car.angle<<std::endl;

//         // }
//         // if(c== 'x'){
//         //     currInput.velocity = 0;
//         //     useAckerman.ackSteering(car, currInput);
//         //     std::cout<<"stop"<<car.posx<<" "<<car.posy<<" "<<car.angle<<std::endl;
//         // }
        
//     }
//     //  for(int i = 0; i< pathPoints.size();i++ ){
//     //     std::cout<<pathPoints[i].x<<" "<<pathPoints[i].y<<" ";
//     // }
    std::cout<<"ENDDDDDDDDDDDDDDDDDDDDD";
}