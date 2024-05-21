#include <iostream>
#include <mpi.h>
#include <opencv2/opencv.hpp>

using namespace cv;

int main(int argc, char** argv) {
    int rank, size;
    const int root = 0;
    const char* filename = "poza.jpg";
    const int newWidth = 1000; // Lățimea nouă dorită a imaginii redimensionate

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        std::cerr << "Programul trebuie să fie rulat pe cel puțin 2 procese MPI!" << std::endl;
        MPI_Finalize();
        return EXIT_FAILURE;
    }

    Mat image;
    if (rank == root) {
        image = imread(filename);
        if (image.empty()) {
            std::cerr << "Nu am putut încărca imaginea " << filename << std::endl;
            MPI_Finalize();
            return EXIT_FAILURE;
        }
    }

    // Transmiterea dimensiunilor imaginii de la rădăcină la celelalte procese
    int rows = 0, cols = 0;
    if (rank == root) {
        rows = image.rows;
        cols = image.cols;
    }
    MPI_Bcast(&rows, 1, MPI_INT, root, MPI_COMM_WORLD);
    MPI_Bcast(&cols, 1, MPI_INT, root, MPI_COMM_WORLD);

    // Calcularea dimensiunilor secțiunii fiecărui proces
    const int sectionSize = rows / size;
    const int remainingRows = rows % size;
    const int startRow = rank * sectionSize + std::min(rank, remainingRows);
    const int endRow = startRow + sectionSize + (rank < remainingRows ? 1 : 0);

    // Trimiterea secțiunii imaginii fiecărui proces
    if (rank == root) {
        for (int dest = 1; dest < size; ++dest) {
            const int destStartRow = dest * sectionSize + std::min(dest, remainingRows);
            const int destEndRow = destStartRow + sectionSize + (dest < remainingRows ? 1 : 0);
            const int rowsToSend = destEndRow - destStartRow;
            MPI_Send(image.ptr<Vec3b>(destStartRow), rowsToSend * cols * sizeof(Vec3b), MPI_BYTE, dest, 0, MPI_COMM_WORLD);
        }
    }
    else {
        image.create(sectionSize + (rank < remainingRows ? 1 : 0), cols, CV_8UC3);
        MPI_Recv(image.data, (sectionSize + (rank < remainingRows ? 1 : 0)) * cols * sizeof(Vec3b), MPI_BYTE, root, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    // Redimensionarea imaginii
    resize(image, image, Size(newWidth, image.rows));

    // Colectarea secțiunilor redimensionate în rădăcină
    if (rank == root) {
        for (int source = 1; source < size; ++source) {
            const int sourceStartRow = source * sectionSize + std::min(source, remainingRows);
            const int sourceEndRow = sourceStartRow + sectionSize + (source < remainingRows ? 1 : 0);
            const int rowsToReceive = sourceEndRow - sourceStartRow;
            MPI_Recv(image.ptr<Vec3b>(sourceStartRow), rowsToReceive * cols * sizeof(Vec3b), MPI_BYTE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
    else {
        MPI_Send(image.data, (sectionSize + (rank < remainingRows ? 1 : 0)) * cols * sizeof(Vec3b), MPI_BYTE, root, 0, MPI_COMM_WORLD);
    }

    // Afisare sau salvare imagine redimensionată
    if (rank == root) {
        imwrite("resized_image.jpg", image);
        imshow("Resized Image", image);
        waitKey(0);
    }

    MPI_Finalize();
    return EXIT_SUCCESS;
}
