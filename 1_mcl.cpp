//#include "src/matplotlibcpp.h"//Graph Library
#include <iostream>
#include <string>
#include <math.h> // M_PI
#include <vector>
#include <stdexcept> // throw errors
#include <random> //C++ 11 Random Numbers

//namespace plt = matplotlibcpp;
// using namespace std;

// Landmarks
// -------------------------
// --- o ----- o ----- o ---
// -------------------------
// -------------------------
// --- o ------------- o ---
// -------------------------
// -------------------------
// --- o ----- o ----- o ---
// -------------------------
double landmarks[8][2] = {  
            { 20.0, 20.0 }, 
            { 20.0, 80.0 }, 
            { 20.0, 50.0 },
            { 50.0, 20.0 }, 
            { 50.0, 80.0 }, 
            { 80.0, 80.0 },
            { 80.0, 20.0 }, 
            { 80.0, 50.0 } 
        };

// Map size in meters
double world_size = 100.0;

// Random Generators
std::random_device rd;
// std::mt19937 is a random value Generator
// need to cooperate with distribution function
std::mt19937 gen( rd() ); 

// Global Functions
double mod(double first_term, double second_term);
double gen_real_random();

class Robot {
public:
    Robot()
    {
        // Constructor
        x = gen_real_random() * world_size; // robot's x coordinate
        y = gen_real_random() * world_size; // robot's y coordinate
        orient = gen_real_random() * 2.0 * M_PI; // robot's orientation

        forward_noise = 0.0; //noise of the forward movement
        turn_noise = 0.0; //noise of the turn
        sense_noise = 0.0; //noise of the sensing
    }

    void set(double new_x, double new_y, double new_orient)
    {
        // Set robot new position and orientation
        if (new_x < 0 || new_x >= world_size)
            throw std::invalid_argument("X coordinate out of bound");
        if (new_y < 0 || new_y >= world_size)
            throw std::invalid_argument("Y coordinate out of bound");
        if (new_orient < 0 || new_orient >= 2 * M_PI)
            throw std::invalid_argument("Orientation must be in [0..2pi]");

        x = new_x;
        y = new_y;
        orient = new_orient;
    }

    void set_noise(double new_forward_noise, double new_turn_noise, double new_sense_noise)
    {
        // Simulate noise, often useful in particle filters
        forward_noise = new_forward_noise;
        turn_noise = new_turn_noise;
        sense_noise = new_sense_noise;
    }

    std::vector<double> sense()
    {
        // Measure the distances from the robot toward the landmarks
        std::vector<double> z(sizeof(landmarks) / sizeof(landmarks[0]));
        double dist;

        for (int i = 0; i < sizeof(landmarks) / sizeof(landmarks[0]); i++) {
            dist = sqrt(pow((x - landmarks[i][0]), 2) + pow((y - landmarks[i][1]), 2));
            dist += gen_gauss_random(0.0, sense_noise);
            z[i] = dist;
        }
        return z;
    }

    Robot move(double turn, double forward)
    {
        if (forward < 0)
            throw std::invalid_argument("Robot cannot move backward");

        // turn, and add randomness to the turning command
        orient = orient + turn + gen_gauss_random(0.0, turn_noise);
        orient = mod(orient, 2 * M_PI);

        // move, and add randomness to the motion command
        double dist = forward + gen_gauss_random(0.0, forward_noise);
        x = x + (cos(orient) * dist);
        y = y + (sin(orient) * dist);

        // cyclic truncate
        x = mod(x, world_size);
        y = mod(y, world_size);

        // set particle
        Robot res;
        res.set(x, y, orient);
        res.set_noise(forward_noise, turn_noise, sense_noise);

        return res;
    }

    std::string show_pose()
    {
        // Returns the robot current position and orientation in a string format
        return "[x=" + std::to_string(x) + " y=" + std::to_string(y) + " orient=" + std::to_string(orient) + "]";
    }

    std::string read_sensors()
    {
        // Returns all the distances from the robot toward the landmarks
        std::vector<double> z = sense();
        std::string readings = "[";
        for (int i = 0; i < z.size(); i++) {
            readings += std::to_string(z[i]) + " ";
        }
        readings[readings.size() - 1] = ']';

        return readings;
    }

