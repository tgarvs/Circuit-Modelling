

#include "MNA.h"

float MNA::process_sample(float n){
    b(1, 2) = n;
    x = (((H - G)*x) + b + b_delay)/A_inv;
    b_delay = b;
    
    return x(1, 2);
}


void MNA::prepare(float sr){
    if(sr != samp_rate){
        samp_rate = sr;
        update_coefficients();
    }
}


void MNA::set_knobs(float capacitor, float resistor){
    if(capacitor != Cap){
        Cap = capacitor;
        update_coefficients();
    }
    
    if(resistor != Res){
        Res = resistor;
        update_coefficients();
    }
}


void MNA::update_coefficients(){
    //lol this is def not the best way to do this and im pretty sure its not memory safe
    G = Eigen::Matrix<float, 3, 3> {{1/Res, -1/Res, 1},
        {-1/Res, 1/Res, 0},
        {1, 0, 0}};
    
    C = Eigen::Matrix<float, 3, 3> {{0, 0, 0},
        {0, Cap, 0},
        {0, 0, 0}};
    
    H = (2*C)/T;
    A = G + H;
    A_inv = A.inverse();
    T = 1/samp_rate;

}
