/*
  ==============================================================================

    WDF.h
    Created: 20 Oct 2025 2:07:15pm
    Author:  Thomas Garvey

  ==============================================================================
*/

#pragma once


/*
  ==============================================================================

    WDF.h
    Created: 20 Oct 2025 2:07:15pm
    Author:  Thomas Garvey

    OVERVIEW (read me first)
    ------------------------
    Wave Digital Filters (WDFs) are a way to simulate circuits using *waves*
    rather than directly using voltages and currents. At each one-port we keep:

        a = incident wave  (flowing from parent *into* this port)
        b = reflected wave (flowing from this port *back* to the parent)

    These are related to physical voltage/current by:
        v = (a + b)/2
        i = (a - b)/(2 * R0)

    where R0 is the "port resistance" assigned to that port. The whole algorithm
    for one sample proceeds in two passes:
      1) UP-SWEEP   : call reflected() bottom-up so each node produces its b.
      2) DOWN-SWEEP : call incident(a) top-down so each node receives its a.

    After both passes, each element has consistent (a,b) and you can read v, i.

  ==============================================================================
*/

#pragma once


class WDF {

public:
    WDF() = default;
    virtual ~WDF() = default;
    
    // Each concrete WDF subclass must:
    //  - calc_impedence(): decide/set its port resistance R0.
    //  - incident(x)    : accept an incident wave 'x' from its parent (down-sweep).
    //  - reflected()    : produce its reflected wave 'b' to send to the parent (up-sweep).
    //
    // Note: the spelling "impedence" is used consistently in your code.
    virtual void calc_impedence() = 0;
    virtual void incident(float x) = 0;
    virtual float reflected() = 0;
    
    // Accessors for port resistance and wave variables.
    float getR0() { return R0; }
    float getA() { return a; }
    float getB() { return b; }
    
    // Helper functions: convert wave variables to physical voltage/current.
    // From WDF identities:
    //   v = (a + b)/2
    float toVoltage(){
        return (a + b)/2.0f;
    }
    
    //   i = (a - b)/(2*R0)
    // Be careful if R0==0 (e.g., ideal source) — division by zero would occur.
    float toCurrent(){
        return (a - b)/(2.0f * R0);
    }
    
protected:
    float a {0.0f}; // incident wave arriving from the parent (down-sweep)
    float b {0.0f}; // reflected wave sent back to the parent (up-sweep)
    float R0 {0.0f}; // port resistance (WDF reference resistance for this port)
    
};



/* Component specific derived subclasses
 * Each will have their own calc_impedence, incident, and reflected functions
 *
 * The key idea: pick R0 so that scattering is simple and stable. Memoryless
 * elements like a matched resistor reflect nothing (b=0). Reactive elements
 * (C, L) introduce state (delay) into the wave relation.
 */
class Resistor : public WDF{
    
public:
    Resistor(float new_resistance) : R{new_resistance}{};
    
    void set_resistance(float new_resistance) { R = new_resistance; }
    
    // For a physical resistor, choose the port resistance equal to the resistor value.
    // This "matches" the port so it doesn't reflect energy (b=0).
    void calc_impedence() override
    {
        R0 = R;
    }
    
    // Down-sweep: parent supplies an incident wave a=x.
    void incident(float x) override
    {
        a = x;
    }
    
    // Up-sweep: a matched resistor reflects nothing (think "it absorbs the wave").
    // We still set b explicitly to keep internal state coherent for getB().
    float reflected() override
    {
        b = 0.0f;
        return b;
    }
    
private:
    float R; // physical resistance in Ohms
};


class Capacitor : public WDF {
    
public:
    Capacitor(float new_capacitance) : C{new_capacitance} {};
    ~Capacitor() = default;
    
    // The capacitor's discrete-time behavior depends on the sample rate.
    void set_sample_rate(float sample_rate) { fs = sample_rate; }
    void set_capacitance(float new_capacitance) { C = new_capacitance; }
    void reset_state() { z = 0.0f; }
    
    // Using the bilinear (Tustin) transform, a capacitor gets a port resistance:
    //     R0 = 1 / (2*fs*C)
    // Intuition: the reactive element is represented as a "resistive" port plus
    // a one-sample delay in the wave variable to capture energy storage.
    void calc_impedence() override
    {
        R0 = 1.0f / (2.0f * fs * C);
    }
    
