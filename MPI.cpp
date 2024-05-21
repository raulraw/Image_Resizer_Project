#include <opencv2/opencv.hpp>
#include <iostream>
#include <mpi.h>

cv::Mat resizeImage(const cv::Mat& src, int newWidth, int newHeight) {
    cv::Mat dst(newHeight, newWidth, src.type());

    float x_ratio = static_cast<float>(src.cols) / newWidth;
    float y_ratio = static_cast<float>(src.rows) / newHeight;
    float px, py;

    for (int i = 0; i < newHeight; i++) {
        for (int j = 0; j < newWidth; j++) {
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
    return dst;
}

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);

    int rank, size;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    std::string imagePath = "poza.jpg";
    int newWidth = 640;
    int newHeight = 480;

    cv::Mat image;
    int rows, cols, type;

    if (rank == 0) {
        image = cv::imread(imagePath);
        if (image.empty()) {
            std::cerr << "Nu s-a putut încărca imaginea!" << std::endl;
            MPI_Abort(MPI_COMM_WORLD, -1);
        }
        rows = image.rows;
        cols = image.cols;
        type = image.type();
    }

    MPI_Bcast(&rows, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&type, 1, MPI_INT, 0, MPI_COMM_WORLD);

    int rows_per_proc = rows / size;
    int extra_rows = rows % size;

    int start_row = rank * rows_per_proc + std::min(rank, extra_rows);
    int end_row = start_row + rows_per_proc + (rank < extra_rows ? 1 : 0);

    int local_rows = end_row - start_row;

    std::cout << "Process " << rank << ": Start row = " << start_row << ", End row = " << end_row << std::endl;

    cv::Mat local_image(local_rows, cols, type);

    if (rank == 0) {
        for (int i = 1; i < size; ++i) {
            int start = i * rows_per_proc + std::min(i, extra_rows);
            int end = start + rows_per_proc + (i < extra_rows ? 1 : 0);
            cv::Mat send_img = image.rowRange(start, end);
            std::cout << "Process " << rank << ": Sending rows " << start << " to " << end - 1 << " to process " << i << std::endl;
            MPI_Send(send_img.data, send_img.total() * send_img.elemSize(), MPI_BYTE, i, 0, MPI_COMM_WORLD);
        }
        local_image = image.rowRange(start_row, end_row);
    }
    else {
        MPI_Recv(local_image.data, local_image.total() * local_image.elemSize(), MPI_BYTE, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Process " << rank << ": Received rows " << start_row << " to " << end_row - 1 << std::endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double start_time = MPI_Wtime();

    cv::Mat local_resized_image = resizeImage(local_image, newWidth, local_rows * newHeight / rows);

    std::cout << "Process " << rank << ": Resized local image from " << local_image.rows << " to " << local_resized_image.rows << " rows" << std::endl;

    if (rank == 0) {
        cv::Mat resizedImage(newHeight, newWidth, type);
        local_resized_image.copyTo(resizedImage.rowRange(0, local_resized_image.rows));
        for (int i = 1; i < size; ++i) {
            int start = i * rows_per_proc * newHeight / rows + std::min(i, extra_rows) * newHeight / rows;
            int end = start + (rows_per_proc + (i < extra_rows ? 1 : 0)) * newHeight / rows;
            cv::Mat recv_img(end - start, newWidth, type);
            MPI_Recv(recv_img.data, recv_img.total() * recv_img.elemSize(), MPI_BYTE, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            std::cout << "Process " << rank << ": Received resized rows " << start << " to " << end - 1 << " from process " << i << std::endl;
            recv_img.copyTo(resizedImage.rowRange(start, end));
        }

        std::string outputImagePath = "redimensionata.jpg";
        cv::imwrite(outputImagePath, resizedImage);

        cv::imshow("Original Image", image);
        cv::imshow("Resized Image", resizedImage);
        cv::waitKey(0);
    }
    else {
        MPI_Send(local_resized_image.data, local_resized_image.total() * local_resized_image.elemSize(), MPI_BYTE, 0, 0, MPI_COMM_WORLD);
        std::cout << "Process " << rank << ": Sent resized rows " << start_row * newHeight / rows << " to " << (end_row * newHeight / rows) - 1 << " to process 0" << std::endl;
    }

    MPI_Barrier(MPI_COMM_WORLD);
    double end_time = MPI_Wtime();

    if (rank == 0) {
        std::cout << "Total execution time: " << end_time - start_time << " seconds." << std::endl;
    }

    MPI_Finalize();
    return 0;
}
