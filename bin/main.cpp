#include <iostream>
#include <fstream>

struct Arguments {
    char* input;
    char* output;
    int32_t maxIter = -1;
    int freq = -1;
};

struct Matrix {
    uint64_t *matrix;
    int width;
    int height;
};

#pragma pack(push, 1)

struct BMPFileHeader {
    uint16_t bfType = 0x4D42;
    uint32_t bfSize = 0;
    uint16_t bfReserved1 = 0;
    uint16_t bfReserved2 = 0;
    uint32_t bfOffBits = 0;
};

struct BMPInfoHeader {
    uint32_t biSize = 40;
    int32_t  biWidth = 0;
    int32_t  biHeight = 0;
    uint16_t biPlanes = 1;
    uint16_t biBitCount = 4;
    uint32_t biCompression = 0;
    uint32_t biSizeImage = 0;
    int32_t  biXPelsPerMeter = 100;
    int32_t   biYPelsPerMeter = 100;
    uint32_t biClrUsed = 10;
    uint32_t biClrImportant = 10;
};

uint32_t palette[10] = {
    0xFFFFFF,  // 0 - Белый
    0x00FF00,  // 1 - Зеленый
    0x800080,  // 2 - Фиолетовый
    0xFFFF00,  // 3 - Желтый
    0xCECECE,  // 4 - серый
    0x33FFFF,  // 5 - голубой
    0xFF0000,  // 6 - красный
    0x3300FF,  // 7 - синий
    0x996600,  // 8 - коричневый
    0x000000   // >8 - Черный
};

#pragma pack(pop)

uint8_t getColor(int col) {
    if (col > 8) return 9;
    return col;
}

void BMPmaker(Matrix matrix, const std::string& filename) {
    BMPFileHeader fileHeader;
    BMPInfoHeader infoHeader;

    int rowSize = (matrix.width + 3) & ~3;
    int imageSize = rowSize * matrix.height;
    int widthHalf = std::ceil(static_cast<float>(matrix.width) / 2);

    fileHeader.bfOffBits = sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + sizeof(palette);
    fileHeader.bfSize = fileHeader.bfOffBits + imageSize;

    infoHeader.biWidth = rowSize;
    infoHeader.biHeight = matrix.height;
    infoHeader.biSizeImage = imageSize;

    std::ofstream file(filename, std::ios::out | std::ios::binary);

    file.write(reinterpret_cast<const char*>(&fileHeader), sizeof(fileHeader));
    file.write(reinterpret_cast<const char*>(&infoHeader), sizeof(infoHeader));
    file.write(reinterpret_cast<const char*>(palette), sizeof(palette));

    for (int y = matrix.height - 1; y >= 0; --y) {
        for (int x = 0; x < matrix.width; x += 2) {
            uint8_t byte;
            uint8_t fpixel = getColor(matrix.matrix[y * matrix.width + x]);
            byte = fpixel << 4;
            uint8_t cpixel;
            if (y * matrix.width + x + 1 < (y + 1) * matrix.width) {
                cpixel = getColor(matrix.matrix[y * matrix.width + x + 1]);
            } else {
                cpixel = 0;
            }
            byte |= cpixel;
            file.write(reinterpret_cast<const char*>(&byte), sizeof(byte));
        }
        if (!(matrix.width % 4 == 0)) {
            char padding = 0;
            for (int e = 0; e < (4 - widthHalf % 4) % 4; ++e)
                file.write(&padding, 1);
        }
    }
    file.close();
}

Arguments ArgumentParser(int argc, char* argv[]) {
    Arguments arguments;
    for (int i = 1; i < argc; i++) {

        if (argv[i][1] == 'o') {
            arguments.output = argv[i + 1];
        }

        if (argv[i][2] == 'o') {
            int j = 10;
            while (argv[i][j] != '\0') {
                arguments.output[j - 3] = argv[i][j];
                j++;
            }
        }

        if (argv[i][1] == 'i') {
            arguments.input = argv[i + 1];
        }

        if (argv[i][2] == 'i') {
            int j = 9;
            while (argv[i][j] != '\0') {
                arguments.input[j] = argv[i][j];
                j++;
            }
        }

        if (argv[i][1] == 'm') {
            int j = 0;
            while (argv[i + 1][j] != '\0') {
                arguments.maxIter = abs((arguments.maxIter * 10) + (argv[i + 1][j] - '0'));
                j++;
            }
        }

        if (argv[i][2] == 'm') {
            int j = 12;
            while (argv[i][j] != '\0') {
                arguments.maxIter = (arguments.maxIter * 10) + (argv[i][j] - '0');
                j++;
            }
        }

        if (argv[i][1] == 'f') {
            int j = 0;
            while (argv[i + 1][j] != '\0') {
                arguments.freq = (arguments.freq * 10) + (argv[i + 1][j] - '0');
                j++;
            }
        }

        if (argv[i][2] == 'f') {
            int j = 9;
            while (argv[i][j] != '\0') {
                arguments.freq = (arguments.freq * 10) + (argv[i][j] - '0');
                j++;
            }
        }
    }
    return arguments;
}

