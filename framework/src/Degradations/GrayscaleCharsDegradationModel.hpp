
#include <vector>

#include <opencv2/core/core.hpp>

#include <QImage>
#include <QObject>

#include <framework_global.h>

/**
  * \mainpage
  *
  * \author kvcuong https://bitbucket.org/kvcuong
  *
  * - We propose a local noise model for GRAYSCALE images. Its main principle is to locally degrade the image in the
  * neighbourhoods of “seed-points” selected close to the character boundary. These points define the center of
  * “noise regions”. The pixel values inside the noise region are modified by a Gaussian random distribution
  * to make the final result more realistic. While Kanungo noise models scanning artifacts, our model simulates
  * degradations due to the age of the document itself and printing/writing process such as ink splotches, white
  * specks or streaks. It is very easy for users to parameterize and create a set of benchmark databases with
  * an increasing level of noise. These databases will further be used to test the robustness of different grayscale
  * document image analysis methods (i.e. text line segmentation, OCR, handwriting recognition).
  *
  * - License LGLPL
  *
  *    - [number of noise regions  nbp = 100, % of independent spots m = 30, % of overlapping spots n = 40, % of disconnection spots l = 30]
  */
const int WHITE = 255;
const int BLACK = 0;
const float PERCENT = 0.01f;

struct Pixel
{
  cv::Point pos;
  float distanceToEdge;
  float probability;
  float gradient_value;
  float gradient_angle;
  bool isBackground;
  
  Pixel(int x=0, int y=0) :
    pos(x, y), 
    distanceToEdge(0),
    probability(-1),
    gradient_value(0),
    gradient_angle(0),
    isBackground(true)
  {}
    
};

struct Seedpoint
{
  Seedpoint(const Pixel &p = Pixel(),
	    float sz = 0.f,
	    float tache = -1.f,
	    float cheval = -1.f,
	    float diffusion = FLT_MAX,
	    int t = -1) :
    pixel(p), 
    size(sz), 
    b_tache(tache),
    b_cheval(cheval),
    b_diffusion(diffusion),
    type(t)
  {}

  Pixel pixel;
  float size;
  float b_tache;
  float b_cheval;
  float b_diffusion;
  int type;
};


class FRAMEWORK_EXPORT GrayscaleCharsDegradationModel
{
public:
    explicit GrayscaleCharsDegradationModel(const QImage &img);
    explicit GrayscaleCharsDegradationModel(const cv::Mat &img);
    ~GrayscaleCharsDegradationModel();

    QImage degradate(int level = 1, float I = 33, float O = 33, float D = 34);
    QImage degradateByLevel(int level);
  

    cv::Mat degradate_cv(int level = 1, float I = 33, float O = 33, float D = 34);
    cv::Mat degradateByLevel_cv(int level);
    cv::Mat getImageGray_cv();
    cv::Mat getImageDegraded_cv();

private:

    int getMinStrokeWidthAtASeedPoint(const Seedpoint &sp, int &angle, cv::Point &A, cv::Point &B) const;

    float getAvgStrokeWidth() const;

    int getNumberOfConnectedComponants() const;
    void initialize(const QImage &input);
    void initialize(const cv::Mat &imgInput);

    void calculateNoiseRegionType(float b0);
    void calculateNoiseRegionType();

    void grayscaleDegradationByTypes();
  //void RapidlyGrayscaleDegradationByTypes();
    void separateSeedPointsByTypes(float percent_cheval, float percent_diffusion);
    void assignmentSizeOfNoiseRegion();


    void extractConnectedComponantInfos();

    int calculateDistanceFromBord(const Pixel &pixel) const;
    int calculateApproximatelyDistanceFromBord(const Pixel &pixel) const;

    std::vector<float*> getEllipsePoints(cv::Rect &rec,int &max_B, int &min_B, cv::Point centre, double semi_minor_axis, double semi_major_axis, double theta);
  //std::vector<cv::Point> getLinePoints(cv::Point A, cv::Point B) const;

    void DegradedLine(const float* Line, cv::Point Centre, int Centre_gray, int pixel_signma, int sigma_gaussien,
                      cv::Mat &imgGrayOutput, bool isFtoB);
    void RapidlyDegradeLine(const float* Line, cv::Point Centre, int Centre_gray, cv::Mat &imgGrayOutput);
 
  //unsigned int* calHist(cv::Mat src);

    cv::Mat binarize();

    void flipPixelsByProbability(float percent_independent);
    void calculatePixelsProbability();

  //int getTypeNoiseRegion(const std::vector<cv::Point> &listElipseEdgePoints);

    void calculatePixelsDistanceTransform();

private:

    bool _isTest;

    //global parameters
    cv::Mat _mat_output;
    cv::Mat _mat_input;
    cv::Mat _mat_gray;
    cv::Mat _mat_binary;

    //=========DEBUG================
    //cv::Mat _mat_color;

    cv::Mat _mat_reduce_pixels;
    cv::Mat _mat_CCs;
    cv::Mat _mat_contour;
    int _width, _height;
    int _avgBackground, _avgForeground, _nb_connectedcomponants, _MAX_a0;
    // Kanungo parameters
    double _alpha, _beta, _alpha0, _beta0, _nf, _nb;
    // Grayscale Model parameters
    float _a0, _g, _MAX_Gradient, _AVG_Gradient, _sigma_gausien;
    bool _is4connected;
    std::vector<Seedpoint> _listSeedPoints;
    std::vector<Pixel> _listPixels;

    std::vector<std::vector<cv::Point> > _contours;
    // connected component
    //std::vector<CC*> _listCCs;
    float _Mean_CC_Distance, _Max_CC_Distance;
    float _Mean_CC_Stroke, _Max_CC_Stroke;
    float _Total_Degradation;
    int _nbSPs_User;
    int _local_zone;

};