    double measurement_prob(std::vector<double> measurement)
    {
        /*
         * we have to calculate the probability of current position to each Landmarks
         * probability density function of Landmark_0 ( pdf_Landmark_0 ) :
         * mu   = distance between current position and Landmark_0
         * sigma = sense_noise
         * probability_i = pdf_Landmark_0(x_i)
         * x_i = measurement distance of Landmark_i = get from sense() function, means from sensor
         * p_i = pdf_Landmark_0(x_i)
         */

        // Calculates how likely a measurement should be
        double prob = 1.0;
        double dist;

        for (int i = 0; i < sizeof(landmarks) / sizeof(landmarks[0]); i++) {
            // Calculate distance between current position and landmarks
            dist = sqrt(pow((x - landmarks[i][0]), 2) + pow((y - landmarks[i][1]), 2));

            // dist=mu, sense_noise = sigma
            // measurement[i] = z = x
            // probability = p(x), p is Gaussian distribution
            prob *= gaussian(dist, sense_noise, measurement[i]);
        }

        return prob;
    }

    double x, y, orient; //robot poses
    double forward_noise, turn_noise, sense_noise; //robot noises

private:
    double gen_gauss_random(double mean, double variance)
    {
        // Gaussian random
        std::normal_distribution<double> gauss_dist(mean, variance);
        return gauss_dist(gen);
    }

    double gaussian(double mu, double sigma, double x)
    {
        // Probability of x for 1-dim Gaussian with mean mu and var. sigma
        return exp(-(pow((mu - x), 2)) / (pow(sigma, 2)) / 2.0) / sqrt(2.0 * M_PI * (pow(sigma, 2)));
    }
};

// Functions
double gen_real_random()
{
    // Generate real random between 0 and 1
    std::uniform_real_distribution<double> real_dist(0.0, 1.0); //Real
    return real_dist(gen);
}

double mod(double first_term, double second_term)
{
    // Compute the modulus
    return first_term - (second_term)*floor(first_term / (second_term));
}

double evaluation(Robot r, Robot p[], int n)
{
    //Calculate the mean error of the system
    double sum = 0.0;
    for (int i = 0; i < n; i++) {
        //the second part is because of world's cyclicity
        double dx = mod((p[i].x - r.x + (world_size / 2.0)), world_size) - (world_size / 2.0);
        double dy = mod((p[i].y - r.y + (world_size / 2.0)), world_size) - (world_size / 2.0);
        double err = sqrt(pow(dx, 2) + pow(dy, 2));
        sum += err;
    }
    return sum / n;
}
double max(double arr[], int n)
{
    // Identify the max element in an array
    double max = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] > max)
            max = arr[i];
    }
    return max;
}
/*
void visualization(int n, Robot robot, int step, Robot p[], Robot pr[])
{
	//Draw the robot, landmarks, particles and resampled particles on a graph
	
    //Graph Format
    plt::title("MCL, step " + to_string(step));
    plt::xlim(0, 100);
    plt::ylim(0, 100);

    //Draw particles in green
    for (int i = 0; i < n; i++) {
        plt::plot({ p[i].x }, { p[i].y }, "go");
    }

    //Draw resampled particles in yellow
    for (int i = 0; i < n; i++) {
        plt::plot({ pr[i].x }, { pr[i].y }, "yo");
    }

    //Draw landmarks in red
    for (int i = 0; i < sizeof(landmarks) / sizeof(landmarks[0]); i++) {
        plt::plot({ landmarks[i][0] }, { landmarks[i][1] }, "ro");
    }
    
    //Draw robot position in blue
    plt::plot({ robot.x }, { robot.y }, "bo");

	//Save the image and close the plot
    plt::save("./Images/Step" + to_string(step) + ".png");
    plt::clf();
}
*/

//####   DON'T MODIFY ANYTHING ABOVE HERE! ENTER CODE BELOW ####

void test1()
{
    // Instantiating a robot object from the Robot class
    Robot myrobot;

    // TODO: Set robot new position to x=10.0, y=10.0 and orientation=0
    // Fill in the position and orientation values in myrobot.set() function
    myrobot.set(10, 10, 0);

    // Printing out the new robot position and orientation
    std::cout << myrobot.show_pose() << std::endl;

    // TODO: Rotate the robot by PI/2.0 and then move him forward by 10.0
    // Use M_PI for the pi value
    myrobot.move(M_PI/2.0, 10.0);

    // TODO: Print out the new robot position and orientation
    std::cout << myrobot.show_pose() << '\n';

    // Printing the distance from the robot toward the eight landmarks
    std::cout << myrobot.read_sensors() << std::endl;
}

void test2()
{
    // TODO: Instantiate a robot object from the Robot class
    Robot myrobot;

    // TODO: Set robot new position to x=30.0, y=50.0 and orientation=PI/2
    myrobot.set(30.0, 50.0, M_PI/2.0);

    // TODO: Turn clockwise by PI/2 and move by 15 meters
    myrobot.move(-M_PI/2.0, 15);

    // TODO: Print the distance from the robot toward the eight landmarks
    std::cout << myrobot.read_sensors() << '\n';

    // TODO: Turn clockwise by PI/2 and move by 10 meters
    myrobot.move(-M_PI/2.0, 10);

    // TODO: Print the distance from the robot toward the eight landmarks
    std::cout << myrobot.read_sensors() << '\n';
}

