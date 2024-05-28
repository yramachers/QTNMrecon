// Digitizer implementation

// std
#include <iostream>
#include <algorithm>

// us
#include "digitizer.hh"

Digitizer::Digitizer() : 
vrange(0.0 * V),
sampling(0.0 * Hz),
bitrange(12) // default 12-bits
{
    if (bitrange>16) bitrange = 16;
    bmax = (int)pow(2,bitrange-1); // for signed range
    calarray.clear();
    darray.clear();
    calarray.resize(2*bmax);
    darray.resize(2*bmax);
    for (std::int16_t i=(std::int16_t)-bmax;i<(std::int16_t)bmax;++i) {
        darray[i+bmax] = i; // digi_t contains std::int16_t
        calarray[i+bmax] = (double)i * V;
    }
}

Digitizer::Digitizer(quantity<V> voltrange, quantity<isq::frequency[Hz]> samp, int b) : 
vrange(voltrange),
sampling(samp),
bitrange(b)
{
    setADCBits(b);
}

quantity<isq::time[s]> Digitizer::getSamplingTimeInterval()
{
    if (sampling>0 * Hz)
        return 1./sampling; // turn into a double of unit [s]
    else
        return -1.0 * s; // no sampling set
}

void Digitizer::dumpInfo()
{
    std::cout << "Digitizer Info: " << std::endl;
    std::cout << "voltage range [V]: " << vrange.in(V) << "; sampling rate [Hz]: " << sampling.in(Hz)
            << " ADC bits: " << bitrange << std::endl;
}

waveform_t Digitizer::digitize(const waveform_t& wfm)
{
    waveform_t temp;
    digitize_raw(wfm); // dwave, vrange set and ready

    quantity<V> stepwidth = vrange / bmax; // calibration
    for (auto entry : dwave) temp.push_back(entry * stepwidth); // short->double [V]
    return temp;
}

void Digitizer::digitize_raw(const waveform_t& wfm)
{
    dwave.clear();
    if (wfm.empty()) return; // empty in; empty result

    quantity<V> vmax = *std::max_element(wfm.begin(),wfm.end());
    quantity<V> vmin = *std::min_element(wfm.begin(),wfm.end());

    if (vrange <= 0.0 * V) // no voltage range set, take from data
        vrange = vmax - vmin;
    
    else if ((vmax-vmin)/vrange < 0.1 * one) // waveform under 10% of voltage range 
        vrange = vmax - vmin; // zoom-in to maximise use of bitrange

    double stepwidth = vrange.numerical_value_in(V) / bmax; // calibration
    // discrete bitrange scaled to voltage range
    std::transform(calarray.begin(),calarray.end(),calarray.begin(),
                    [&stepwidth](waveform_value x){return (x * stepwidth);});

    for (waveform_value entry : wfm) dwave.push_back(_adc(entry));
}

std::int16_t Digitizer::_adc(waveform_value val)
{
    std::int16_t idx = 0;
    for (auto it=begin(calarray); it!=end(calarray) && (*it)<=val; ++it) 
        ++idx; // count to find first index for val > darray
    if (idx>=2*bmax) idx = 2*bmax-1; // saturation range, get last entry
    return darray[idx];
}
