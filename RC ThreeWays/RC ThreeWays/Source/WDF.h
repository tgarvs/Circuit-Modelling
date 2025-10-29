/*
  ==============================================================================

    WDF.h
    Created: 20 Oct 2025 2:07:15pm
    Author:  Thomas Garvey

  ==============================================================================
*/

#pragma once


//TODO: Make this ready to use with a knob in juce


/* WDF class needs reflected (b) and incident (a) wave, toVoltage/Current helper functions, calc impedences, getter functions
 *
 *
 */
class WDF {
    
public:
    
    WDF() = default;
    virtual ~WDF() = default;
    
    virtual void calc_impedences() = 0;
    virtual float reflected() = 0; //reflected wave will return a number to be passed to next component
    virtual void incident(float x) = 0; //incident wave requires the wave from the parent (x)
    
    //getter functions
    float get_R0 () { return R0; }
    float get_b () { return b; }
    float get_a () { return a; }
    
    //helper functions to transfer into K-Domain
    float toVoltage(){
        return (a + b)/2.0f;
    }

    float toCurrent(){
        return (a - b)/(2.0f * R0);
    }
    

    
protected:
    
    float R0 = 0.0f; //resistance
    float b = 0.0f; //reflected wave
    float a = 0.0f; //incident wave

    
};



/* Components subClasses!
 * Each needs a function to calculate impedance, pass the reflected wave, and pass the incident wave
 * Has only one child component
 */
class Resistor : public WDF {
  
public:
    
    Resistor(float r) : R{r} {};
    
    void calc_impedences() override {
        R0 = R;
    }
    
    float reflected() override {
        b = 0; //for resistors, the reflected wave is just 0
        return b;
    }
    
    void incident(float x) override {
        a = x; //pass the incident wave back up to the resistor port
    }
    
    float R; //actual resistor value
    
private:
    
};







class Capacitor : public WDF {
  
public:
    
    Capacitor(float c) : C{c} {}
    
    void calc_impedences() override {
        R0 = 1/(2.0f * fs * C);
    }
    
    float reflected() override {
        b = delayed_a; //set the reflected wave as the delayed [n-1] incident wave
        delayed_a = a; //updated the delayed state with the current incident wave
        return b;
    }
    
    void incident(float x) override {
        a = x; //update the incident wave without any changes --> there is no scattering since this is a one port
    }
    
    void update_sample_rate(float sr) {
        if(sr != fs){
            fs = sr;
        }
    }
    
    void reset_state () { delayed_a = 0; }
    
    float C; //actual capacitance value
    
private:
    float delayed_a = 0.0f; //initial value for the delayed wave
    float fs = 44100.f;
};


/* Root Node!
 * Need to calculate impedence (none), and turn the reflected wave into an incident wave
 *
 */
class VoltageSource : public WDF {
    
public:
    VoltageSource(float _vs) : Vin{_vs} {};
    void set_voltage_source (float _vs) { Vin = _vs; }
    
    void calc_impedences() override {
        //no impedence at voltage source
    }
    
    float reflected() override {
        b = 2*Vin - a; //calculate reflected wave
        return b;
    }
    
    void incident(float x) override {
        a = x; //
    }
    
private:
    float Vin; //voltage input
};




/* Series Adaptor
 * Needs scattering weights, port impedence, reflected wave to send to parent (root), and 2 incident waves to send children
 *
 */
class SeriesAdaptor : public WDF {
    
public:
    
    SeriesAdaptor(WDF& c1, WDF& c2) : child1(c1), child2(c2) {}
    
    void calc_impedences() override {
        R0 = child1.get_R0() + child2.get_R0(); //adaptor impedence is the combination of the two children
    }
    
    float reflected() override {
        b = -(child1.reflected() + child2.reflected());
        return b;
    }
    
    void incident(float x) override { //treating x like it is -a0 here
        const float w = x + child1.get_b() + child2.get_b();
        const auto port1ReflectedWeights = child1.get_b()/(child1.get_b() + child2.get_b());
        const auto port1_a = child1.get_b() - (port1ReflectedWeights * w);
        
        child1.incident(port1_a);
        child2.incident(-(x + port1_a));
    }
    
    
private:
    WDF& child1;
    WDF& child2;
    
};


/* Tying it all together!
 *
 *
 */
class RCLowPass {

public:
    //setup
    void prepare(float sr) {
        //set capacitor stuff
        cap.reset_state();
        cap.update_sample_rate(sr);
        
        //calculate initial impedences for EACH node
        res.calc_impedences();
        cap.calc_impedences();
        Vin.calc_impedences();
        adaptor.calc_impedences();
    }
    
    //process
    float process_sample(float input_voltage){
        //set Vin
        Vin.set_voltage_source(input_voltage);
        Vin.incident(adaptor.reflected());   // up: adaptor pulls from leaves
        adaptor.incident(Vin.reflected());   // down: adaptor pushes to leaves
        return cap.toVoltage();
    }
    
    void setKnobs(float newR, float newC){
        if(newR != res.R){
            res.R = newR;
            res.calc_impedences();
            adaptor.calc_impedences();
        }
        
        if(newC != cap.C){
            cap.C = newC;
            cap.calc_impedences();
            adaptor.calc_impedences();
        }
    }

    
private:
    void update_coefficients(){
        
    }
    
    //list out all component values
    Resistor res {10000};
    Capacitor cap {10000};
    VoltageSource Vin {5};
    SeriesAdaptor adaptor {res, cap}; //send the child nodes to series adaptor
    float initial_sr {44100};
};