    // Down-sweep: receive the incident wave from the parent network.
    void incident(float x) override
    {
        a = x;
    }
    
    // Up-sweep: produce the reflected wave.
    // For the bilinear capacitor one-port, the reflected wave is the *stored*
    // previous incident wave (z). Then update z to the current incident a.
    // This single-sample memory is the capacitor's state.
    float reflected() override
    {
        b = z;          // return stored wave (previous a)
        z = a;          // update state to current a for the next sample
        return b;
    }
    

private:
    float fs; // sample rate (Hz) — must be set before calc_impedence()
    float C;  // capacitance (F)
    float z;  // state (stores previous incident wave a)
    
};



/* Port Adaptors
 *
 * Adaptors connect one-ports together. This is where KCL/KVL are enforced in
 * the wave domain. They:
 *   - combine child reflected waves b1, b2 ... to produce their own b (up-sweep),
 *   - split an incident wave a into child incidents a1, a2 ... (down-sweep).
 *
 * This one is a *series* adaptor for two ports: currents are equal; voltages add.
 */

class SeriesAdaptor : public WDF {
public:
    // Pass in the two children ports that are connected in series.
    SeriesAdaptor(WDF* _port_1, WDF* _port_2) : port1{_port_1}, port2{_port_2}{};
    
    // The adaptor's port resistance is the sum of children's port resistances (R_series).
    // We also cache alpha = R1/(R1+R2) which is used in scattering during down-sweep.
    void calc_impedence() override
    {
        R0 = port1->getR0() + port2->getR0();
        port1Reflect = port1->getR0()/R0; // alpha = R1 / (R1 + R2)
    }
    
//    (Old attempt kept for reference. The sign pattern inside was the issue.)
//    void incident(float x) override
//    {
//        auto b1 = port1->getB() - port1Reflect * (x + port1->getB() + port2->getB());
//        port1->incident(b1);
//        port2->incident((b1 + x));
//        a = x;
//    }
    
    // Down-sweep: split parent's incident wave 'x' into two child incident waves.
    // Standard 2-port series scattering:
    //   let b1 = port1->getB(), b2 = port2->getB(), alpha = R1/(R1+R2)
    //   wsum = x - b1 - b2
    //   a1 = b1 + alpha        * wsum
    //   a2 = b2 + (1 - alpha)  * wsum
    void incident(float x) override
    {
        a = x;

        const float b1 = port1->getB();  // child-1's outgoing wave (already computed in up-sweep)
        const float b2 = port2->getB();  // child-2's outgoing wave

        const float alpha = port1Reflect;          // alpha = R1 / (R1 + R2)
        const float wsum  = x - b1 - b2;           // NOTE: minus signs are essential

        const float a1 = b1 + alpha        * wsum; // incident for child-1
        const float a2 = b2 + (1.0f-alpha) * wsum; // incident for child-2

        port1->incident(a1); // push down to child-1
        port2->incident(a2); // push down to child-2
    }
    
    // Up-sweep: gather child b's and produce adaptor b for the parent.
    // For a 2-port series adaptor: b = -(b1 + b2).
    float reflected() override
    {
        b = -(port1->reflected() + port2->reflected());
        return b;
    }
    
private:
    WDF* port1 = nullptr; // first child one-port
    WDF* port2 = nullptr; // second child one-port
    float port1Reflect = 0.0f; // alpha term cached for scattering
};



/* Root Node
 *
 * The root sets boundary conditions. Here it's an ideal/Thevenin voltage source
 * with series resistance Rs (R0=Rs). For an ideal source, Rs=0.
 * In wave form for a Thevenin source: b = 2*vs - a.
 */
class IdealVoltageSource : public WDF {
public:
    explicit IdealVoltageSource(WDF* root, float Rs_ = 1.0f, float vs_ = 0.0f)
        : child(root), Rs(Rs_), vs(vs_)
    {
        R0 = Rs; // The source port resistance equals its series resistance.
    }
    void setChild(WDF* root) { child = root; }

    void setVoltage(float v) { vs = v; }
    void setSeriesResistance(float r){ Rs = r; R0 = r; }

    // Nothing to compute dynamically: R0 is directly the series resistance Rs.
    void calc_impedence() override
    {
        //
    }

