

#pragma once
#include <Eigen/Dense>


class MNA {
    
public:
    MNA() = default;
    
    float process_sample(float n);
    void prepare(float sr);
    void set_knobs(float capacitor, float resistor);
    
private:
    
    void update_coefficients();
    
    //RC values
    float Res = 10000.f;
    float Cap = 10000.f;
    
    //Extra things
    float samp_rate = 44100;
    float T = 1/samp_rate;
    float Vs = 0; //this will be our input
    float n_delay = 0;
    
    //Matrices
    Eigen::Matrix<float, 3, 3> G {
        {1/Res, -1/Res, 1},
        {-1/Res, 1/Res, 0},
        {1, 0, 0}};
    
    Eigen::Matrix<float, 1, 3> b {
        {0},
        {0},
        {0}}; //This is Vs, our initial input
    
    Eigen::Matrix<float, 1, 3> b_delay {
        {0},
        {0},
        {0}}; //This is Vs, our initial input
    
    Eigen::Matrix<float, 1, 3> x { //the unknown vector...Vb (or the second on in the vector) will be our Vout
        {0}, //start with all zeroes
        {0},
        {0}};
    
    Eigen::Matrix<float, 3, 3> C {
        {0, 0, 0},
        {0, Cap, 0},
        {0, 0, 0}};
    
    Eigen::Matrix<float, 3, 3> H = (2*C)/T;
    Eigen::Matrix<float, 3, 3> A = G + H;
    Eigen::Matrix<float, 3, 3> A_inv = A.inverse();
};
