#define FRAMERATE 20

//--------------------------------------------------tmp stuff
#define DELAY_T 10  
#define PI 3.1415   
#define EPSILON 0.5
//------------------------------------
#define MAX_COUNT 150     
#define LEARNING_FRAMES 50

//MOG PARAMETERS
static int mog_history = 50;
static int nmixtures = 7;
static double backgroundRatio = 0.5;
static double noiseSigma = 0.05;
//---------------

//MOG2 PARAMETERS
static int mog2_history = 50;
static float varThreshold = 1.0;
static bool bShadowDetection = true;
//---------------
static double learning_rate = 0.01;

static double tau = 0.05; // smaller - faster
static double lambda = 0.15;
static double theta = 0.3;
static int nscales = 5;
static int warps = 3;
static double epsilon = 0.05;
static int iterations = 100;
static bool useInitialFlow = false;

static const int subblocks_x_amount = 3;
static const int subblocks_y_amount = 3;

static double cumulative_angle_deviation_max = 1015.5;
static double cumulative_angle_deviation_min = 0;

static double cumulative_travel_deviation_max = 1015.5;
static double cumulative_travel_deviation_min = 0;

static double angle_mean_max = 10000;
static double angle_mean_min = -100000;

static double travel_mean_max = 20;
static double travel_mean_min = 3;

static double cumulative_color_deviation_max = 63;
static double cumulative_color_deviation_min = 0;

static double current_local_color_deviation_max = 63;
static double current_local_color_deviation_min = 0;

static double current_local_h_deviation_max = 127;
static double current_local_h_deviation_min = 0;

static double current_local_v_deviation_max = 127;
static double current_local_v_deviation_min = 0;

static double current_local_d_deviation_max = 255;
static double current_local_d_deviation_min = 0;

//[0;1]
static double cumulative_h_deviation_max = 0.00125;
static double cumulative_h_deviation_min = 0.0;

static double cumulative_v_deviation_max = 0.00125;
static double cumulative_v_deviation_min = 0.0;

static double cumulative_d_deviation_max = 0.00125;
static double cumulative_d_deviation_min = 0.0;

static double local_h_mean_max = 0.55;
static double local_h_mean_min = 0.45;

static double local_v_mean_max = 0.55;
static double local_v_mean_min = 0.45;

static double local_d_mean_max = 255;
static double local_d_mean_min = 0;
//[0;255]
static double mask_mean_max = 256;
static double mask_mean_min = 66;


#define DEFAULT_BUFLEN 1024
#define DEFAULT_PORT "27015"
#define TIMEOUT 10000

static char* SIGTERM = "SIGTERM";
static char* SMOKE_ALARM = "SMOKE_ALARM";
static char* EndOfFile = "EOF";