    // Down-sweep: receive the root incident wave 'x', compute the source's b,
    // and push that incident into the child network.
    // For Thevenin: b = 2*vs - a.
    void incident(float x) override
    {
        a = x;
        b = 2.0f * vs - a;
        child->incident(b);
    }

    // Up-sweep: the source simply returns 'a'. (It dictates 'b' via vs.)
    float reflected() override
    {
        return a;
    }

private:
    WDF*  child { nullptr }; // the network below the source
    float Rs    { 1.0f };    // series resistance of the source (Ohms)
    float vs    { 0.0f };    // instantaneous source voltage (Volts)
};



/* Class to tie it all together
 *
 * This builds a classic RC low-pass:
 *
 *   Vin -- R1 --+--- Vout
 *               |
 *               C1
 *               |
 *              GND
 *
 * WDF mapping:
 *   - R1 and C1 are connected by a SeriesAdaptor (current equal; voltages add).
 *   - Vin is a Thevenin root that injects a wave based on vs.
 *   - The capacitor's port voltage is the low-pass output.
 *
 * Processing per sample:
 *   1) Up-sweep: ask the adaptor S1 for its reflected wave (this queries children).
 *   2) Down-sweep: give that 'a_root' to the source; the source computes b=2*vs-a
 *      and pushes down; the adaptor splits into a1,a2 for R and C.
 *   3) Readout: after both sweeps, read C1.toVoltage() for Vout.
 */
class RCLowpass {
public:
    Resistor        R1 {1.0e3f};
    Capacitor       C1 {1.0e-6f};
    SeriesAdaptor   S1 { &R1, &C1 };
    IdealVoltageSource Vin { &S1, 0.0f /*ideal*/ };

    // Must be called before processing to set fs and compute port resistances.
    void setup(float sample_rate){
        C1.set_sample_rate(sample_rate);
        C1.reset_state();

        // compute port resistances in dependency order:
        // leaves (R1,C1) -> adaptor (S1) -> root (Vin)
        R1.calc_impedence();
        C1.calc_impedence();
        S1.calc_impedence();
        Vin.calc_impedence();
    }

    // Process one input sample (volts) and return the low-pass output (volts).
    float process_sample(float input_voltage){
        // Update the source value for this sample.
        Vin.setVoltage(input_voltage);

        // 1) up-sweep: from leaves to adaptor
        //    Calling S1.reflected() recursively pulls b from R1 and C1, then
        //    combines them into S1's b. That b is what arrives at the root as 'a'.
        float a_root = S1.reflected();     // adaptor b is what arrives at root

        // 2) down-sweep: root scatters and pushes down in its incident()
        //    The source uses a_root and vs to compute its b, and then injects
        //    that into the network; the series adaptor splits to children.
        Vin.incident(a_root);

        // 3) Read capacitor voltage at this sample (the RC low-pass output).
        return C1.toVoltage();
    }
};





////////////////////////////////////////////////





