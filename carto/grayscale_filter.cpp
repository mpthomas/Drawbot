//
//  grayscale_filter.cpp
//  carto
//
//  Created by Matthew Thomas on 1/21/18.
//  Copyright © 2018 Matthew Thomas. All rights reserved.
//

#include <stdlib.h>
#include <iostream>
#include <string>
#include "CartoImageProc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "PerlinNoise.hpp"

using namespace Carto;
using namespace cv;

void refresh_window(int pos, void *userData);
void init_window(int window_number);
void refresh_preview();
void refresh_path();
void decrement_preview(Mat *edges, Mat *preview, uchar amount);
void waitLoop();

int start = 0, end = 150;
int starts[]= {1,1,1}; // Leave at 1 to pass getTrackbarPos test
int ends[] = {255,255,255};
int perlin_scales[] = {25,25,25};
int curr_window=0;
int perlin_scale=25;
Mat imgmat1, imgmat2, imgmat3, imgmat4 ,canny, preview;
Mat window_img[3], edges[3];

CartoImageProc *img;

CartoImageProc *procs[3] = {new CartoImageProc("/Users/matt/xcode/Cartogrifer/carto/carto/img.jpg",0),
                           new CartoImageProc("/Users/matt/xcode/Cartogrifer/carto/carto/img.jpg",1),
                           new CartoImageProc("/Users/matt/xcode/Cartogrifer/carto/carto/img.jpg",2)};

void refresh(int pos, void *userData);
int main( int argc, char** argv ){
    img=new CartoImageProc("/Users/matt/xcode/Cartogrifer/carto/carto/img.jpg");
    img->toGrayscale();
    /*
    imgmat1 = img->mat.clone();
    imgmat2 = img->mat.clone();
    imgmat3 = img->mat.clone();
    imgmat4 = img->mat.clone();
    
    img->filterGrayscale(&imgmat1,0,150);
    img->filterPerlin(&imgmat1,(double)perlin_scale/100);
    canny=imgmat1.clone();
    Canny(canny,canny,1,3,3);
    
    bitwise_not(canny,canny); // reverse black/white
    img->show(canny,"Scratch");
    
    img->show(imgmat1,"0 to 150");
    
    img->filterGrayscale(&imgmat2,151,200);
    img->show(imgmat1,"151 to 200");
    moveWindow("151 to 200",imgmat2.cols,0);
    
    img->filterGrayscale(&imgmat3,200,255);
    img->show(imgmat3,"200 to 255");
    moveWindow("200 to 255",0,imgmat3.rows+50);
    
    img->filterGrayscale(&imgmat4,151,255);
    img->show(imgmat4,"151 to 255");
    moveWindow("151 to 255",imgmat4.cols,imgmat4.rows);
    
    createTrackbar("Start","0 to 150",&start, 50, refresh);
    createTrackbar("End","0 to 150",&end, 200, refresh);
    createTrackbar("Perlin","0 to 150",&perlin_scale, 100, refresh);
    */
    procs[0]->toGrayscale();
    procs[1]->toGrayscale();
    procs[2]->toGrayscale();
    
    init_window((int)0);
    init_window((int)1);

    waitLoop();
}

void waitLoop() {
    while(1){
        char key=waitKey(0);
        
        switch(key) {
            case 'r': {
                refresh_preview();
                break;
            }
            case 'p': {
                refresh_preview();
                refresh_path();
                break;
            }default: { exit(0); }
        }
    }
}
void init_window(int win_number) {
    Mat *i;
    CartoImageProc *p;
    int window_number=win_number;
    curr_window=win_number;
    
    string window_name="Step ";
    window_name.append(std::to_string(window_number));

    p=procs[window_number];
    
    i = &window_img[window_number];
    
    *i=p->mat.clone();
    
    p->filterGrayscale(i,starts[window_number],ends[window_number]);
    
    if(p->id == 0) {
        p->filterPerlin(i,(double)perlin_scales[window_number]/100);
    }
    
    edges[window_number]=i->clone();
    Canny(edges[window_number],edges[window_number],1,3,3);
    bitwise_not(edges[window_number],edges[window_number]);
    
    p->show(*i,window_name);
    
    //refresh_preview();
    
    if(!(getTrackbarPos("Start",window_name) > 0)) {
        createTrackbar("Start",window_name,&starts[window_number],255,refresh_window,(void *)&p->id);
        createTrackbar("End",window_name,&ends[window_number],255,refresh_window,&p->id);
        createTrackbar("Perlin",window_name,&perlin_scales[window_number],100,refresh_window,(void *)&p->id);
    }
}

void refresh_window(int pos, void *userdata) {
    //int window = *((int *)&userdata);
    int *window = reinterpret_cast<int *>(userdata);
    
    int num=*window;
    init_window(num);
    return;
}

void refresh_preview() {
    preview=edges[0].clone();
    preview=Scalar::all(255);

    decrement_preview(&edges[0],&preview,50);
    decrement_preview(&edges[1],&preview,100);
    
    //img->buildPath(&preview);
    img->show(preview,"Preview");
}

void refresh_path() {
    img->buildPath(&preview);
    img->show(preview,"Path");
}

void decrement_preview(Mat *edges, Mat *preview, uchar amount){
    uchar *eval,*pval;
    
    for(int x=0; x < edges->cols; x++){
        for(int y=0; y < edges->rows; y++) {
            eval=&edges->at<uchar>(Point(x,y));
            pval=&preview->at<uchar>(Point(x,y));

            if(*eval < 255 && *pval > amount) {
                *pval-=amount;
            }
        }
    }
}

void refresh(int pos, void *userData) {
    imgmat1 = img->mat.clone();
    img->filterGrayscale(&imgmat1, start,end);
    img->filterPerlin(&imgmat1,(double)perlin_scale/100);

    img->show(imgmat1,"0 to 150");
    
    canny=imgmat1.clone();
    Canny(canny,canny,1,3,3);
    bitwise_not(canny,canny);
    
    img->show(canny,"Scratch");
}
