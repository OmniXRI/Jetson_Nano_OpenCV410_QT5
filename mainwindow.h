#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>
#include <QLabel>
#include <QMessageBox>

#include <opencv.hpp>

using namespace std;
using namespace cv;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_btnLoad_clicked();
    void on_btnBilateral_clicked();
    void on_btnCuda_clicked();

private:
    void ShowFrame(cv::Mat imgS, QLabel *imgT); // 將cv::Mat顯示在QLabel上

    Ui::MainWindow *ui;

    QString fileName; // 開啟影像檔名
    Mat imgSrc; // 來源影像
    Mat imgTrg; // 目標影像
};

#endif // MAINWINDOW_H