void GetMaxCoordinates(int& x, int& y, char* inPutFile) {
    std::ifstream file(inPutFile);
    int maxx = 0, maxy = 0, value;
    if (file.is_open()) {
        while (!file.eof()) {
            file >> x >> y >> value;
            maxx = std::max(x, maxx);
            maxy = std::max(y, maxy);
        }
    }
    x = maxx + 1;
    y = maxy + 1;
    //std::cout << x << ' ' << y << '\n';
}

void GetSand(uint64_t *matrix, int width, int height, char* inPutFile) {
    std::ifstream file(inPutFile);
    std::fill(matrix, matrix + width * height, 0);
    for (int i = 0; !file.eof(); ++i) {
        int16_t x;
        int16_t y;
        uint64_t sand;
        file >> x >> y >> sand;
        matrix[y * width + x] = sand;
        //std::cout << (y * height) + (x * width) << ' ' << y << ' ' << height << ' ' << x << ' ' << width << ' ' << sand << '\n';
    }
}

void MakeMatrixBigger(Matrix &matrix, int side) {
    int new_width = matrix.width;
    int new_height = matrix.height;

    int x_offset = 0;
    int y_offset = 0;

    switch (side) {
        case 1:
            new_width += 1;
            x_offset = 1;
            break;
        case 2:
            new_height += 1;
            y_offset = 1;
            break;
        case 0:
            new_width += 1;
            break;
        case 3:
            new_height += 1;
            break;
    }

    uint64_t* new_data = new uint64_t[new_width * new_height];
    std::fill(new_data, new_data + new_width * new_height, 0);

    for (int y = 0; y < matrix.height; ++y) {
        for (int x = 0; x < matrix.width; ++x) {
            new_data[new_width * (y + y_offset) + (x + x_offset)] = matrix.matrix[y * matrix.width + x];
        }
    }

    delete[] matrix.matrix;

    matrix.matrix = new_data;
    matrix.width = new_width;
    matrix.height = new_height;
}

std::ostream& operator<<(std::ostream &stream, const Matrix &matrix) {
    for (int y = 0; y < matrix.height; ++y) {
        for (int x = 0; x < matrix.width; ++x) {
            stream << matrix.matrix[y * matrix.width + x] << ' ';
        }
        stream << '\n';
    }
    return stream;
}

void SandFall(Matrix &matrix, int maxIter, int freq) {
    bool flag = true;
    int iterator = 0;
    
    while (flag) {
        flag = false;

        int xoffset = 0;
        int yoffset = 0;

        int startWidth = matrix.width;
        int startHeight = matrix.height;

        for (int i = 0; i < startHeight; ++i) {
            for (int j = 0; j < startWidth; ++j) {
                //std::cout << startWidth << ' ' << startHeight << '\n';
                //std::cout << matrix.height * matrix.width << ' ' << (i + yoffset) * matrix.width + j + xoffset <<
                //' ' << i << ' ' << j << ' ' << matrix.width << '\n';

                if (matrix.matrix[(i + yoffset) * matrix.width + j + xoffset] > 3) {
                    //0->   1<-   2^   3Y

                    if (i + yoffset == 0) {
                        MakeMatrixBigger(matrix, 2);
                        yoffset = 1;
                    }
                    if (j + xoffset == matrix.width - 1) {
                        MakeMatrixBigger(matrix, 0);
                    }
                    if (i + yoffset == matrix.height - 1) {
                        MakeMatrixBigger(matrix, 3);
                    }
                    if (j + xoffset == 0) {
                        MakeMatrixBigger(matrix, 1);
                        xoffset = 1;
                    }
                    //std::cout << matrix;
                    uint16_t newx = j + xoffset;
                    uint16_t newy = i + yoffset;

                    //std::cout << newy << ' ' << newx << ' ' << '\n';


                    matrix.matrix[newy * matrix.width + newx + matrix.width] += matrix.matrix[newy * matrix.width + newx] / 4;
                    matrix.matrix[newy * matrix.width + newx + 1] += matrix.matrix[newy * matrix.width + newx] / 4;
                    matrix.matrix[newy * matrix.width + newx - matrix.width] += matrix.matrix[newy * matrix.width + newx] / 4;
                    matrix.matrix[newy * matrix.width + newx - 1] += matrix.matrix[newy * matrix.width + newx] / 4;

                    matrix.matrix[newy * matrix.width + newx] %= 4;

                    flag = true;
                    
                    ++iterator;
                    
                    if (maxIter != -1) {
                        if (iterator >= maxIter) return;
                    }
                    if (freq != -1) {
                        if (iterator % freq == 0) BMPmaker(matrix, std::to_string(iterator) + ".bmp");
                    }
                    //std::cout << matrix << "\n\n\n\n\n";
                }
            }
        }
    }
}

int main(int argc, char* argv[]) {
    Arguments arguments = ArgumentParser(argc, argv);
    int width = 0, height = 0;

    GetMaxCoordinates(width, height, arguments.input);

    uint64_t *mat = new uint64_t[width * height];
    Matrix matrix = {mat, width, height};

    GetSand(matrix.matrix, width, height, arguments.input);

    SandFall(matrix, arguments.maxIter, arguments.freq);
    
    BMPmaker(matrix, std::string(arguments.output) + ".bmp");
    
    //rm -rf *.bmp ----- спасение от большой ошибки

    return 0;
}