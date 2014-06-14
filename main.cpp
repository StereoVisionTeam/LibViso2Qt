#include <QCoreApplication>
#include <QObject>
#include <application.h>
using namespace std;

void calibrationOnline();
void odometryFramesFromFile(string odometryURL, int numberOfFrames);
void videoStreamingOdometry();

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  Application myApplication;

  return a.exec();
}

//Fragmenty poprzedniej wersji do wczytywania danych z pliku, jescze bez bibliioteki Qt
//int main()
//{
//  Application myApplication;
//  signal(SIGINT, breakLoop);


//    //Kalibracja
//    bool isCalibrationDone = false;

//    while(!isCalibrationDone){

//        char choiceCalibration=0;

//        while(choiceCalibration != '1' && choiceCalibration!= '2' && choiceCalibration != '3'){
//            std::cout<< " Przeprowadzenie kalibracji ..."<<std::endl;
//            std::cout<< " 1.Wczytanie pliku z danymi kalibracji"<<std::endl;
//            std::cout<< " 2.Kalibracja na podstawie zestawu zdjec"<<std::endl;
//            std::cout<< " 3.Kalibracja ze zdjęć zrobionych online "<<std::endl;
//            std::cin>>choiceCalibration;
//        }
//        switch(choiceCalibration){
//    //1.Wczytanie pliku z danymi kalibracji
//        case '1':{
//            string calibrationDataURL;
//            std::cout<< "Podaj sciezke do pliku"<<std::endl;
//            std::cin>>calibrationDataURL;

//            isCalibrationDone=core.loadCalibration(calibrationDataURL);
//            break;
//        }
//    //2.Kalibracja na podstawie zestawu zdjec
//        case '2':{
//            string calibrationDataURL;
//            string calibrationImagesURL;
//            std::cout<< "Podaj sciezke do pliku dla danych kalibracyjnych"<<std::endl;
//            std::cin>>calibrationDataURL;

//            std::cout<< "Podaj sciezke do folderu ze zdjeciami"<<std::endl;
//            std::cin>> calibrationImagesURL;

//            int numberOfPictures;
//            std::cout << "Podaj liczbe zdjec kalibracyjnych"<<std::endl;
//            std::cin>> numberOfPictures;
//            isCalibrationDone=core.calibrateFromImages(calibrationImagesURL,calibrationDataURL,numberOfPictures);

//            break;
//        }
//    //3.Kalibracja ze zdjęć zrobionych online
//        case '3':{
//            int x,y;
//            std::cout << "Podaj rozmiar szachownicy kalibracyjnej"<<std::endl;
//            std::cin >> x >> y;
//            core.setPatternSize(x,y);
//            calibrationOnline();
//            string calibrationDataURL;
//            std::cout<< "Podaj sciezke do pliku dla danych kalibracyjnych"<<std::endl;
//            std::cin>>calibrationDataURL;

//            isCalibrationDone=core.calibrateCamera(calibrationDataURL);
//            break;
//        }
//        }
//    }

//    //Odometria
//    char choiceOdometry;
//    std::cout<< " Odometria wizyjna ..."<<std::endl;
//    std::cout<< " 1.Odometria z gotowych zdjęć"<<std::endl;
//    std::cout<< " 2.Odometria ze streamu video"<<std::endl;
//    std::cin>>choiceOdometry;

//    string odometryFile;
//    std::cout<<"Podaj sciezke do pliku zapisu danych z odometrii"<<std::endl;
//    std::cin>>odometryFile;
//    core.fileWithOdometry.open(odometryFile.data());

//    if(core.fileWithOdometry.is_open())
//        std::cout<<"Zapisuję dane z odometrii"<<std::endl;
//    else
//        std::cout<<"Cant open odometry file"<<std::endl;

//    core.createVisualOdometryMonoObject();

//    switch(choiceOdometry){
//        //1.Odometria z gotowych zdjęć
//    case '1':{
//        string odometryURL;
//        std::cout << "Podaj sciezke do pliku ze zdjeciami Odometrii"<<std::endl;
//        std::cin >> odometryURL;
//        int numberOfFrames;
//        std::cout <<"Podaj liczbe zdjec"<<std::endl;
//        std::cin >> numberOfFrames;
//        odometryFramesFromFile(odometryURL,numberOfFrames);

//        break;
//    }
//        //2.Odometria ze streamu video
//    case '2':{
//        videoStreamingOdometry();
//        break;
//    }
//    }

//    cout<<"WYSZLO";
//    //Odometry File

//        core.fileWithOdometry.flush();
//        core.fileWithOdometry.close();

//    return 0;
//}
//void videoStreamingOdometry(){
//    bool replace = false;
//    cv::VideoCapture videoCapture(0);
//    if (!videoCapture.isOpened()){
//        std::cout  << "Could not open the input video: "<< std::endl;
//        return;
//    }
//    cv::Mat frame;
//    while(1){
//        if(stop ==true )
//            break;
//        videoCapture>>frame;
//        videoCapture>>frame;
//        videoCapture>>frame;
//        videoCapture>>frame;
//        videoCapture>>frame;

//        cvtColor(frame, frame, CV_RGB2GRAY);

//        core.setImageSize(frame.size());

//        if(!core.addImgToOdometry(frame,replace) && core.errorCode==ODOMETRY_ERROR_LITTLE_MOTION){
//            replace=true;
//        }
//        else
//            replace=false;

//        core.fileWithOdometry<<core.pose<<std::endl;
//    }
//}

//void odometryFramesFromFile(string odometryURL, int numberOfFrames){
//    cv::Mat frame;
//    bool replace = false;
//    for(int i=0;i<numberOfFrames;i++){
//        char base_name[256];
//        sprintf(base_name,"%06d.png",i);
//        string actualImageURL  = odometryURL +"/I1_" + base_name;

//        frame = cv::imread(actualImageURL,CV_LOAD_IMAGE_GRAYSCALE);
//        core.setImageSize(frame.size());
//        if(! frame.data ){
//            cout << "Could not open or find the image for odometry in (odometryFramesFromFile) " << std::endl ;
//            break;
//        }
//        if(!core.addImgToOdometry(frame,replace) && core.errorCode==ODOMETRY_ERROR_LITTLE_MOTION){
//            replace=true;
//        }
//        else
//            replace=false;

//        core.fileWithOdometry<<core.pose<<std::endl;
//    }
//}
//void calibrationOnline(){
//    cv::Mat frame;
//    char choice;
//    int countFrames = 0 ;

//    cv::VideoCapture videoCapture(0);
//    if (!videoCapture.isOpened()){
//        std::cout  << "Could not open the input video: "<< std::endl;
//        return;
//    }

//    while(true){
//        countFrames++;
//        std::cout<<"Zrobic kolejne zdjecie? [y]";
//        std::cin>>choice;
//        if(choice != 'y' && choice != 'Y')
//            break;
//        std::cout<<"A";
//        videoCapture >> frame;

//        cvtColor(frame,frame,CV_RGB2GRAY);
//        if(countFrames==1)
//            core.setImageSize(frame.size());

//        if(core.addSampleToCalibration(frame))
//            std::cout<<"GOOD";
//        else
//            std::cout<<"BAD";
//    }
//}
