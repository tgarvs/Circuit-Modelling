/*
  ==============================================================================

    MNA.h
    Created: 20 Oct 2025 2:07:00pm
    Author:  Thomas Garvey

  ==============================================================================
*/

#pragma once


class MNA {
    
public:
    MNA() = default;
    
    float process_sample(float n);
    void prepare(float sr);
    void set_knobs(float cutoff);
    
private:
    
    void update_coefficients();
    
    //RC values
    float Res = 10000.f;
    float Cap = 10000.f;
    
    //Matrices
    
    //G
    //b
    //x
    //C
    //A
    //A_inverse
    
    //Extra condensed things
    float samp_rate = 44100;
    float T = 1/samp_rate;
    //float H = (2*C)/T;
    
    
};
