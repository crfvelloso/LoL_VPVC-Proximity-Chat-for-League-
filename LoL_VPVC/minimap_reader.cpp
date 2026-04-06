#include "minimap_reader.h"

MinimapReader::MinimapReader() {
    mapX = 0; mapY = 0; mapWidth = 0; mapHeight = 0;
}

MinimapReader::~MinimapReader() {
}

void MinimapReader::Init(int x, int y, int width, int height) {
    // Salvamos as coordenadas que o main.cpp nos passar
    mapX = x;
    mapY = y;
    mapWidth = width;
    mapHeight = height;
    std::cout << "[MinimapReader] Minimapa configurado na regiao X:" << x << " Y:" << y << std::endl;
}

void MinimapReader::Capture() {
    // Se a largura ou altura for 0, não tem o que capturar
    if (mapWidth <= 0 || mapHeight <= 0) return;

    // 1. Prepara as ferramentas do Windows para "ler" a tela
    HWND hwnd = GetDesktopWindow();
    HDC hwindowDC = GetDC(hwnd);
    HDC hwindowCompatibleDC = CreateCompatibleDC(hwindowDC);
    SetStretchBltMode(hwindowCompatibleDC, COLORONCOLOR);

    // 2. Cria um espaço vazio na memória para jogar a foto
    HBITMAP hbwindow = CreateCompatibleBitmap(hwindowDC, mapWidth, mapHeight);
    BITMAPINFOHEADER bi = {0};
    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = mapWidth;
    bi.biHeight = -mapHeight; // Negativo para a imagem não ficar de ponta-cabeça
    bi.biPlanes = 1;
    bi.biBitCount = 32;
    bi.biCompression = BI_RGB;

    SelectObject(hwindowCompatibleDC, hbwindow);
    
    // 3. O CLICK DA CÂMERA: Copia os pixels da tela para a nossa memória
    BitBlt(hwindowCompatibleDC, 0, 0, mapWidth, mapHeight, hwindowDC, mapX, mapY, SRCCOPY);

    // 4. Passa a imagem do Windows para o OpenCV (cv::Mat)
    frameAtual.create(mapHeight, mapWidth, CV_8UC4);
    GetDIBits(hwindowCompatibleDC, hbwindow, 0, mapHeight, frameAtual.data, (BITMAPINFO*)&bi, DIB_RGB_COLORS);

    // 5. Limpa a memória (MUITO importante para o PC não travar)
    DeleteObject(hbwindow);
    DeleteDC(hwindowCompatibleDC);
    ReleaseDC(hwnd, hwindowDC);

   // 6. Remove a transparência para o OpenCV trabalhar melhor com as cores
    cv::cvtColor(frameAtual, frameAtual, cv::COLOR_BGRA2BGR);


}

MapPos MinimapReader::FindCentroid(int targetId) {
    MapPos pos; pos.x = 0; pos.y = 0; pos.is_valid = false;
    if (frameAtual.empty()) return pos;

    cv::Mat hsv;
    cv::cvtColor(frameAtual, hsv, cv::COLOR_BGR2HSV);

    cv::Mat maskWhite;
    cv::inRange(hsv, cv::Scalar(0, 0, 200), cv::Scalar(180, 50, 255), maskWhite);


    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    cv::dilate(maskWhite, maskWhite, kernel, cv::Point(-1, -1), 2);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(maskWhite, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    double maxArea = 0;

    for (size_t i = 0; i < contours.size(); i++) {
        cv::Rect box = cv::boundingRect(contours[i]);
        double area = box.width * box.height; 
        
        if (box.width > 40 && box.width < 250 && box.height > 30 && box.height < 200) {
            int cx = box.x + box.width / 2;
            int cy = box.y + box.height / 2;

            if (area > maxArea) {
                maxArea = area;
                pos.x = cx;
                pos.y = cy;
                pos.is_valid = true;
            }
        }
    }

    // Desenhávamos uma mira aqui, mas desligamos para economizar processamento
    // A janela principal da Visão do Robô também está desligada abaixo:
    // cv::imshow("Visao do Robo", frameAtual);
    // cv::waitKey(1);

    return pos;
}
