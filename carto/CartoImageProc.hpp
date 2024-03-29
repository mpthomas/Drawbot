//
//  CartoImageProc.hpp
//  carto
//
//  Created by Matthew Thomas on 1/21/18.
//  Copyright © 2018 Matthew Thomas. All rights reserved.
//

#ifndef CartoImageProc_hpp
#define CartoImageProc_hpp

#include <stdio.h>
#include <string>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "CartoPath.hpp"
#include "CartoNode.hpp"
#include "CartoSimulator.hpp"

namespace Carto {

class CartoImageProc {
public:
    std::string image_name;
    int window_ctr,id, line_counter, canvas_rows=3000, canvas_cols=4900;
    cv::Mat mat;
    CartoSimulator *sim;
    std::vector<CartoNode> annPath;

    // Constructors
    CartoImageProc();
    CartoImageProc(std::string filename, int id=0);

    // Destructors
    ~CartoImageProc();
    
    void setMat(cv::Mat mat);
    
    cv::Mat getMat();
    
    cv::Mat filterGrayscale(int start, int end);
    void filterGrayscale(cv::Mat *inmat, int start, int end);
    void filterPerlin(cv::Mat *inmat, double scale);
    void autoFilterPerlin(cv::Mat *inmat, double scale);
    void createMask(cv::Mat *inmat, cv::Mat *mask, int start_x, int len_x, int start_y, int len_y);
    void getPath(cv::Mat *inmat,  std::vector<Carto::CartoNode> *nodes, cv::Point start_point = cv::Point(0,0));
    void processPath(cv::Mat *inmat, std::vector<Carto::CartoNode> *nodes);
    void buildTSPath(cv::Mat *inmat);
    void buildPath(cv::Mat *inmat);
    void renderPath(std::vector<CartoNode>, cv::Mat *inmat, cv::Point start_point);
    void toGrayscale();
    void show();
    void show(cv::Mat mat, std::string name);
    void reloadImage(std::string filename);
    
};
}

#endif /* CartoImageProc_hpp */
