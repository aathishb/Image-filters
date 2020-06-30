#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <math.h>
#include "stb_image.h"
#include "stb_image_write.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_exit_clicked()
{
    QApplication::quit();
}

void MainWindow::on_apply_clicked()
{
    // Remember filenames
    QString inf = ui->image_name->text();
    QString outf = ui->image_rename->text();
    if (inf == NULL || outf == NULL)
    {
        QMessageBox::warning(this, "", "Specify a path");
        return;
    }

    if (!ui->grey->isChecked() && !ui->sepia->isChecked() && !ui->reflect->isChecked() &&
        !ui->reflectV->isChecked() && !ui->blur->isChecked() && !ui->edge->isChecked())
    {
        QMessageBox::warning(this, "", "Choose a filter");
        return;
    }

    QByteArray in = inf.toLatin1();
    QByteArray out = outf.toLatin1();
    char* infile = in.data();
    char* outfile = out.data();

    // Check extension


    // Create input image
    int width, height, bpp;
    uint8_t* image = stbi_load(infile, &width, &height, &bpp, 3);
    if (image == NULL)
    {
        QMessageBox::critical(this, "", "Unable to open file");
        return;
    }

    // Create output image
    int size = width * height * 3;
    uint8_t* newimage = new uint8_t[size];
    if (newimage == NULL)
    {
        delete[] image;
        delete[] newimage;
        QMessageBox::critical(this, "", "Unable to create file");
        return;
    }

    char* extension = new char[3];

    // Filter image
    if (ui->grey->isChecked())
    {
        int avg;
        for (int i = 0; i < size-2; i+=3)
        {
            avg = round((image[i] + image[i+1] + image[i+2])/3);
            newimage[i] = avg;
            newimage[i+1] = avg;
            newimage[i+2] = avg;
        }
    }

    else if (ui->sepia->isChecked())
    {
        int r, g, b;
        for (int i = 0; i < size-2; i+=3)
        {
            r = round(0.189*image[i+2] + 0.769*image[i+1] + 0.393*image[i]);
            g = round(0.168*image[i+2] + 0.686*image[i+1] + 0.349*image[i]);
            b = round(0.131*image[i+2] + 0.534*image[i+1] + 0.272*image[i]);
            if (r > 255) r = 255;
            if (g > 255) g = 255;
            if (b > 255) b = 255;
            newimage[i] = r;
            newimage[i+1] = g;
            newimage[i+2] = b;
        }
    }

    else if (ui->reflect->isChecked())
    {
        // 2D array is needed to represent image
        uint8_t** matrix = new uint8_t*[height];
        for (int i = 0; i < height; i++)
        {
            matrix[i] = new uint8_t[width*3];
        }

        int c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                matrix[i][j] = image[c];
                c++;
            }
        }

        int half, k;
        uint8_t r, g, b;
        if ((width*3)%2 == 0) half = (width*3)/2;
        else half = round((width*3)/2)-1;
        for (int i = 0; i < height; i++)
        {
            k = (3*width) - 3;
            for (int j = 0; j < half-2; j+=3)
            {
                r = matrix[i][j];
                g = matrix[i][j+1];
                b = matrix[i][j+2];

                matrix[i][j] = matrix[i][k];
                matrix[i][j+1] = matrix[i][k+1];
                matrix[i][j+2] = matrix[i][k+2];

                matrix[i][k] = r;
                matrix[i][k+1] = g;
                matrix[i][k+2] = b;

                k-=3;
            }
        }

        c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                newimage[c] = matrix[i][j];
                c++;
            }
        }

        for (int i = 0; i < height; i++)
        {
            delete[] matrix[i];
            matrix[i] = nullptr;
        }
        delete[] matrix;
        matrix = nullptr;
    }

    else if (ui->reflectV->isChecked())
    {
        // 2D array is needed to represent image
        uint8_t** matrix = new uint8_t*[height];
        for (int i = 0; i < height; i++)
        {
            matrix[i] = new uint8_t[width*3];
        }

        int c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                matrix[i][j] = image[c];
                c++;
            }
        }

        int half;
        uint8_t r, g, b;
        if (height%2 == 0) half = height/2;
        else half = round(height/2);
        for (int i = 0; i < half; i++)
        {
            for (int j = 0; j < (width*3)-2; j+=3)
            {
                r = matrix[i][j];
                g = matrix[i][j+1];
                b = matrix[i][j+2];

                matrix[i][j] = matrix[height-i-1][j];
                matrix[i][j+1] = matrix[height-i-1][j+1];
                matrix[i][j+2] = matrix[height-i-1][j+2];

                matrix[height-i-1][j] = r;
                matrix[height-i-1][j+1] = g;
                matrix[height-i-1][j+2] = b;
            }
        }

        c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                newimage[c] = matrix[i][j];
                c++;
            }
        }

        for (int i = 0; i < height; i++)
        {
            delete[] matrix[i];
            matrix[i] = nullptr;
        }
        delete[] matrix;
        matrix = nullptr;
    }

    else if (ui->blur->isChecked())
    {
        // 2D array is needed to represent image
        uint8_t** matrix = new uint8_t*[height];
        for (int i = 0; i < height; i++)
        {
            matrix[i] = new uint8_t[width*3];
        }

        int c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                matrix[i][j] = image[c];
                c++;
            }
        }

        int w = (width*3)-2;
        uint8_t r, g, b;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < w; j+=3)
            {
                if (i == 0 && j == 0)
                {

                }
                else if (i == 0 && (j > 0 && j < w-1))
                {

                }
                else if (i == 0 && j == w-1)
                {

                }
                else if ((i > 0 && i < height-1) && j == 0)
                {

                }
                else if ((i > 0 && i < height-1) && (j > 0 && j < w-1))
                {

                }
                else if ((i > 0 && i < height-1) && j == w-1)
                {

                }
                else if (i == height-1 && j == 0)
                {

                }
                else if (i == height-1 && (j > 0 && j < w-1))
                {

                }
                else if (i == height-1 && j == w-1)
                {

                }
            }
        }

        c = 0;
        for (int i = 0; i < height; i++)
        {
            for (int j = 0; j < width*3; j++)
            {
                newimage[c] = matrix[i][j];
                c++;
            }
        }

        for (int i = 0; i < height; i++)
        {
            delete[] matrix[i];
            matrix[i] = nullptr;
        }
        delete[] matrix;
        matrix = nullptr;
    }

    else
    {

    }

    // Write to output file
    stbi_write_png(outfile, width, height, 3, newimage, width*3);

    delete[] extension;
    extension = nullptr;
    delete[] newimage;
    newimage = nullptr;
    stbi_image_free(image);
    QMessageBox::information(this, "", "Successfully filtered image");
    return;
}
