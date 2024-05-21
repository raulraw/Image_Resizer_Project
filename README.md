# **_Image Resizer Project_**


 Pentru acest proiect am folosit limbajul C++, impreuna cu mediul de dezvoltare Visual Studio. Proiectul consta in compararea a mai multor moduri de abordare asupra Image Resizer-ului, pentru acesta fiind implementate urmatoarele metode :
* Procesare Secventiala
* Procesare Secventiala folosind libraria OpenCv2
* Paralelizare folosind MPI
* Paralelizare folosind MPI dar si libraria OpenCv2
* Paralelizare folosind Threads

Algoritmul incepe prin citirea imaginii de intrare prin intermediul librariei OpenCv2, sunt afisate dimensiunile originale ale imaginii, apoi se specifica noile dimensiuni dorite. Imaginea este redimensionata in cadrul functiei de resize, in cazul paralelizarii, poza este impartita pe mai multe procese/fire de executie, iar pe fiecare bucatica de poza trimisa se efectueaza functia de resize. Dupa ce functia a fost apelata, toate bucatile se trimit catre procesul/firul de executie principal, unde poza urmeaza sa fie asamblata. La finalul executiei este afisat timpul de executie pentru fiecare metoda in parte.

## **_Dispozitivul folosit in cadrul proiectului_** / **_Laptop Lenovo Legion 5_**
* Procesor AMD Ryzen 5 5600H (6 nuclee, 12 threads, 3.3GHz, TURBO 4.2 GHz)
* Placa Grafica NVIDIA RTX 3060 Mobile (GDDR6 6GB VRAM, 3584 CUDA Cores)
* 16GB RAM


 

![input - Threads](https://github.com/raulraw/Image_Resizer_Project/assets/119895675/7228ec98-0c71-4927-9471-ab26b3f74470)
![output - Threads](https://github.com/raulraw/Image_Resizer_Project/assets/119895675/ec9249b6-b393-4aa5-869b-10b95513c324)
