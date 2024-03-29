#include <stdlib.h>
#include <iostream>
#include <string>
#include "CartoImageProc.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "PerlinNoise.hpp"
#include "json.hpp"
//#include <QtWidgets/QFileDialog>

#define BASE_WINDOW_WIDTH   700
#define BASE_WINDOW_HEIGHT  670
using namespace Carto;
using namespace cv;
using json = nlohmann::json;

void refresh_window(int pos, void *userData);
void init_window(int window_number);
void refresh_preview();
void refresh_path();
void open_file(void *userdata);
void decrement_preview(Mat *edges, Mat *preview, uchar amount);
void waitLoop();
void save(void *userdata);
void load(void *userdata);
void new_layer(void *userdata);
void init_control_panel();
void get_layer_count();
void delete_layer(void *userdata);

int start = 0, end = 150, total_layers=1;
int starts[]= {1,1,1,1,1,1,1,1,1}; // Leave at 1 to pass getTrackbarPos test
int ends[] = {255,255,255,255,255,255,255,255,255};
int perlin_scales[] = {25,25,25,25,25,25,25,25,25};
int canny_scales[] = {0,1};
int curr_window=0, next_window_x=BASE_WINDOW_WIDTH, next_window_y=0;
int perlin_scale=25;
bool first_init = true;
//QString img_file;

std::string *config_file, image_name;

Mat imgmat1, imgmat2, imgmat3, imgmat4 ,canny, preview, preview_tsp, path_img;
Mat window_img[8], edges[8];

CartoImageProc *img;
CartoImageProc *procs[8];

void refresh(int pos, void *userData);
int main( int argc, char** argv ){
    config_file = new std::string(argv[2]);
    void *p = nullptr;
    //std::cout << cv::getBuildInformation() << std::endl;
    img=new CartoImageProc(argv[1]);
    img->toGrayscale();
    image_name.assign(argv[1]);
    get_layer_count();
    
    for(int i=0; i<total_layers; i++) {
        procs[i] = new CartoImageProc(argv[1], i);
        procs[i]->toGrayscale();
    }
    //procs[1] = new CartoImageProc(argv[1], 1);
    //procs[2] = new CartoImageProc(argv[1], 2);
    
    //procs[0]->toGrayscale();
    //procs[1]->toGrayscale();
    //procs[2]->toGrayscale();
    
    for(int i=0;i<total_layers;i++) {
        init_window((int)i);
    }
    
    init_control_panel();
    load(p);
    moveWindow("Preview",0,0);
    resizeWindow("Preview",BASE_WINDOW_WIDTH,BASE_WINDOW_HEIGHT);
    first_init=false;
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
            }
            case 'l': {
                img->reloadImage(image_name);
                img->toGrayscale();
                
                get_layer_count();
                
                for(int i=0; i<total_layers; i++) {
                    procs[i] = new CartoImageProc(image_name, i);
                    procs[i]->toGrayscale();
                }
                for(int i=0;i<total_layers;i++) {
                    init_window((int)i);
                }
                refresh_preview();

                break;
            }
            default: { exit(0); }
        }
    }
}
void init_window(int win_number) {
    Mat *i;
    CartoImageProc *p;
    int window_number=win_number;
    curr_window=win_number;
    
    String window_name="Step ";
    window_name+=std::to_string(window_number);

    p=procs[window_number];
    
    i = &window_img[window_number];
    
    *i=p->mat.clone();
   
    p->filterGrayscale(i,starts[window_number],ends[window_number]);
    
    if(perlin_scales[window_number] > 0) {
        p->filterPerlin(i,(double)perlin_scales[window_number]/100);
    }
    //p->autoFilterPerlin(i,(double)perlin_scales[window_number]/100);

    if(canny_scales[window_number] > 0) {
        Canny(*i,*i,1,3,3);
        bitwise_not(*i,*i);
    }
    //bitwise_not(edges[window_number],edges[window_number]);
    edges[window_number]=i->clone();
    p->show(*i,window_name);
    
    if(first_init) {
        createTrackbar("Start",window_name,&starts[window_number],255,refresh_window,(void *)&p->id);
        createTrackbar("End",window_name,&ends[window_number],255,refresh_window,&p->id);
        createTrackbar("Perlin",window_name,&perlin_scales[window_number],100,refresh_window,(void *)&p->id);
        createTrackbar("Canny",window_name,&canny_scales[window_number],1,refresh_window,(void *)&p->id);
        resizeWindow(window_name,BASE_WINDOW_WIDTH,BASE_WINDOW_HEIGHT);
  
        if(next_window_x+BASE_WINDOW_WIDTH > 2559) {
            next_window_x=0;
            next_window_y=BASE_WINDOW_HEIGHT;
        }
        
        moveWindow(window_name,next_window_x,next_window_y);
        
        next_window_x+=BASE_WINDOW_WIDTH;
    }
}

void refresh_window(int pos, void *userdata) {
    int *window = reinterpret_cast<int *>(userdata);
    
    int num=*window;
    init_window(num);
    refresh_preview();
    
    return;
}

