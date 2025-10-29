/*
  ==============================================================================

    DK Method.h
    Created: 20 Oct 2025 2:07:08pm
    Author:  Thomas Garvey

  ==============================================================================
*/

#pragma once


class DKMethod {
    
public:
    
    float process_sample(float x);
    void prepare (float newFs);
    void setKnobs(float res, float cap);
    
private:
    
    void update_coefficients();
    
    float R = 10000.0f;
    float C = 10000.0f;
    float X = 0.0f;
    float fs = 44100.f;
    float Z = 1/(2* fs * C);
    
};
