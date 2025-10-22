/*
  ==============================================================================

    DK Method.cpp
    Created: 20 Oct 2025 2:07:08pm
    Author:  Thomas Garvey

  ==============================================================================
*/

#include "DK Method.h"


float DKMethod::process_sample(float n){
    float Vout = (n * (R/(R + Z))) + (X * (R/(R + Z)));
    X = ((2/Z)*Vout) - X;
    return Vout;
}


void DKMethod::prepare (float newFs){
    if(newFs != fs){
        fs = newFs;
        update_coefficients();
    }
}


void DKMethod::setKnobs(float cutoff){
    if(cutoff != C){
        C = cutoff;
        update_coefficients();
    }
}

void DKMethod::update_coefficients(){
    float Z = 1/(2* fs * C);
}
