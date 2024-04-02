#include <iostream>
#include <opencv2/opencv.hpp>
#include <ctime> 

using namespace std;
using namespace cv;

int main() {
    clock_t start = clock();

    // Citirea imaginii
    Mat image = imread("poza.jpg");

    // Verificăm dacă imaginea a fost citită corect
    if (image.empty()) {
        cout << "Nu am putut citi imaginea." << endl;
        return -1;
    }

    // Afisăm dimensiunile originale ale imaginii
    cout << "Dimensiunile originale ale imaginii: " << image.size() << endl;

    // Specificăm noile dimensiuni dorite pentru imagine
    Size newSize(200, 300);

    // Redimensionăm imaginea la noile dimensiuni folosind funcția resize din OpenCV
    Mat resizedImage;
    resize(image, resizedImage, newSize, 0, 0, INTER_LINEAR);

    // Afisăm dimensiunile redimensionate ale imaginii
    cout << "Dimensiunile redimensionate ale imaginii: " << resizedImage.size() << endl;

    // Salvează imaginea redimensionată în proiect
    string outputFilename = "resized_image.jpg"; 
    imwrite(outputFilename, resizedImage);

    // Oprire cronometru și calcul timp de rulare
    clock_t end = clock();
    double duration = (double)(end - start) / CLOCKS_PER_SEC;

    
    cout << "Timpul de rulare al programului: " << duration << " secunde." << endl;

    // Afisăm imaginea originală și imaginea redimensionată
    namedWindow("Imagine originala", WINDOW_NORMAL);
    namedWindow("Imagine redimensionata", WINDOW_NORMAL);

    imshow("Imagine originala", image);
    imshow("Imagine redimensionata", resizedImage);

    waitKey(0);
    destroyAllWindows();

    return 0;
}
