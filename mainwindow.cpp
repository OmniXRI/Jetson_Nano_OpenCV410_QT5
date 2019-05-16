#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->pbxS->setScaledContents(true); // 令顯示於QLabel上影像強制縮放到和物件尺寸相同
}

MainWindow::~MainWindow()
{
    delete ui;
}

// 將cv::Mat影像顯示於QLabel上
void MainWindow::ShowFrame(cv::Mat imgS, QLabel *imgT)
{
    Mat imgC;

    cvtColor(imgS,imgC,cv::COLOR_BGR2RGB); // 將影像色彩空間從BGR轉成RGB

    QImage tmp(imgC.data, imgC.cols, imgC.rows, imgC.step,
               QImage::Format_RGB888); // 生成QImage格式暫存影像

    imgT->setPixmap(QPixmap::fromImage(tmp)); // 將影像繪至QLabel上
}

// 按下【Load...】鍵
void MainWindow::on_btnLoad_clicked()
{
    fileName = QFileDialog::getOpenFileName(this,tr("Open File")); // 開啟開檔對話盒並取得檔名
    QWidget::setWindowTitle (fileName); // 設定視窗標題為開啟檔案名稱

    imgSrc = cv::imread(fileName.toStdString(),IMREAD_COLOR); // 讀取影像檔到cv:Mat

     if(!imgSrc.empty()) { // 若檔案不空
         ShowFrame(imgSrc, ui->pbxS); // 將cv::Mat影像顯示於QLabel上
         ui->btnBilateral->setEnabled(true); // 令【Bilateral】按鍵致能
         ui->btnCuda->setEnabled(true); // 令【Cuda】按鍵致能
     }
     else{ // 若檔案不存在
         ui->btnBilateral->setEnabled(false);  // 令【Bilateral】按鍵禁能
         ui->btnCuda->setEnabled(false); // 令【Cuda】按鍵禁能
         QMessageBox::critical(NULL, "Error", "File not exist!"); // 開檔失敗顯示錯誤訊息
         return;
    }
}

// 按下【Bilatral】雙向濾波按鍵(CPU計算)
void MainWindow::on_btnBilateral_clicked()
{
    double t1=(double)getTickCount(); // 取得目前時間(Tick)

    cv::bilateralFilter(imgSrc, imgTrg, 33, 60, 60); // 以CPU計算雙向濾波結果

    double t2=(double)getTickCount(); // 取得目前時間(Tick)
    double diff_t = (t2 - t1) / getTickFrequency() * 1000; // 將時間差轉成ms

    ui->labStatus->setText("Status : " + QString::number(diff_t,'f',3) + " ms"); // 顯示時間差於狀態字串
    ShowFrame(imgTrg, ui->pbxS); // 將cv::Mat影像顯示於QLabel上
}

// 按下【Cuda】雙向濾波按鍵(GPU計算)
void MainWindow::on_btnCuda_clicked()
{
    int num_devices = cv::cuda::getCudaEnabledDeviceCount(); // 檢查系統上GPU裝置數量

    ui->labStatus->setText("Status : Cuda= " + QString::number(num_devices)); // 顯示GPU裝置數量狀態字串上

    if(num_devices > 0){ // 若裝置數量大於0
        cv::cuda::DeviceInfo dev_info(0); // 取得第0組GPU裝置資訊
        int enable_device_id = -1; // 設定GPU致能裝置代碼為-1

        if(dev_info.isCompatible()){ // 若GPU裝置為正常
            enable_device_id = 0; // 設定GPU致能裝置代碼為第0組
            cv::cuda::setDevice(0); // 設定第0組GPU裝置

            ui->labStatus->setText(ui->labStatus->text() +
                                   ", Name: " + QString(dev_info.name()) +
                                   ", ID: " + QString::number(enable_device_id)); // 顯示GPU名稱及代碼於狀態字串

            double t1=(double)getTickCount(); // 取得目前時間(Tick)

            cv::cuda::GpuMat gmatS(imgSrc); // 將cv::Mat影像傳入GPU記憶體區
            cv::cuda::GpuMat gmatT; // 準備存放GPU結果影像記

            double t2=(double)getTickCount(); // 取得目前時間(Tick)

            cv::cuda::bilateralFilter(gmatS, gmatT, 33, 60, 60); // 以CUDA(GPU)計算雙向濾波結果

            double t3=(double)getTickCount(); // 取得目前時間(Tick)

            gmatT.download(imgTrg); // 將GPU結果影像傳回cv::Mat

            double t4=(double)getTickCount(); // 取得目前時間(Tick)

            double diff_t1 = (t2 - t1) / getTickFrequency() * 1000; // 計算影像載入GPU時間
            double diff_t2 = (t3 - t2) / getTickFrequency() * 1000; // 計算GPU執行雙向濾波時間
            double diff_t3 = (t4 - t3) / getTickFrequency() * 1000; // 計算GPU傳回結果影像時間
            double diff_t4 = (t4 - t1) / getTickFrequency() * 1000; // 計算總共執行時間
            QString strResult = "Load Image to GPU = " + QString::number(diff_t1,'f',3) + " ms\n" +
                                "Cuda Bilateral = " + QString::number(diff_t2,'f',3) + " ms\n" +
                                "GPU to Mat = " + QString::number(diff_t3,'f',3) + " ms\n" +
                                "Total time = " + QString::number(diff_t4,'f',3) + " ms\n"; // 產生計算時間結果字串

            QMessageBox::information(this,"Infomation",strResult); // 利用訊息盒顯示結果字串
            ShowFrame(imgTrg, ui->pbxS); // 將cv::Mat影像顯示於QLabel上
        }
        else{
            ui->labStatus->setText("Status : "); // 清空狀態字串
            QMessageBox::critical(NULL, "Error", "GPU module isn't built for GPU!"); // 利用訊息盒顯示GPU驅動模組未正確建立訊息
        }
    }
}