void save(void *userdata) {
    json j;
    std::string file = procs[0]->image_name;
    int start,end,perlin,docanny;
    
    std::ifstream in(config_file->c_str());
    
    if(in.good()) {
        in >> j;
    }
    
    in.close();
    
    for(int i=0; i<total_layers; i++) {
        String window_name="Step ";
        window_name+=std::to_string(i);
        start=getTrackbarPos("Start", window_name);
        end=getTrackbarPos("End", window_name);
        perlin=getTrackbarPos("Perlin", window_name);
        docanny=getTrackbarPos("Canny",window_name);
        j[file]["windows"][i]["name"]=window_name;
        j[file]["windows"][i]["trackbars"]= {
            { "Start", start },
            { "End", end },
            { "Perlin", perlin },
            { "Canny", docanny}
        };
    }
    
    std::ofstream out(config_file->c_str());
    out << std::setw(4) << j << std::endl;
    out.close();
}

void get_layer_count() {
    json j;
    
    std::ifstream in(config_file->c_str());
    
    if(!in.good()) {
        total_layers=1;
        return;
    }else{
        in >> j;
        in.close();
    }
    
    total_layers=(int)j[image_name]["windows"].size();
    
    if(total_layers == 0){
        total_layers=1;
        return;
    }
    return;
}

void open_file(void *userdata) {
   // QWidget *widget =0;
  //  QString imageName = QFileDialog::getOpenFileName(widget,"Open image",".","Image Files (*.png *.jpg *.jpeg)");
    //std::cout << "Opening " << imageName.toStdString() << std::endl;
}

void load(void *userdata) {
    json j;
    int i;
    //std::string file=procs[0]->image_name;
    std::string file=image_name;
    
    std::ifstream in(config_file->c_str());
    
    if(in.good()) {
        in >> j;
        in.close();
    }else{
        in.close();
        total_layers=1;
        return;
    }
    
    for(i=0; i < j[file]["windows"].size(); i++) {
        std::string window_name=j[file]["windows"][i]["name"].get<std::string>();
            for(json::iterator it = j[file]["windows"][i]["trackbars"].begin(); it != j[file]["windows"][i]["trackbars"].end(); ++it){
                setTrackbarPos(it.key(), window_name, it.value());
           }
    }
    
    if(i == 0)
        total_layers=1;
    else
        total_layers=i;
}

void new_layer(void *userdata) {
    curr_window++;
    total_layers++;
    
    procs[total_layers-1] = new CartoImageProc(image_name, total_layers-1);
    procs[total_layers-1]->toGrayscale();
    init_window(total_layers-1);
}

void delete_layer(void *userdata) {
    curr_window--;
    total_layers--;
    
    free(procs[total_layers]);
}
void init_control_panel() {
    createButton("Open File", (ButtonCallback)open_file);
    createButton("Save", (ButtonCallback)save);
    createButton("Load", (ButtonCallback)load);
    createButton("New Layer", (ButtonCallback)new_layer);
    createButton("Delete Layer", (ButtonCallback)delete_layer);
}

void refresh_preview() {
    preview=edges[0].clone();
    preview=Scalar::all(255);

    for(int i=0; i< total_layers; i++) {
        decrement_preview(&edges[i],&preview,50);
    }
    
    //decrement_preview(&edges[0],&preview,50);
    //decrement_preview(&edges[1],&preview,100);
    
    //img->buildPath(&preview);
    img->show(preview,"Preview");
}

void refresh_path() {
    // We will split 3 times
    // Test will go against both original image and "filtered" preview
    //double block = 256 / 3;
    //only doing it once for now.
    double block=256;
    Mat tmp_mat = preview.clone();
    int ctr=0, top_limit=0;
    uchar orig_val, preview_val;
    String window_name;
    std::vector<Carto::CartoNode> nodes, final_nodes;
    Point start_point=Point(0,0);
    
    //img->show();
    
    while(ctr < 1) {
        tmp_mat=Scalar::all(255);
        top_limit+= block;
        
        for(int x=0; x < preview.cols; x++) {
            for(int y=0;y < preview.rows; y++) {
                orig_val = img->mat.at<uchar>(Point(x,y));
                preview_val = preview.at<uchar>(Point(x,y));
                
                if(orig_val < top_limit && preview_val < 255) {
                    tmp_mat.at<uchar>(Point(x,y))+=255;
                }
            }
        }
        img->getPath(&tmp_mat,&nodes,start_point);
        
        // Next time around start where we left off, so grab the last one.
        start_point=nodes[ nodes.size()-1].point;
        
        // Clear the last one preparing for appending to the final path. Insert so the start will be the previous finish.
        nodes.pop_back();
        final_nodes.insert(final_nodes.end(), nodes.begin(), nodes.end());
        
        window_name=std::to_string(ctr);
        //img->show(tmp_mat,window_name);
        ctr++;
    }
    
    // We now have a full path generated by ANN path finder. Time to process..
    //img->processPath(&preview, &final_nodes);
    tmp_mat = img->mat.clone();
    img->processPath(&tmp_mat,&final_nodes);
    img->show(tmp_mat,"Path");
    
    
    
    
    //img->buildPath(&preview);
    //img->show(preview,"Path");
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
    //img->autoFilterPerlin(&imgmat1,(double)perlin_scale/100);

    img->show(imgmat1,"0 to 150");
    
    canny=imgmat1.clone();
    Canny(canny,canny,1,3,3);
    bitwise_not(canny,canny);
    
    img->show(canny,"Scratch");
}
