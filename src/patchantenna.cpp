// Patch Antenna response implementation

// us
#include "PatchAntenna.hh"


PatchAntenna::PatchAntenna(XYZPoint apos, XYZVector apol, double w, double h, double er) : 
antPos(apos),
antPol(apol),
width(w),
height(h),
epsr(er)
{}

vec_t PatchAntenna::voltage_response(Event<std::any>& ev)
{
    std::vector<XYZVector> efield = calculate_Efield(ev, antPos);
    vec_t response;
    try
    {
      auto omega = std::any_cast<std::vector<double>>(ev["OmVec"]); // ang freq
      double effperm = 0.5*(epsr+1.0) + 0.5*(epsr-1.0)/std::sqrt(1.0+12.0*height/width);
      for (size_t i=0; i<omega.size();++i) { // check units
        double effL = c_SI / (omega[i]/myPi*std::sqrt(effperm));
        double resp = (effL * efield[i].Dot(antPol)); // unit [V]
        response.push_back(resp);
      }
    }
    catch (const std::bad_any_cast& e)
    {
      std::cerr << e.what() << std::endl;
    }
    return response;
}