//class WDF {
//
//public:
//    WDF() = default;
//    virtual ~WDF() = default;
//
//    virtual void calc_impedence() = 0;
//    virtual void incident(float x) = 0;
//    virtual float reflected() = 0;
//
//    float getR0() { return R0; }
//    float getA() { return a; }
//    float getB() { return b; }
//
//    //helper functions to turn into voltage or current
//    float toVoltage(){
//        return (a + b)/2.0f;
//    }
//
//    float toCurrent(){
//        return (a - b)/(2.0f * R0);
//    }
//
//protected:
//    float a {0.0f}; //incident wave
//    float b {0.0f}; //reflected wave
//    float R0 {0.0f}; //port impedence
//
//};
//
//
//
///* Component specific derived subclasses
// * Each will have their own calc_impedence, incident, and reflected functions
// *
// */
//class Resistor : public WDF{
//
//public:
//    Resistor(float new_resistance) : R{new_resistance}{};
//
//    void set_resistance(float new_resistance) { R = new_resistance; }
//
//    //port resistance is equal to the physical resistance of the resistor...easy enough!
//    void calc_impedence() override
//    {
//        R0 = R;
//    }
//
//    void incident(float x) override
//    {
//        a = x;
//    }
//
//    float reflected() override
//    {
//        b = 0.0f;
//        return b;
//    }
//
//private:
//    float R;
//};
//
//
//class Capacitor : public WDF {
//
//public:
//    Capacitor(float new_capacitance) : C{new_capacitance} {};
//    ~Capacitor() = default;
//
//    void set_sample_rate(float sample_rate) { fs = sample_rate; }
//    void set_capacitance(float new_capacitance) { C = new_capacitance; }
//    void reset_state() { z = 0.0f; }
//
//    void calc_impedence() override
//    {
//        R0 = 1.0f / (2.0f * fs * C);
//    }
//
//    void incident(float x) override
//    {
//        a = x;
//    }
//
//    float reflected() override
//    {
//        b = z;          // return stored wave
//        z = a;          // update state for next sample
//        return b;
//    }
//
//
//private:
//    float fs;
//    float C;
//    float z;
//
//};
//
//
//
///* Port Adaptors
// *
// *
// */
//
//class SeriesAdaptor : public WDF {
//public:
//    //pass in the two children ports
//    SeriesAdaptor(WDF* _port_1, WDF* _port_2) : port1{_port_1}, port2{_port_2}{};
//
//    //impedence thru the adaptor is just the combination of the children
//    void calc_impedence() override
//    {
//        R0 = port1->getR0() + port2->getR0();
//        port1Reflect = port1->getR0()/R0;
//    }
//
////    void incident(float x) override
////    {
////        auto b1 = port1->getB() - port1Reflect * (x + port1->getB() + port2->getB());
////        port1->incident(b1);
////        port2->incident((b1 + x));
////        a = x;
////    }
//
//    void incident(float x) override
//    {
//        a = x;
//
//        const float b1 = port1->getB();
//        const float b2 = port2->getB();
//
//        const float alpha = port1Reflect;          // alpha = R1 / (R1 + R2)
//        const float wsum  = x - b1 - b2;           // NOTE the minus signs
//
//        const float a1 = b1 + alpha        * wsum;
//        const float a2 = b2 + (1.0f-alpha) * wsum;
//
//        port1->incident(a1);
//        port2->incident(a2);
//    }
//
//    float reflected() override
//    {
//        b = -(port1->reflected() + port2->reflected());
//        return b;
//    }
//
//private:
//    WDF* port1 = nullptr;
//    WDF* port2 = nullptr;
//    float port1Reflect = 0.0f;
//};
//
//
///* Root Node
// *
// *
// */
//class IdealVoltageSource : public WDF {
//public:
//    explicit IdealVoltageSource(WDF* root, float Rs_ = 1.0f, float vs_ = 0.0f)
//        : child(root), Rs(Rs_), vs(vs_)
//    {
//        R0 = Rs;
//    }
//    void setChild(WDF* root) { child = root; }
//
//    void setVoltage(float v) { vs = v; }
//    void setSeriesResistance(float r){ Rs = r; R0 = r; }
//
//    void calc_impedence() override
//    {
//        //
//    }
//
//    void incident(float x) override
//    {
//        a = x;
//        b = 2.0f * vs - a;
//        child->incident(b);
//    }
//
//    float reflected() override
//    {
//        return a;
//    }
//
//private:
//    WDF*  child { nullptr };
//    float Rs    { 1.0f };
//    float vs    { 0.0f };
//};
//
//
///* Class to tie it all together
// *
// *
// */
//class RCLowpass {
//public:
//    Resistor        R1 {1.0e3f};
//    Capacitor       C1 {1.0e-6f};
//    SeriesAdaptor   S1 { &R1, &C1 };
//    IdealVoltageSource Vin { &S1, 0.0f /*ideal*/ };
//
//    void setup(float sample_rate){
//        C1.set_sample_rate(sample_rate);
//        C1.reset_state();
//
//        // compute port resistances in dependency order
//        R1.calc_impedence();
//        C1.calc_impedence();
//        S1.calc_impedence();
//        Vin.calc_impedence();
//    }
//
//    float process_sample(float input_voltage){
//        Vin.setVoltage(input_voltage);
//
//        // 1) up-sweep: from leaves to adaptor
//        float a_root = S1.reflected();     // adaptor b is what arrives at root
//
//        // 2) down-sweep: root scatters and pushes down in its incident()
//        Vin.incident(a_root);
//
//        // Read capacitor voltage at this sample
//        return C1.toVoltage();
//    }
//};