void test3()
{
    Robot myrobot;
    // TODO: Simulate Noise
    // Forward Noise=5.0, Turn Noise=0.1,Sense Noise=5.0
    double Forward_Noise=5.0, Turn_Noise=0.1, Sense_Noise=5.0;
    myrobot.set_noise(Forward_Noise, Turn_Noise, Sense_Noise);
    
    myrobot.set(30.0, 50.0, M_PI / 2.0);
    myrobot.move(-M_PI / 2.0, 15.0);
    std::cout << myrobot.read_sensors() << std::endl;
    myrobot.move(-M_PI / 2.0, 10.0);
    std::cout << myrobot.read_sensors() << std::endl;
}

void generate_particle()
{
    //Practice Interfacing with Robot Class
    Robot myrobot;
    myrobot.set_noise(5.0, 0.1, 5.0);
    myrobot.set(30.0, 50.0, M_PI / 2.0);
    myrobot.move(-M_PI / 2.0, 15.0);
    //cout << myrobot.read_sensors() << endl;
    myrobot.move(-M_PI / 2.0, 10.0);
    //cout << myrobot.read_sensors() << endl;

    //####   DON'T MODIFY ANYTHING ABOVE HERE! ENTER CODE BELOW ####

    // Instantiating 1000 Particles each with a random position and orientation
    int n = 1000;
    Robot p[n];
    //TODO: Your job is to loop over the set of particles
    //TODO: For each particle add noise: Forward_Noise=0.05, Turn_Noise=0.05, and Sense_Noise=5.0
    //TODO: And print its pose on a single line
    double Forward_Noise=0.05, Turn_Noise=0.05, Sense_Noise=5.0;
    for(size_t i=0; i < 1000;++i)
    {
        p[i] = Robot{};
        p[i].set_noise(Forward_Noise, Turn_Noise, Sense_Noise);

        double x = gen_real_random() * 100;
        double y = gen_real_random() * 100;
        double turn = gen_real_random() * M_PI;
        p[i].set(x, y, turn);
        // std::cout << p[i].show_pose() << '\n';        
    }

    for(Robot& r : p)
    {
        std::cout << r.show_pose() << '\n';
    }
    
}

void simulate_motion()
{
    // Create a set of particles
    int n = 1000;
    Robot p[n];

    for (int i = 0; i < n; i++) {
        p[i].set_noise(0.05, 0.05, 5.0);
        //cout << p[i].show_pose() << endl;
    }

    //Now, simulate motion for each particle
    //TODO: Create a new particle set 'p2'
    //TODO: Rotate each particle by 0.1 and move it forward by 5.0
    //TODO: Assign 'p2' to 'p' and print the particle poses, each on a single line
    Robot p2[n];
    for(size_t i=0; i < 1000;++i)
    {
        p2[i] = p[i].move(0.1, 5.0);
        p[i] = p2[i];
        std::cout << p[i].show_pose() << '\n';
    }
}

void importance_weight()
{
    //Practice Interfacing with Robot Class
    Robot myrobot;
    myrobot.set_noise(5.0, 0.1, 5.0);
    myrobot.set(30.0, 50.0, M_PI / 2.0);
    myrobot.move(-M_PI / 2.0, 15.0);
    //cout << myrobot.read_sensors() << endl;
    myrobot.move(-M_PI / 2.0, 10.0);
    //cout << myrobot.read_sensors() << endl;

    // Create a set of particles
    int n = 1000;
    Robot p[n];
    for (int i = 0; i < n; i++) {
        p[i].set_noise(0.05, 0.05, 5.0);
        //cout << p[i].show_pose() << endl;
    }

    //Re-initialize myrobot object and Initialize a measurment vector
    myrobot = Robot();
    std::vector<double> z;

    //Move the robot and sense the environment afterwards
    myrobot = myrobot.move(0.1, 5.0);
    z = myrobot.sense();

    // Simulate a robot motion for each of these particles
    Robot p2[n];
    for (int i = 0; i < n; i++) {
        p2[i] = p[i].move(0.1, 5.0);
        p[i] = p2[i];
    }

    //####   DON'T MODIFY ANYTHING ABOVE HERE! ENTER CODE BELOW ####

    //TODO: Generate particle weights depending on robot's measurement
    //TODO: Print particle weights, each on a single line
    double w[n];
    for(size_t i=0; i < n; ++i)
    {
        
        w[i] = p[i].measurement_prob(z);
        std::cout << w[i] << '\n';
    }

}


int main()
{
    // test1();
    // test2();
    // test3();
    // generate_particle();
    // simulate_motion();
    importance_weight();


    



    

    return 0;
}