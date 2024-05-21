#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>
#include <thread>
#include <vector>

void resizeSegment(const cv::Mat& src, cv::Mat& dst, int startRow, int endRow, float x_ratio, float y_ratio) {
    float px, py;
    for (int i = startRow; i < endRow; i++) {
        for (int j = 0; j < dst.cols; j++) {
            px = j * x_ratio;
            py = i * y_ratio;
            int x = static_cast<int>(px);
            int y = static_cast<int>(py);
            float x_diff = px - x;
            float y_diff = py - y;

            cv::Vec3b a = src.at<cv::Vec3b>(y, x);
            cv::Vec3b b = src.at<cv::Vec3b>(y, x + 1);
            cv::Vec3b c = src.at<cv::Vec3b>(y + 1, x);
            cv::Vec3b d = src.at<cv::Vec3b>(y + 1, x + 1);

            for (int k = 0; k < 3; k++) {
                dst.at<cv::Vec3b>(i, j)[k] =
                    a[k] * (1 - x_diff) * (1 - y_diff) +
                    b[k] * (x_diff) * (1 - y_diff) +
                    c[k] * (y_diff) * (1 - x_diff) +
                    d[k] * (x_diff * y_diff);
            }
        }
    }
}

cv::Mat resizeImage(const cv::Mat& src, int newWidth, int newHeight, int numThreads) {
    cv::Mat dst(newHeight, newWidth, src.type());

    float x_ratio = static_cast<float>(src.cols) / newWidth;
    float y_ratio = static_cast<float>(src.rows) / newHeight;

    std::vector<std::thread> threads;
    int segmentHeight = newHeight / numThreads;
    for (int t = 0; t < numThreads; t++) {
        int startRow = t * segmentHeight;
        int endRow = (t == numThreads - 1) ? newHeight : (t + 1) * segmentHeight;
        threads.emplace_back(resizeSegment, std::ref(src), std::ref(dst), startRow, endRow, x_ratio, y_ratio);
    }

    for (auto& thread : threads) {
        thread.join();
    }

    return dst;
}

int main() {
    // Calea către imaginea pe care vrei să o redimensionezi
    std::string imagePath = "input.jpg";

    // Încarcă imaginea
    cv::Mat image = cv::imread(imagePath);
    if (image.empty()) {
        std::cerr << "Nu s-a putut încărca imaginea!" << std::endl;
        return -1;
    }

    // Dimensiunile noi la care vrei să redimensionezi imaginea
    int newWidth = 640;
    int newHeight = 480;

    // Numărul de thread-uri
    int numThreads = std::thread::hardware_concurrency();

    // Măsoară timpul de execuție al funcției de redimensionare
    auto start = std::chrono::high_resolution_clock::now();
    cv::Mat resizedImage = resizeImage(image, newWidth, newHeight, numThreads);
    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << "Timpul de executie pentru redimensionare: " << duration.count() << " secunde" << std::endl;

    // Salvează imaginea redimensionată
    std::string outputImagePath = "redimensionata.jpg";
    cv::imwrite(outputImagePath, resizedImage);

    // Afișează imaginile
    cv::imshow("Original Image", image);
    cv::imshow("Resized Image", resizedImage);

    // Așteaptă apăsarea unei taste pentru a închide ferestrele
    cv::waitKey(0);

    return 0;
}
