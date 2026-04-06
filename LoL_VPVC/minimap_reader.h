#ifndef MINIMAP_READER_H
#define MINIMAP_READER_H

#include <iostream>
#include <windows.h>
#include <opencv2/opencv.hpp>

struct MapPos {
    int x;
    int y;
    bool is_valid;
    
    bool valid() {
        return is_valid;
    }
};

class MinimapReader {
public:
    MinimapReader();
    ~MinimapReader();

    void Init(int x, int y, int width, int height);
    void Capture();
    MapPos FindCentroid(int targetId); 

private:
    int mapX, mapY, mapWidth, mapHeight;
    
    cv::Mat frameAtual; 
};

#endif
